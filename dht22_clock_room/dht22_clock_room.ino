#include "ESP8266WiFi.h"
#include "DHT.h"
#include "settings.h"
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include <ArduinoJson.h>
#define DHTPIN 2     // what digital pin we're connected to

// Uncomment whatever type you're using!
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321

// Initialize DHT sensor.
DHT dht(DHTPIN, DHTTYPE);
//turn on when reading and transimitting data
int ledGreenPin = 5;

int value = 0;
int value2 = 0;
// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;

// Adafruit IO
#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME    "Frhollow12"
#define AIO_KEY         "0dc35c4b4e424047416d92c9a09949d3fab632e5"
// Store the MQTT server, client ID, username, and password in flash memory.
const char MQTT_SERVER[] PROGMEM    = AIO_SERVER;

// Set a unique MQTT client ID using the AIO key + the date and time the sketch
// was compiled (so this should be unique across multiple devices for a user,
// alternatively you can manually set this to a GUID or other random value).
const char MQTT_CLIENTID[] PROGMEM  = AIO_KEY __DATE__ __TIME__;
const char MQTT_USERNAME[] PROGMEM  = AIO_USERNAME;
const char MQTT_PASSWORD[] PROGMEM  = AIO_KEY;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, MQTT_SERVER, AIO_SERVERPORT, MQTT_CLIENTID, MQTT_USERNAME, MQTT_PASSWORD);/****************************** Feeds ***************************************/

// Setup feeds for temperature & humidity
const char TEMPERATURE_FEED[] PROGMEM = AIO_USERNAME "/feeds/temp-basement";
Adafruit_MQTT_Publish temperature = Adafruit_MQTT_Publish(&mqtt, TEMPERATURE_FEED);

const char HUMIDITY_FEED[] PROGMEM = AIO_USERNAME "/feeds/humidity-basement";
Adafruit_MQTT_Publish humidity = Adafruit_MQTT_Publish(&mqtt, HUMIDITY_FEED);

const char HEAT_INDEX_FEED[] PROGMEM = AIO_USERNAME "/feeds/heat-index-basement";
Adafruit_MQTT_Publish heat_index = Adafruit_MQTT_Publish(&mqtt, HEAT_INDEX_FEED);

void setup() {
  Serial.begin(9600);
  pinMode(BUILTIN_LED, OUTPUT); //builtin LED, turn on if any errors
  pinMode(ledGreenPin, OUTPUT);
  
  dht.begin();
  //wifi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(WiFi.localIP());
}

void loop() {
  adaFruit();
  takeReadings();
}

void adaFruit() {
 // ping adafruit io a few times to make sure we remain connected
 digitalWrite(ledGreenPin, HIGH); //turn on green to signal we are taking a reading
 digitalWrite(BUILTIN_LED, HIGH); //make sure red goes off
  if(! mqtt.ping(3)) {
    // reconnect to adafruit io
    if(! mqtt.connected())
      connect();
  }

  // Grab the current state of the sensor
  //int humidity_data = (int)dht.readHumidity();
  //int temperature_data = (int)dht.readTemperature();
  float humidity_data = dht.readHumidity();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float temperature_data = dht.readTemperature(true);
  // Compute heat index in Fahrenheit (the default)
  float heat_index_data = dht.computeHeatIndex(temperature_data, humidity_data);
  
  // Publish data
  if (! temperature.publish(temperature_data))
    Serial.println(F("Failed to publish temperature"));
  else
    Serial.println(F("Temperature published!"));

  if (! humidity.publish(humidity_data))
    Serial.println(F("Failed to publish humidity"));
  else
    Serial.println(F("Humidity published!"));

  if (! heat_index.publish(heat_index_data))
    Serial.println(F("Failed to publish heat index"));
  else
    Serial.println(F("heat index published!"));
    
  digitalWrite(ledGreenPin, LOW); //turn off green
  // Repeat every 60 seconds
  //delay(60000);
}

