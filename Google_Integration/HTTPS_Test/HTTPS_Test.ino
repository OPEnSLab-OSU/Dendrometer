#include "ESP8266WiFi.h"
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include "gsheets.h"
#include "config.h"
#include "vector"


GSheets api(clientID, clientSecret, refreshToken, sheetID);

void setup() {

  Serial.begin(9600);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      Serial.println("Connecting to WiFi..");
    }
    Serial.println("Connected to the WiFi network");
}

void loop() {

 
  std::vector<std::vector<String>> test = {{"asdffgdsa", "12345", "=2*3", "foo"},
                        {"asdffgdsa", "12345", "=2*3", "foo"},
                        {"asdffgdsa", "12345", "=2*3", "foo"}};

  api.updateSheet("Sheet1!A21:D23", test, PARSED);

}
