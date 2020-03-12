#include <WiFi101.h>
#include "AS5311.h"
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


// ----------- GSHEETS -------------

// GSheets api(clientID, clientSecret, refreshToken, sheetID);
// int i = 1;
// String currTime;

// ----------- GSHEETS -------------


// Variables to track overall displacement
uint32_t start = 0;
uint32_t prevTwoSig = 0;
float elapsed = 0;

void setup() {

  // Setup for Hypnos Board
  pinMode(5, OUTPUT);
  digitalWrite(5, LOW); // Sets pin 5, the pin with the 3.3V rail, to output and enables the rail
  pinMode(6, OUTPUT);
  digitalWrite(6, HIGH); // Sets pin 6, the pin with the 5V rail, to output and enables the rail

  //Setup for Loom Factory
  Loom.begin_serial(true);
  Loom.parse_config(config);
  Loom.print_config();
  Loom.pause();
  

  // Pins to communicate with AS5311
  pinMode(CS, OUTPUT);
  pinMode(CLK, OUTPUT);
  pinMode(DO, INPUT_PULLDOWN);
  digitalWrite(CS, HIGH);
  digitalWrite(CLK, LOW);

  // Get starting Serial output (0-4095 value)
  start = getSerialPosition(CLK, CS, DO);
  // Save 2 most significant bits of start
  prevTwoSig = start & 0xC00;




  // ----------- GSHEETS -------------
  
  // WiFi.setPins(8, 7, 4, 2);
  //   WiFi.begin(ssid);
  //   while (WiFi.status() != WL_CONNECTED) {
  //     Loom.pause();
  //     Serial.println("Connecting to WiFi..");
  //   }
  //   Serial.println("Connected to the WiFi network");

  //   int i = 1;
  //   String currTime;

    // Create header for Google Sheet Table
    // std::vector<std::vector<String>> header = {{"Time", "Control Temp", "Value", "Actual Temp", "Humidity"}};
    // api.updateSheet("Sheet1!A1:E1", header, PARSED);

  // ----------- /GSHEETS -------------

}

void loop() {

    Loom.measure();
    Loom.package();
    Loom.display_data();
    Loom.pause();


  delay(2000); //Sensing value every 6 seconds (3 + 2 + 1, 3 from GSheets and 1 from Loom.pause)

  // 16 point average of Serial Position
  int average = 0;
  for(int j = 0; j < 16; j++)
  {
    average += getSerialPosition(CLK, CS, DO);
  }
  average /= 16;
  Serial.println("Average Serial Pos: " + String(average));


// Also updates prevTwoSig to two most significant bits of first param, is being passed by ref
  elapsed = computeElapsed(average, prevTwoSig, elapsed);

  // Computes total distance
  float distance = (elapsed + ((2.0 * ((int) average - (int) start))/4095.0));
  Serial.println("Total distance: " + String(distance));


// Cannot add two keys to same module 
// TODO: Talk to Loom Developers about this
  Loom.add_data("AS5311", "Serial Value", average);
  Loom.add_data("Displacement", "Distance", distance);

  JsonObject data_json = Loom.internal_json(false);
  Serial.println("What will be logged to SD");
  serializeJsonPretty(data_json, Serial);
  
  Loom.SDCARD().log();


  // ----------- GSHEETS -------------
  //Increment Row
  // i+=1;

  //Getting Time and Date
  // TODO
  // currTime = "Time";
  // 

  // Getting temperature and humidity
  // TODO
  // int temp = 2;
  // int humid = 2;

  //Building Google Sheet Row and sending
  // std::vector<std::vector<String>> row = {{currTime, "", String(average), String(temp), String(humid)}};
  // String a1Val = "Sheet1!A" + String(i) + ":E" + String(i);
  // api.updateSheet(a1Val, row, PARSED);

  // ----------- /GSHEETS -------------


}
