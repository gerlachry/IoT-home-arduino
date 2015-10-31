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
//turn if any errors
int ledRedPin = 4;

int value = 0;

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
  //WiFi
  WiFiClient client;
  
  if (!client.connect("192.168.1.15", 8000)) {
    Serial.println("connection failed");
    digitalWrite(BUILTIN_LED, LOW);
  }
  // Wait a few seconds between measurements.
  delay(2000);
  digitalWrite(ledGreenPin, HIGH); //turn on green to signal we are taking a reading
  digitalWrite(BUILTIN_LED, HIGH); //make sure red goes off
  float h = dht.readHumidity();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(f)) {
    Serial.println("Failed to read from DHT sensor!");
    digitalWrite(BUILTIN_LED, LOW);
    delay(1000);
    return;
  }

  // Compute heat index in Fahrenheit (the default)
  float hif = dht.computeHeatIndex(f, h);

  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print(f);
  Serial.print(" *F\t");
  Serial.print("Heat index: ");
  Serial.print(hif);
  Serial.println(" *F");
  digitalWrite(ledGreenPin, LOW);
}
