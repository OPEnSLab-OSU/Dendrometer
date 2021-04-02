///////////////////////////////////////////////////////////////////////////////

// This is a basic example of sending data via LoRa. 

// The corresponding example is LoRa > Receive (or Receive_Blocking)

// These two examples are the Loom equivalent of the basic RX / TX LoRa 
// examples

// See https://openslab-osu.github.io/Loom/html/class_loom___lo_ra.html
// for details of LoRa config options

// While you can send whatever data you collect, for the purposes of this 
// example, only analog data is being collected

// Documentation for LoRa: https://openslab-osu.github.io/Loom/doxygenV2/html/class_loom___lo_ra.html

///////////////////////////////////////////////////////////////////////////////

#include <Loom.h>
#include "AS5311.h"

// Include configuration
const char* json_config = 
#include "config.h"
;

// Set enabled modules
LoomFactory<
	Enable::Internet::Disabled,
	Enable::Sensors::Enabled,
	Enable::Radios::Enabled,
	Enable::Actuators::Enabled,
	Enable::Max::Disabled
> ModuleFactory{};

LoomManager Loom{ &ModuleFactory };

//Pins
#define CS 9
#define CLK A5
#define DO A4
#define LED A2

#define INT_BUT 11
#define RTC_INT_PIN 12

//Global Variables
volatile bool flag = false;   // Interrupt flag
volatile bool button = false; // Check to see if button was pressed
uint32_t start = 0;
uint32_t prevTwoSig = 0;
float elapsed = 0;
float prev = 0;
float prevMicro = 0;
const int max_packets = 10;
int counter;

void setup() 
{
	Loom.begin_serial(true);
	Loom.parse_config(json_config);
	Loom.print_config();
  
  // Enable waiting for RTC interrupt, MUST use a pullup since signal is active low
  pinMode(RTC_INT_PIN, INPUT_PULLUP);  
  pinMode(INT_BUT, INPUT_PULLUP);
  pinMode(5, OUTPUT);    // Enable control of 3.3V rail
  pinMode(6, OUTPUT);   // Enable control of 5V rail
  pinMode(13, OUTPUT);

  //initialize Hypnos
  digitalWrite(5, LOW); // Enable 3.3V rail
  digitalWrite(6, HIGH);  // Enable 5V rail
  digitalWrite(13, LOW);

  //Begin Communication with AS5311
  init_AS();

  //LED pin set
  pinMode(LED, OUTPUT);

  //LED indicator
  /*-------------- Keep commented until utilized. Will cause infinite loop --------------------------
  //Make sure the magnet is positioned correctly
  verify_position();
  //Flash three times for verification
  green_flash();
  ----------------------------------------------------------------------------------------------------
  */
  
  Loom.InterruptManager().register_ISR(RTC_INT_PIN, ISR_pin12, LOW, ISR_Type::IMMEDIATE);
  Loom.InterruptManager().register_ISR(INT_BUT, ISR_pin11, LOW, ISR_Type::IMMEDIATE);
  delay(5000);
  
  serial_init_measure();
  // Save 2 most significant bits of start
  prevTwoSig = start & 0xC00;
  
  Loom.Neopixel().set_color(2, 0, 0, 0, 0); // Turns off Neopixel
  counter = 0;
	LPrintln("\n ** Setup Complete ** ");
}

