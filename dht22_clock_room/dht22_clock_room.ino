#include "ESP8266WiFi.h"
#include "DHT.h"
#include "settings.h"
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

void takeReadings() {
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
    //build url, could not get data to POST as body, the web server was not detecting the stream for some reason so moved to url params
    String data2 = "{\"device_name\":\"esp8266_001\",\"temperature\":";
    data2 += f;
    data2 += ",\"humidity\":";
    data2 += h;
    data2 += ",\"heat_index\":";
    data2 += hif;
    data2 += "} ";
    String hostS = host;
    String auth = "Authorization: Token ";
    auth += API_TOKEN;
    String url = "POST /api/readings?data=";
    url += data2;
    url += "HTTP/1.1\r\n";
    url += "HOST:";
    url += hostS;
    url += "\r\n";
    url += auth;
    url += "\r\n";
    url += "Content-Type: application/json\r\n";
    Serial.println(url);
    client.println(url);
    //client.println();
    while (!client.available()) {
      Serial.println("checking for response from server");
      delay(200);
    }
    while (client.available()) {
      char c = client.read();
      Serial.print(c);
    }
    Serial.println();
    Serial.println("--END---");
    digitalWrite(ledGreenPin, LOW);
  } else {
    Serial.println("host connection failed");
    Serial.println(host);
    Serial.println(port);
    digitalWrite(BUILTIN_LED, LOW);
  }
  client.stop();
}

void loop() {
  takeReadings();
  //every minute
  delay(60000);
}





