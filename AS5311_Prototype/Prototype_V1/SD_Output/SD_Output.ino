#define DELAY_IN_SECONDS 0
#define DELAY_IN_MINUTES 15

#include "AS5311.h"
#include "vector"
#include <Loom.h>
#include <stdint.h>

const char* config = 
#include "loomconfig.h"
;

#define CS 9
#define CLK A5
#define DO A4

#define RTC_INT_PIN 12

#define HYPNOS3 5     // Hypnos 3V rail
#define HYPNOS5 6     // Hypnos 5V rail

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
float prev = 0;
float prevMicro = 0;

void setup() 
{

  // Needs to be done for Hypno Board
  pinMode(HYPNOS3, OUTPUT);
  digitalWrite(HYPNOS3, LOW); // Sets pin 5, the pin with the 3.3V rail, to output and enables the rail
  pinMode(HYPNOS5, OUTPUT);
  digitalWrite(HYPNOS5, HIGH); // Sets pin 6, the pin with the 5V rail, to output and enables the rail

  pinMode(RTC_INT_PIN, INPUT_PULLUP);		// Enable waiting for RTC interrupt, MUST use a pullup since signal is active low

  //Setup for Loom Factory
  Loom.begin_serial(true);
  Loom.parse_config(config);
  Loom.print_config();

  // Pins to communicate with AS5311
  pinMode(CS, OUTPUT);
  pinMode(CLK, OUTPUT);
  pinMode(DO, INPUT_PULLUP);
  digitalWrite(CS, HIGH);
  digitalWrite(CLK, LOW);

  Loom.begin_LED();                     // Indicator for users to see if magnet is in good position
  digitalWrite(LED_BUILTIN, LOW);

  // LED indicator, TODO: Change to Neopixel LED
  uint32_t ledCheck = getErrorBits(CLK, CS, DO);                                           // Tracking magnet position for indicator

  while(ledCheck < 16 || ledCheck > 18) {

    if (ledCheck >= 16 && ledCheck <= 18) { // Error bits: 10000, 10001, 10010
      digitalWrite(LED_BUILTIN, LOW);
      break;
    }
   
    digitalWrite(LED_BUILTIN, HIGH);
    delay(3000);                                                                           // Gives user 3 seconds to adjust magnet before next reading
    ledCheck = getErrorBits(CLK, CS, DO);

  }

  delay(2000);

	// Register an interrupt on the RTC alarm pin
	Loom.InterruptManager().register_ISR(RTC_INT_PIN, wakeISR_RTC, LOW, ISR_Type::IMMEDIATE);

  // Takes 16 measurements and averages them for the starting Serial value (0-4095 value)
  for(int j = 0; j < 16; j++)
  {
    start += getSerialPosition(CLK, CS, DO);
  }
  start /= 16;
  //  Serial.println("Start: " + String(start));

  // Save 2 most significant bits of start
  prevTwoSig = start & 0xC00;

}


void loop() {

  digitalWrite(HYPNOS3, LOW); // Turn on 3.3V rail
  digitalWrite(HYPNOS5, HIGH);  // Turn on 5V rail
  pinMode(10, OUTPUT);
  pinMode(23, OUTPUT);
  pinMode(24, OUTPUT);

  Serial.println("IN LOOP");

  Loom.power_up();

  Serial.println("After powerup");

  Loom.measure();
  Loom.package();      
  Loom.display_data(); 

  // 16 point average of Serial Position
  int average = 0;
  for(int j = 0; j < 16; j++)
  {
    average += getSerialPosition(CLK, CS, DO);
  }
  average /= 16;
  // Serial.println("Average Serial Pos: " + String(average));

  uint32_t errorBits = getErrorBits(CLK, CS, DO);

// Also updates prevTwoSig to two most significant bits of first param, is being passed by ref
  elapsed = computeElapsed(average, prevTwoSig, elapsed);

  // Computes total distance in mm and um
  float distance = (elapsed + ((2.0 * ((int) average - (int) start))/4095.0));
  float distanceMicro = (elapsed * 1000) + ((2000 * ((int) average - (int) start))/4095.0);
  float difference = 0;
  float differenceMicro = 0;

  // Reads the movement if any, else it sets the changed distance to 0
  if (distance != prev)
    difference = distance - prev;

  if (distanceMicro != prevMicro)
    differenceMicro = distanceMicro - prevMicro;

  // Cannot add two keys to same module 
  // TODO: Talk to Loom Developers about this
  Loom.add_data("AS5311", "Serial Value", average);
  Loom.add_data("Displacement (mm)", "mm", distance);
  Loom.add_data("Displacement (um)", "um", distanceMicro);
  Loom.add_data("Difference (mm)", "mm", difference);
  Loom.add_data("Difference (um)", "um", differenceMicro);

  // Logs the status of the magnet position (whether the data is good or not) {Green = Good readings, Red = Bad readings}
  // "Error" occurs when something other than magnet placement causes a problem
  // Ignores the parity bit (last bit)
  if (errorBits >= 16 && errorBits <= 18) { // Error bits: 10000, 10001, 10010
    Loom.add_data("Status", "Color", "Green");
  }
  else if (errorBits == 19) {               // Error bits: 10011
    Loom.add_data("Status", "Color", "Yellow");
  }
  else if (errorBits == 23) {               // Error bits: 10111
    Loom.add_data("Status", "Color", "Red");
  }
  else {
    Loom.add_data("Status", "Color", "Error");
  }

  JsonObject data_json = Loom.internal_json(false);

  Loom.SDCARD().log();
  prev = distance;
  prevMicro = distanceMicro;

	// set the RTC alarm to a specified duration, DELAY_IN_SECONDS, with TimeSpan
	Loom.InterruptManager().RTC_alarm_duration(TimeSpan(0,0, DELAY_IN_MINUTES, DELAY_IN_SECONDS)); 
  Loom.InterruptManager().reconnect_interrupt(RTC_INT_PIN);

  Loom.power_down();

  digitalWrite(HYPNOS3, HIGH); // Power down 3.3V rail
	digitalWrite(HYPNOS5, LOW);  // Power down 5V rail
  pinMode(23, INPUT);
  pinMode(24, INPUT);
  pinMode(10, INPUT);

	Loom.SleepManager().sleep();

}
