#include <WiFi101.h>
#include <SPI.h>

String ssid = "OSU_Access";

char server[] = "www.google.com";

int status = WL_IDLE_STATUS;
WiFiSSLClient client;

void setup() {
  WiFi.setPins(8, 7, 4, 2);
  Serial.begin(9600);
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    status = WiFi.begin(ssid);
    delay(1000);
    }
    Serial.println("Connected to wifi");
   Serial.println("\nStarting connection to server...");
    if (client.connect(server, 443)) {
    Serial.println("connected to server");
    }
}

void loop() {
  // put your main code here, to run repeatedly:

}
