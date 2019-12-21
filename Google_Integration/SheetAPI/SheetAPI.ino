#include "credentials.h"
#include "ESP8266WiFi.h"
#include "ESP8266HTTPClient.h"

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
    Serial.println("Connected to the WiFi network");
 }

void loop() {
  // put your main code here, to run repeatedly:

//  Serial.write(OAuthKey);
//  Serial.write("\n");
//  delay(1000);
//  Serial.write(SheetID);
//  Serial.write("\n");
//  delay(1000);

}
