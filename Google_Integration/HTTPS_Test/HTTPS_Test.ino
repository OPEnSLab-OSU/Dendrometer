#include <WiFi101.h>
#include "gsheets.h"
#include "config.h"
#include "vector"


GSheets api(clientID, clientSecret, refreshToken, sheetID);


void setup() {
  WiFi.setPins(8, 7, 4, 2);
  Serial.begin(9600);
    WiFi.begin(ssid);
    while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      Serial.println("Connecting to WiFi..");
    }
    Serial.println("Connected to the WiFi network");
}

void loop() {

  //Create header for Google Sheet Table
  std::vector<std::vector<String>> header = {{"PWM Value", "Some Value", "Temperature", "Humidity"}};
  api.updateSheet("Sheet1!A1:D1", header, PARSED);

  int i = 1;
  int PWM;
  int rand;
  int temp;
  int humid;
  while(1)
  {
    delay(1000); //Simulate sensing value every second
    i+=1;
    PWM = i+1;
    rand = i+2;
    temp = i+3;
    humid = i+4;
    std::vector<std::vector<String>> row = {{String(PWM), String(rand), String(temp), String(humid)}};
    String a1Val = "Sheet1!A" + String(i) + ":D" + String(i);
    api.updateSheet(a1Val, row, PARSED);
  }
}
