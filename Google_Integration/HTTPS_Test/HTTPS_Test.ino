#include "ESP8266WiFi.h"
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

const char* ssid = "PC";
const char* password = "wifiiscool123";

BearSSL::WiFiClientSecure client;

void setup() {

  Serial.begin(115200);
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
//  char host[] = "postman-echo.com";

  client.setInsecure();
  if(client.connect(host, 443))
  {
    Serial.println("Connected to host");
  }
String url = "/v4/spreadsheets/1V596vKnz4UQQdkYMQul1m7FXl5XAwja6qOcjwsxcars?fields=sheets.properties&prettyPrint=false&key=AIzaSyALmgnzW17g9vLQmqi0pP86fAVJyRtLLTk";
//  String url = "/get";
//
//  client.println("GET " + url + " HTTP/1.1");
//  client.print("Host: ");
//  client.println(host);
//  client.println("User-Agent: arduino/1.0");
//  client.println("Connection: keep-alive");
//  client.println("");

  client.print(String("GET ") + url + " HTTP/1.0\r\n" + "Accept: */*\r\n" + "Cache-Control: no-cache\r\n" + "Host: " + host + "\r\n\r\n");
  Serial.println("Sent");
//  long now = millis(); 
  delay(2000);
  String response = "";
  while(client.available())
  {
//    String line = client.readStringUntil('\n');
//    Serial.println(line);

//      String header = client.readStringUntil('\r');
//      Serial.println(header);
  
//  char c = client.read();
//  response = response + c;
//  Serial.print(c);



    char c = client.read();
//    Serial.print(c);
    if (finishedHeaders) {
//          Serial.println(c, DEC);
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
  Serial.println("HEAD");
  Serial.println(headers);
    Serial.println("BODY");
  Serial.println(body);
  delay(10000);
}
