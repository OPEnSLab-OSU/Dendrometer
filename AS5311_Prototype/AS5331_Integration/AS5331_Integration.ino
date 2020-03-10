#include <WiFi101.h>
#include "AS5331.h"
#include "gsheets.h"
#include "config.h"
#include "vector"
#include <Loom.h>
#include <stdint.h>

const char* config = 
#include "loomconfig.h"
;

#define CS 9
#define CLK A5
#define DO A4
#define PULSE 11

LoomFactory<
	Enable::Internet::Disabled,
	Enable::Sensors::Enabled,
	Enable::Radios::Disabled,
	Enable::Actuators::Disabled,
	Enable::Max::Disabled
> ModuleFactory{};

LoomManager Loom{ &ModuleFactory };


GSheets api(clientID, clientSecret, refreshToken, sheetID);

int i = 1;
String currTime;

uint32_t start = 0;
uint32_t prevTwoSig = 0;
float elapsed = 0;


void setup() {
  pinMode(5, OUTPUT);
  digitalWrite(5, LOW); // Sets pin 5, the pin with the 3.3V rail, to output and enables the rail
  pinMode(6, OUTPUT);
  digitalWrite(6, HIGH); // Sets pin 6, the pin with the 5V rail, to output and enables the rail

  Loom.begin_serial(true);
  Loom.parse_config(config);
  Loom.print_config();
    
  pinMode(CS, OUTPUT);
  pinMode(CLK, OUTPUT);
  pinMode(DO, INPUT_PULLDOWN);
  digitalWrite(CS, HIGH);
  digitalWrite(CLK, LOW);

  Loom.pause();

  
  WiFi.setPins(8, 7, 4, 2);
    WiFi.begin(ssid);
    while (WiFi.status() != WL_CONNECTED) {
      Loom.pause();
      Serial.println("Connecting to WiFi..");
    }
    Serial.println("Connected to the WiFi network");


    int i = 1;
    String currTime;

    start = bitbang(CLK, CS, DO);
    start = convertBits(start);
    prevTwoSig = start & 0xC00;

    // Create header for Google Sheet Table
    // std::vector<std::vector<String>> header = {{"Time", "Control Temp", "Value", "Actual Temp", "Humidity"}};
    // api.updateSheet("Sheet1!A1:E1", header, PARSED);
}

void loop() {

    Loom.measure();
    Loom.package();
    // Loom.display_data();
    Loom.pause();


  delay(2000); //Sensing value every 5 seconds (3 + 2)
  uint32_t curr = bitbang(CLK, CS, DO);
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


  // Getting Serial data
  //    x16 for average
  int average = 0;

  for(int j = 0; j < 16; j++)
  {
    uint32_t value = bitbang(CLK, CS, DO);
    uint32_t readval = value & 0xFFF;
    uint32_t newval = 0;
    for (int i = 11; i >= 0; i--) 
    {
      uint32_t exists = (readval & (1 << i)) ? 1 : 0;
      newval |= (exists << (11 - i));
    }
    average += newval;
  }
  average /= 16;

  //Increment Row
  i+=1;

  //Getting Time and Date
  // TODO
  currTime = "Time";
  // 

  // Getting temperature and humidity
  // TODO
  int temp = 2;
  int humid = 2;

  //Building Google Sheet Row and sending
  // std::vector<std::vector<String>> row = {{currTime, "", String(average), String(temp), String(humid)}};
  // String a1Val = "Sheet1!A" + String(i) + ":E" + String(i);
  // api.updateSheet(a1Val, row, PARSED);

  Serial.println(average);
}
