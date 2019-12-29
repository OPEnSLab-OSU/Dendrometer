#include "ESP8266WiFi.h"
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include "gsheets.h"
#include "config.h"
#include "vector"



// WiFiClientSecure client;
GSheets api(O2key, sheetID);

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

 
  api.connectToHost();
  WiFiClientSecure client = api.getClient();

  std::vector<std::vector<String>> test = {{"asdffgdsa", "12345", "=2*3", "foo"},
                        {"asdffgdsa", "12345", "=2*3", "foo"},
                        {"asdffgdsa", "12345", "=2*3", "foo"}};

  String testValues[3][4] = {{"asdffgdsa", "12345", "=2*3", "foo"},
                        {"asdffgdsa", "12345", "=2*3", "foo"},
                        {"asdffgdsa", "12345", "=2*3", "foo"}};

  api.updateSheet("Sheet1!A21:D23", test, PARSED);

  // DynamicJsonDocument doc(2048);
  // DeserializationError err = deserializeJson(doc, body);
  // if(err)
  // {
  //   Serial.print("ERR");
  //   Serial.println(err.c_str());
  // }

  // String range = doc["updatedRange"];
  // Serial.println(range);
  // int code = doc["error"]["code"];
  // Serial.println(code);

}
