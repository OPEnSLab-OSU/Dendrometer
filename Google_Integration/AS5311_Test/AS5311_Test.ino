//#include <dummy.h>
//#include "RTClib.h" // real time clock library
#include "credentials.h"
#include "ESP8266WiFi.h"
#include "ESP8266HTTPClient.h"
//#include <WiFiClientSecure.h>
int input = D7;

//(DateTime now;

//WiFiClientSecure client;

void setup() {
  // put your setup code here, to run once:
  pinMode(input, INPUT);
  Serial.begin(9600);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
    Serial.println("Connected to the WiFi network");
//   http.begin("https://sheets.googleapis.com/v4/spreadsheets/:spreadsheetId/values/Sheet1!A1:D1?valueInputOption=USER_ENTERED");
//   http.addHeader("Authorization", OAuthKey);
//   int httpCode = http.PUT("{\"range\": \"Sheet1!A1:D1\",\"majorDimension\": \"ROWS\",\"values\": [[\"foo\", \"Casdfost\", \"Sgsdftocked\", \"Shiasdp Date\"],]}");
//
//String payload = http.getString();
//
//Serial.println(httpCode);
//Serial.println(payload);
//http.end();

 }

void loop() {
  HTTPClient http;
//   http.begin("https://sheets.googleapis.com/v4/spreadsheets/1V596vKnz4UQQdkYMQul1m7FXl5XAwja6qOcjwsxcars/values/Sheet1!A1:D1?valueInputOption=USER_ENTERED");
   http.begin("http://sheets.googleapis.com/v4/spreadsheets/1V596vKnz4UQQdkYMQul1m7FXl5XAwja6qOcjwsxcars?fields=sheets.properties&key=AIzaSyALmgnzW17g9vLQmqi0pP86fAVJyRtLLTk");
//    http.begin("http://jsonplaceholder.typicode.com/users/1");
//   http.addHeader("Authorization", OAuthKey);
//    http.addHeader("Content-Type", "application/json; charset=UTF-8");
//   int httpCode = http.PUT("{\"range\": \"Sheet1!A1:D1\",\"majorDimension\": \"ROWS\",\"values\": [[\"foo\", \"Casdfost\", \"Sgsdftocked\", \"Shiasdp Date\"],]}");
  int httpCode = http.GET();

String payload = http.getString();
Serial.println("Code");
Serial.println(httpCode);
Serial.println("Pay");
Serial.println(payload);
Serial.println("End");

http.end();
delay(5000);
  // put your main code here, to run repeatedly:
//  int val = digitalRead(input);
  //PWM Output range 1...4097 microseconds, correlates to 0-2mm
//  
//  int val = pulseIn(input, HIGH);
//  Serial.println(val);
//  delay(100);
}
