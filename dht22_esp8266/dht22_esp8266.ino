#include "DHT.h"
#include "ESP8266WiFi.h"
#include "settings.h"

WiFiClient client;
#define DHTPIN 12     // what digital pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321

DHT dht(DHTPIN, DHTTYPE);

void setup()
{
  Serial.begin(9600);
  Serial.println();
  Serial.println("Status\tHumidity (%)\tTemperature (C)\t(F)");

  //wifi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(WiFi.localIP());
  dht.begin();
}

void takeReadings() {
  client.stop();
  if (client.connect(host, port)) {
    //float humidity = dht.getHumidity();
    //float temperature = dht.getTemperature();
    //float temperatureF = dht.toFahrenheit(temperature);
    float humidity = dht.readHumidity();
    float temperatureF = dht.readTemperature(true);
    Serial.println(humidity);
    Serial.println(temperatureF);
    // Check if any reads failed and exit early (to try again).
    if (isnan(humidity) || isnan(temperatureF)) {
      Serial.println("Failed to read from DHT sensor!");
      delay(1000);
      return;
    }
    //build url, could not get data to POST as body, the web server was not detecting the stream for some reason so moved to url params
    String data2 = "{\"feed_name\":\"basement_temperature\",\"temperature\":";
    data2 += temperatureF;
//    data2 += ",\"humidity\":";
//    data2 += humidity;
    data2 += "} ";
    String hostS = host;
    String auth = "Authorization: Token ";
    auth += API_TOKEN;
    String url = "POST /api/feeds?data=";
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
    client.println();
    while (!client.available()) {
      Serial.println("checking for response from server");
      delay(200);
    }
    while (client.available()) {
      char c = client.read();
      Serial.print(c);
    }
    Serial.println();
    Serial.println("--END Temperature POST---");

    //build url, could not get data to POST as body, the web server was not detecting the stream for some reason so moved to url params
    data2 = "{\"feed_name\":\"basement_humidity\",\"humidity\":";
    //data2 += temperatureF;
    data2 += humidity;
    data2 += "} ";
    //String hostS = host;
    //String auth = "Authorization: Token ";
    //auth += API_TOKEN;
    url = "POST /api/feeds?data=";
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
    client.println();
    while (!client.available()) {
      Serial.println("checking for response from server");
      delay(200);
    }
    while (client.available()) {
      char c = client.read();
      Serial.print(c);
    }
    Serial.println();
    Serial.println("--END Humidity POST---");
  } else {
    Serial.println("host connection failed");
    Serial.println(host);
    Serial.println(port);
  }
  client.stop();
}

void loop()
{
  takeReadings();
  delay(300000);
}