void loop() 
{
  //initialize Hypnos
  digitalWrite(5, LOW); // Enable 3.3V rail
  digitalWrite(6, HIGH);  // Enable 5V rail
  digitalWrite(13, LOW);
  
  Serial.println("Looping now");
  
  init_AS();
  delay(2000); //Warmup AS5311 chip

  if (flag) {
    pinMode(23, OUTPUT);
    pinMode(24, OUTPUT);
    pinMode(10, OUTPUT);
  
    // delay(1000);
  
    Loom.power_up();
  }

  verify_LED_button();

  //-------------------DATA MANAGEMENT-------------------------------------------------------
  int average = measure_average();
  uint32_t errorBits = getErrorBits(CLK, CS, DO);
  
  // Also updates prevTwoSig to two most significant bits of first param, is being passed by ref
  elapsed = computeElapsed(average, prevTwoSig, elapsed);

  // Computes total distance in mm and um
  float distance = (elapsed + ((2.0 * ((int)average - (int)start)) / 4095.0));
  float distanceMicro = (elapsed * 1000) + ((2000 * ((int)average - (int)start)) / 4095.0);
  float difference = 0;
  float differenceMicro = 0;

  // Reads the movement if any, else it sets the changed distance to 0
  if (distance != prev)
    difference = distance - prev;

  if (distanceMicro != prevMicro)
    differenceMicro = distanceMicro - prevMicro;

  Loom.measure();

  Loom.add_data("AS5311", "Serial Value", average);
  Loom.add_data("Displacement (mm)", "mm", distance);
  Loom.add_data("Displacement (um)", "um", distanceMicro);
  Loom.add_data("Difference (mm)", "mm", difference);
  Loom.add_data("Difference (um)", "um", differenceMicro);

  // Logs the status of the magnet position (whether the data is good or not) {Green = Good readings, Red = Bad readings}
  // Ignores the parity bit (last bit)
  if (errorBits >= 16 && errorBits <= 18) // Error bits: 10000, 10001, 10010
    Loom.add_data("Status", "Color", "Green");
  else if (errorBits == 19) // Error bits: 10011
    Loom.add_data("Status", "Color", "Yellow");
  else if (errorBits == 23) // Error bits: 10111
    Loom.add_data("Status", "Color", "Red");
  else if (errorBits < 16) // If OCF Bit is 0
    Loom.add_data("Status", "Color", "OCF Error");
  else if (errorBits > 24) // If COF Bit is 1
    Loom.add_data("Status", "Color", "COF Error");
  else
    Loom.add_data("Status", "Color", "Other Error");

  float temp, humidity, SVP, VPD;

  float e = 2.71828;

  temp = Loom.SHT31D().get_temperature();
  humidity = Loom.SHT31D().get_humidity();
  SVP = (0.61078 * pow(e, (17.2694 * temp) / (temp + 237.3)));
  VPD = SVP * (1 - (humidity / 100));

  Loom.add_data("VPD", "VPD", VPD);

  prev = distance;
  prevMicro = distanceMicro;
//-----------------------------------------------------------------------------------
  
	Loom.package();
	Loom.display_data();

  // Log SD in case it doesn't send
  Loom.log_all();
  counter++;
  
	// Send to address 1
  if(counter%max_packets == 0){
	  Loom.LoRa().send_batch(1, 4000);
  }

	Loom.pause();	// Delay between interations set with 'interval' in config
}



//AS5311 functions

void init_AS(){
  // Protocol to turn on AS5311
  pinMode(CS, OUTPUT);
  pinMode(CLK, OUTPUT);
  pinMode(DO, INPUT_PULLUP);
  digitalWrite(CS, HIGH);
  digitalWrite(CLK, LOW);
  delay(2000);
}



int measure_average(){
  int average = 0;
  for (int j = 0; j < 16; j++)
  {
    average += getSerialPosition(CLK, CS, DO);
  }
  average /= 16;
  return average;
}

void serial_init_measure(){
  // Takes 16 measurements and averages them for the starting Serial value (0-4095 value)
  for (int j = 0; j < 16; j++)
  {
    start += getSerialPosition(CLK, CS, DO);
  }
  start /= 16; 
}

// LED Functions

void verify_LED_button(){
  if (button)
  {
    uint32_t ledCheck = getErrorBits(CLK, CS, DO);

    if (ledCheck >= 16 && ledCheck <= 18)
      Loom.Neopixel().set_color(2, 0, 200, 0, 0); // Green
    else if (ledCheck == 19)
      Loom.Neopixel().set_color(2, 0, 200, 200, 0); // Yellow
    else
      Loom.Neopixel().set_color(2, 0, 0, 200, 0); // Red

    delay(3000);
    Loom.Neopixel().set_color(2, 0, 0, 0, 0);
  }
  flag = false;
  button = false;
}

void verify_position(){
  uint32_t ledCheck = getErrorBits(CLK, CS, DO); // Tracking magnet position for indicator

  while (ledCheck < 16 || ledCheck > 18)
  {
    if (ledCheck == 19)
      Loom.Neopixel().set_color(2, 0, 200, 200, 0); // Changes Neopixel to yellow
    else
      Loom.Neopixel().set_color(2, 0, 0, 200, 0); // Changes Neopixel to red

    delay(3000); // Gives user 3 seconds to adjust magnet before next reading
    ledCheck = getErrorBits(CLK, CS, DO);
  }
}

void green_flash(){
  for (int i = 0; i < 3; i++)
  {

    Loom.Neopixel().set_color(2, 0, 200, 0, 0); // Changes Neopixel to green
    delay(500);

    Loom.Neopixel().set_color(2, 0, 0, 0, 0); // Turns off Neopixel
    delay(500);
  }
}

// Interrupt Functions
void ISR_pin12()
{
  detachInterrupt(RTC_INT_PIN);
  flag = true;
}

void ISR_pin11()
{
  detachInterrupt(INT_BUT);
  flag = true;
  button = true;
}
