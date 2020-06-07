#include <WiFi101.h>
#include "AS5311.h"
// #include "gsheets.h"
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

#define RTC_INT_PIN 12

LoomFactory<
	Enable::Internet::Disabled,
	Enable::Sensors::Enabled,
	Enable::Radios::Disabled,
	Enable::Actuators::Disabled,
	Enable::Max::Disabled
> ModuleFactory{};

LoomManager Loom{ &ModuleFactory };

void wakeISR_RTC()
{
    Serial.println("IN ISR");
  detachInterrupt(RTC_INT_PIN);
    digitalWrite(CS, HIGH);
    digitalWrite(CLK, LOW);
}

// Variables to track overall displacement
uint32_t start = 0;
uint32_t prevTwoSig = 0;
float elapsed = 0;

void setup() 
{
    // Needs to be done for Hypno Board
    pinMode(5, OUTPUT);
    digitalWrite(5, LOW); // Sets pin 5, the pin with the 3.3V rail, to output and enables the rail
    pinMode(6, OUTPUT);
    digitalWrite(6, HIGH); // Sets pin 6, the pin with the 5V rail, to output and enables the rail

    pinMode(12, INPUT_PULLUP);		// Enable waiting for RTC interrupt, MUST use a pullup since signal is active low

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

	// Register an interrupt on the RTC alarm pin
	Loom.InterruptManager().register_ISR(RTC_INT_PIN, wakeISR_RTC, LOW, ISR_Type::IMMEDIATE);


// Get starting Serial output (0-4095 value)
  start = getSerialPosition(CLK, CS, DO);
  // Save 2 most significant bits of start
  prevTwoSig = start & 0xC00;

}


void loop() {

  digitalWrite(5, LOW); // Turn on 3.3V rail
  digitalWrite(6, HIGH);  // Turn on 5V rail
pinMode(10, OUTPUT);
pinMode(23, OUTPUT);
pinMode(24, OUTPUT);

    Serial.println("IN LOOP");

    Loom.power_up();

    Serial.println("After powerup");


    Loom.measure();
    Loom.package();
    Loom.display_data();
    Loom.pause();


//   delay(2000); //Sensing value every 6 seconds (3 + 2 + 1, 3 from GSheets and 1 from Loom.pause)

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
//   Serial.println("What will be logged to SD");
//   serializeJsonPretty(data_json, Serial);
  
    Loom.SDCARD().log();

	// set the RTC alarm to a duration of 10 seconds with TimeSpan
	Loom.InterruptManager().RTC_alarm_duration(TimeSpan(0,0,0,10));
	Loom.InterruptManager().reconnect_interrupt(RTC_INT_PIN);

    Loom.power_down();

    digitalWrite(5, HIGH); // power down 3.3V rail
	digitalWrite(6, LOW);  // power down 5V rail
    pinMode(23, INPUT);
    pinMode(24, INPUT);
    pinMode(10, INPUT);


	Loom.SleepManager().sleep();


}