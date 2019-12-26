#include "ESP8266WiFi.h"
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include "gsheets.h"
#include "config.h"



BearSSL::WiFiClientSecure client;
GSheets form("asdf");

void setup() {

  Serial.begin(9600);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      Serial.println(form.testfunc());
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

  client.setInsecure();
  if(client.connect(host, 443))
  {
    Serial.println("Connected to host");
  }
String url = "/v4/spreadsheets/1V596vKnz4UQQdkYMQul1m7FXl5XAwja6qOcjwsxcars/values/Sheet1!A1:D1?valueInputOption=USER_ENTERED";
// String url = "/v4/spreadsheets/1V596vKnz4UQQdkYMQul1m7FXl5XAwja6qOcjwsxcars?fields=sheets.properties&prettyPrint=false&key=AIzaSyALmgnzW17g9vLQmqi0pP86fAVJyRtLLTk";

String oauth = "Bearer " + O2key;

  String payload = "{\"range\": \"Sheet1!A1:D1\",\"majorDimension\": \"ROWS\",\"values\": [[\"Foo\", \"Boo\", \"Stocked\", \"Ship Date\"]],}";
  client.print(String("PUT ") + url + " HTTP/1.0\r\n" + "Authorization: " + oauth + "\r\n" + "Accept: */*\r\n" + "Content-Length: " + payload.length() + "\r\n" + "Cache-Control: no-cache\r\n" + "Host: " + host + "\r\n\r\n" + payload + "\r\n\r\n");
  // client.print(String("GET ") + url + " HTTP/1.0\r\n" + "Accept: */*\r\n" + "Cache-Control: no-cache\r\n" + "Host: " + host + "\r\n\r\n");
  Serial.println("Sent");
//  long now = millis(); 
  delay(2000);
  String response = "";
  while(client.available())
  {
    char c = client.read();
    if (finishedHeaders) {
          body=body+c;
        } else {
          if (currentLineIsBlank && c == '\n') {
            finishedHeaders = true;
          }
          else {
            headers = headers + c;
          }
        }

        if (c == '\n') {
          currentLineIsBlank = true;
        }else if (c != '\r') {
          currentLineIsBlank = false;
        }
  }

  
  DynamicJsonDocument doc(2048);
  DeserializationError err = deserializeJson(doc, body);
  if(err)
  {
    Serial.print("ERR");
    Serial.println(err.c_str());
  }

  String range = doc["updatedRange"];
  Serial.println(range);
  int code = doc["error"]["code"];
  Serial.println(code);

  Serial.println("HEAD");
  Serial.println(headers);
    Serial.println("BODY");
  Serial.println(body);
  delay(10000);
}
