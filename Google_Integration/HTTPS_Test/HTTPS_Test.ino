#include <WiFi101.h>
#include "gsheets.h"
#include "config.h"
#include "vector"
#include <Adafruit_SHT31.h>
#include "RTClib.h"

#define CS 9
#define CLK A5
#define DO A4
#define PULSE 11

GSheets api(clientID, clientSecret, refreshToken, sheetID);
Adafruit_SHT31 sht31 = Adafruit_SHT31();
DateTime now;
RTC_DS3231 rtc;

void setup() {

  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
  }
    
  pinMode(CS, OUTPUT);
  pinMode(CLK, OUTPUT);
  pinMode(DO, INPUT_PULLDOWN);
//  pinMode(PULSE, INPUT);
  digitalWrite(CS, HIGH);
  digitalWrite(CLK, LOW);

  delay(1000);

  
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
//     following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }


}

uint32_t bitbang() {
  // write clock high to select the angular position data
  digitalWrite(CLK, HIGH);
  delay(1);
  // select the chip
  digitalWrite(CS, LOW);
  delay(1);
  digitalWrite(CLK, LOW);
  // read the value in it's entirety
  uint32_t value = 0;
  for (uint8_t i = 0; i < 18; i++) {
    delay(1);
    digitalWrite(CLK, HIGH);
    delay(1);
    digitalWrite(CLK, LOW);
    delay(1);
    auto readval = digitalRead(DO);
    if (readval == HIGH)
      value |= (1U << i);
  }
  digitalWrite(CS, HIGH);
  return value;
}

uint32_t convertBits(uint32_t num) {
      uint32_t readval = num & 0xFFF;
      uint32_t newval = 0;
      for (int i = 11; i >= 0; i--) 
      {
        uint32_t exists = (readval & (1 << i)) ? 1 : 0;
        newval |= (exists << (11 - i));
      }
      return newval;
}

void loop() {
  //Create header for Google Sheet Table
  // std::vector<std::vector<String>> header = {{"Time", "Control Temp", "Value", "Actual Temp", "Humidity"}};
  // api.updateSheet("Sheet1!A1:E1", header, PARSED);


  // int i = 1;
  // int PWM;
  // int temp;
  // int humid;
  // String time;

    uint32_t start = bitbang();
    start = convertBits(start);
    uint32_t prevTwoSig = start & 0xC00;
    float elapsed = 0;

  while(1)
  { 
    delay(2000); //Sensing value every 5 seconds (3 + 2)
    uint32_t curr = bitbang();
    curr = convertBits(curr);
    uint32_t currTwoSig = curr & 0xC00;
    if((currTwoSig == 0xC00 && prevTwoSig == 0x0)) {
      Serial.println("ROLLOVER UNDERFLOW");
      elapsed -= 2.0;
    } else if (prevTwoSig == 0xC00 && currTwoSig == 0x0) {
      Serial.println("ROLLOVER OVERFLOW");
      elapsed += 2.0;
    }
    Serial.print("Current value");
    Serial.println(curr);

    float distance = (elapsed + ((2.0 * ((int) curr - (int) start))/4095.0));
    Serial.print("Total distance: ");
    Serial.print(distance);

    
    prevTwoSig = currTwoSig;


    //Getting Serial data
//      x16 for average
    // int average = 0;

    // for(int j = 0; j < 16; j++)
    // {
    //   uint32_t value = bitbang();
    //   uint32_t readval = value & 0xFFF;
    //   uint32_t newval = 0;
    //   for (int i = 11; i >= 0; i--) 
    //   {
    //     uint32_t exists = (readval & (1 << i)) ? 1 : 0;
    //     newval |= (exists << (11 - i));
    //   }
    //   average += newval;
    // }
    // average /= 16;

    // //Increment Row
    // i+=1;

    // //Getting Time and Date
    // now = rtc.now();
    // time = String(now.month()) + "-" + String(now.day()) + "-" + String(now.hour()) + ":" + String(now.minute()) + ":" + String(now.second());

    // //Getting Temp and Humid
    // temp = sht31.readTemperature();
    // humid = sht31.readHumidity();

    // //Building Google Sheet Row and sending
    // std::vector<std::vector<String>> row = {{time, "", String(average), String(temp), String(humid)}};
    // String a1Val = "Sheet1!A" + String(i) + ":E" + String(i);
    // api.updateSheet(a1Val, row, PARSED);

    // Serial.println(average);
  }
}
