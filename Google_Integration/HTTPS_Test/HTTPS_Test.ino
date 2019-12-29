#include "ESP8266WiFi.h"
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include "gsheets.h"
#include "config.h"



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
  String title = "";
  String headers = "";
  String body = "";
  bool finishedHeaders = false;
  bool currentLineIsBlank = true;
  bool gotResponse = false;
 
  char host[] = "sheets.googleapis.com";

  api.connectToHost();
  WiFiClientSecure client = api.getClient();

  api.updateSheet("Sheet1!A1:D1", PARSED);

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
