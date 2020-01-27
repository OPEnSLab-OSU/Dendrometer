#include <WiFi101.h>
#include "gsheets.h"
#include "config.h"
#include "vector"
#include <Adafruit_SHT31.h>
#include "RTClib.h"

GSheets api(clientID, clientSecret, refreshToken, sheetID);
Adafruit_SHT31 sht31 = Adafruit_SHT31();
DateTime now;
RTC_DS3231 rtc;

void setup() {

  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }
  
//  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  
  pinMode(11, INPUT);
  WiFi.setPins(8, 7, 4, 2);
  Serial.begin(9600);
    WiFi.begin(ssid);
    while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      Serial.println("Connecting to WiFi..");
    }
    Serial.println("Connected to the WiFi network");

    if (! sht31.begin(0x44)) {   //Initializes temperature/humidity sensor
      Serial.println("Couldn't find SHT31"); 
    }
    if (rtc.lostPower()) {
    Serial.println("RTC lost power, lets set the time!");
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
}

void loop() {
  //Create header for Google Sheet Table
  std::vector<std::vector<String>> header = {{"Time", "Control Temp", "Value", "Actual Temp", "Humidity"}};
  api.updateSheet("Sheet1!A1:E1", header, PARSED);

  int i = 1;
  int PWM;
  int temp;
  int humid;
  String time;
  while(1)
  {
    delay(2000); //Sensing value every 5 seconds (3 + 2)
    i+=1;
    now = rtc.now();
    time = String(now.month()) + "-" + String(now.day()) + "-" + String(now.hour()) + ":" + String(now.minute()) + ":" + String(now.second());
    PWM = pulseIn(11, HIGH);
    temp = sht31.readTemperature();
    humid = sht31.readHumidity();
    std::vector<std::vector<String>> row = {{time, "", String(PWM), String(temp), String(humid)}};
    String a1Val = "Sheet1!A" + String(i) + ":E" + String(i);
    api.updateSheet(a1Val, row, PARSED);
  }
}