// connect to adafruit io via MQTT
void connect() {

  Serial.print(F("Connecting to Adafruit IO... "));

  int8_t ret;

  while ((ret = mqtt.connect()) != 0) {

    switch (ret) {
      case 1: Serial.println(F("Wrong protocol")); break;
      case 2: Serial.println(F("ID rejected")); break;
      case 3: Serial.println(F("Server unavail")); break;
      case 4: Serial.println(F("Bad user/pass")); break;
      case 5: Serial.println(F("Not authed")); break;
      case 6: Serial.println(F("Failed to subscribe")); break;
      default: Serial.println(F("Connection failed")); break;
    }

    if(ret >= 0)
      mqtt.disconnect();

    digitalWrite(ledGreenPin, LOW); //turn off green
    digitalWrite(BUILTIN_LED, LOW); //make sure red goes off
    Serial.println(F("Retrying connection..."));
    delay(5000);

  }

  Serial.println(F("Adafruit IO Connected!"));

}

void takeReadings() {
     // Wait a few seconds between measurements.
    //delay(5000);
    //WiFiClient client;
    client.stop();
    if (client.connect(host, port)) {
      digitalWrite(ledGreenPin, HIGH); //turn on green to signal we are taking a reading
      digitalWrite(BUILTIN_LED, HIGH); //make sure red goes off
      int h = dht.readHumidity();
      // Read temperature as Fahrenheit (isFahrenheit = true)
      int f = dht.readTemperature(true);
    
      // Check if any reads failed and exit early (to try again).
      if (isnan(h) || isnan(f)) {
        Serial.println("Failed to read from DHT sensor!");
        digitalWrite(ledGreenPin, LOW);
        digitalWrite(BUILTIN_LED, LOW);
        delay(1000);
        return;
      }
  
      // Compute heat index in Fahrenheit (the default)
      int hif = dht.computeHeatIndex(f, h);
      
      // create json for sending to feed
      StaticJsonBuffer<200> jsonBuffer;
      JsonObject& root = jsonBuffer.createObject();
      root["device_name"] = sensor_id;
      root["temperature_f"] = f;
      root["humidity"] = h;
      root["heat_index"] = hif;
      //get json string
      char dataBuffer[200];
      root.printTo(dataBuffer, sizeof(dataBuffer));
      //Serial.println("dataBuffer");
      //Serial.println(dataBuffer);
      
      String data2 = "{\"device_name\":\"esp8266_basement\",\"temperature_f\":\"";
      data2 += f;
      data2 += "\",\"humidity\":\"";
      data2 += h;
      data2 += "\",\"heat_index\":\"";
      data2 += hif;
      data2 += "\"}\r\n";
      String data3 = "test";
      //Serial.println(data2);
      //Serial.println(data2.length());
      String hostS = host;
      String auth = "Authorization: Token ";
      auth += API_TOKEN;
      String url = "POST /api/readings/ HTTP/1.0\r\n";
      url += "HOST:";
      url += hostS;
      url += "\r\n";
      url += auth;
      url += "\r\n";
      url += "User-Agent: Arduino/1.0\r\n";
      url += "Accept: */*\r\n";
      url += "Content-Type: application/json\r\n";
      url += "Connection: close\r\n";
      url += "Content-Length:";
      url += data3.length();
      url += "\r\n";
      url += data3;
      Serial.println(url);
      client.println(url);
      client.println();
      while(!client.available()){
        Serial.println("checking for response from server");
        delay(200);
      }
      while(client.available()){
        char c = client.read();
        Serial.print(c);
      }
  
      Serial.println("--END---");
      digitalWrite(ledGreenPin, LOW);
    } else {
        Serial.println("host connection failed");
        Serial.println(host);
        Serial.println(port);
        digitalWrite(BUILTIN_LED, LOW);
        delay(500);
    }
    client.stop();
}
  
