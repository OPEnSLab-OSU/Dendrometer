#include <Loom.h>
#include "AS5311.h"

// Include configuration
const char* json_config = 
#include "config.h"
;

// In Tools menu, set:
// Internet  > Disabled
// Sensors   > Enabled
// Radios    > Enabled
// Actuators > Enabled
// Max       > Disabled

using namespace Loom;

Loom::Manager Feather{};

//Pins
#define CS 9
#define CLK A5
#define DO A4
#define LED A2

#define DELAY_IN_SECONDS 0
#define DELAY_IN_MINUTES 15
#define INT_BUT 11
#define RTC_INT_PIN 12

#define HYPNOS3 5
//#define HYPNOS5 6
 
//Global Variables
volatile bool flag = false;   // Interrupt flag
volatile bool button = false; // Check to see if button was pressed
uint32_t start = 0;
uint32_t prevTwoSig = 0;
float elapsed = 0;
float prev = 0;
float prevMicro = 0;

void setup() 
{
  // Enable waiting for RTC interrupt, MUST use a pullup since signal is active low
  pinMode(RTC_INT_PIN, INPUT_PULLUP);  
  pinMode(INT_BUT, INPUT_PULLUP);
  pinMode(HYPNOS3, OUTPUT);    // Enable control of 3.3V rail
  //pinMode(HYPNOS5, OUTPUT);   // Enable control of 5V rail
  pinMode(13, OUTPUT);

  // Initialize Hypnos
  digitalWrite(HYPNOS3, LOW); // Enable 3.3V rail
  //digitalWrite(HYPNOS5, HIGH);  // Enable 5V rail
  digitalWrite(13, HIGH);

  Feather.begin_serial(true);
  Feather.parse_config(json_config);
  Feather.print_config();

  // Begin Communication with AS5311
  init_AS();

  // LED pin set
  pinMode(LED, OUTPUT);

  // Make sure the magnet is positioned correctly
  verify_position();
  
  // Flash three times for verification
  green_flash();
 
  getInterruptManager(Feather).register_ISR(RTC_INT_PIN, ISR_RTC, LOW, ISR_Type::IMMEDIATE);
  getInterruptManager(Feather).register_ISR(INT_BUT, ISR_BUT, LOW, ISR_Type::IMMEDIATE);
  delay(500);
  
  // Starting measurement of AS5311
  serial_init_measure();

  // Save 2 most significant bits of start
  prevTwoSig = start & 0xC00;
  
  // Turns off Neopixel
  getNeopixel(Feather).set_color(2, 0, 0, 0, 0); 

  LPrintln("\n ** Setup Complete ** ");
}

void loop() 
{
  // Initialize Hypnos
  digitalWrite(HYPNOS3, LOW); // Enable 3.3V rail
  //digitalWrite(HYPNOS5, HIGH);  // Enable 5V rail
  digitalWrite(13, HIGH);

  delay(100);

  // Protocol to turn on AS5311
  pinMode(CS, OUTPUT);
  pinMode(CLK, OUTPUT);
  pinMode(DO, INPUT_PULLUP);
  digitalWrite(CS, HIGH);
  digitalWrite(CLK, LOW);

  // Protocol to turn on Neopixel
  pinMode(LED, OUTPUT);

  // Initialize magnet sensor  
  init_AS();

  if (flag) {
    pinMode(23, OUTPUT);
    pinMode(24, OUTPUT);
    pinMode(10, OUTPUT);
  
    Feather.power_up();
  }

  // Check to see if button was pressed for LED indicator
  verify_LED_button();

  // --- Data Management Begin --- 

  int average = measure_average();
  uint32_t errorBits = getErrorBits(CLK, CS, DO);
  
  // Also updates prevTwoSig to two most significant bits of first param, is being passed by ref
  elapsed = computeElapsed(average, prevTwoSig, elapsed);

  // Computes total distance in mm and um
  float distance = (elapsed + ((2.0 * ((int)average - (int)start)) / 4095.0));
  float distanceMicro = (elapsed * 1000) + ((2000 * ((int)average - (int)start)) / 4095.0);
  float difference = 0;
  float differenceMicro = 0;

  // Reads the movement if any, else it sets the distance to 0
  if (distance != prev)
    difference = distance - prev;

  if (distanceMicro != prevMicro)
    differenceMicro = distanceMicro - prevMicro;

  Feather.measure();
  Feather.package();

  Feather.add_data("AS5311", "Serial_Value", average);
  Feather.add_data("Displacement_mm", "mm", distance);
  Feather.add_data("Displacement_um", "um", distanceMicro);
  Feather.add_data("Difference_mm", "mm", difference);
  Feather.add_data("Difference_um", "um", differenceMicro);

  // Logs the status of the magnet position (whether the data is good or not) {Green = Good readings, Red = Bad readings}
  // Ignores the parity bit (last bit)
  // Datasheet: https://www.mouser.com/pdfdocs/AS5311_Datasheet_EN_v6-914614.pdf
  // Read sections 7.3 and 7.5 of datasheet for more information on error bit values
  
  if (errorBits >= 16 && errorBits <= 18)          // Error bits: 10000, 10001, 10010
    Feather.add_data("Status", "Color", "Green");
  else if (errorBits == 19)                        // Error bits: 10011
    Feather.add_data("Status", "Color", "Yellow");
  else if (errorBits == 23)                        // Error bits: 10111
    Feather.add_data("Status", "Color", "Red");
  else if (errorBits < 16)                         // If OCF Bit is 0
    Feather.add_data("Status", "Color", "OCF_Error");
  else if (errorBits > 24)                         // If COF Bit is 1
    Feather.add_data("Status", "Color", "COF_Error");
  else
    Feather.add_data("Status", "Color", "Other_Error");
  
  // Calculate VPD based on temperature and humidity
  float temp, humid, SVP, VPD;
  float e = 2.71828;

  temp = Feather.get<Loom::SHT31D>()->get_temperature();
  humid = Feather.get<Loom::SHT31D>()->get_humid();
  
  SVP = (0.61078 * pow(e, (17.2694 * temp) / (temp + 237.3)));
  VPD = SVP * (1 - (humid / 100));

  Feather.add_data("VPD", "VPD", VPD);

  // Log RSSI value from LoRa communication
  float rssi = Feather.get<Loom::LoRa>()->get_signal_strength();
  Feather.add_data("RSSI", "RSSI", rssi);

  // Log whether system woke up from button or not
  Feather.add_data("Button", "Pressed?", button);

  prev = distance;
  prevMicro = distanceMicro;

  // --- Data Management End ---

  Feather.display_data();

  // Log SD in case it doesn't send
  getSD(Feather).log();

  // Send data to address 0
  getLoRa(Feather).send(0);

  // Resetting interrupt
  flag = false, button = false;

  getInterruptManager(Feather).RTC_alarm_duration(TimeSpan(0,0,DELAY_IN_MINUTES,DELAY_IN_SECONDS));
  getInterruptManager(Feather).reconnect_interrupt(RTC_INT_PIN);
  getInterruptManager(Feather).reconnect_interrupt(INT_BUT);

  Feather.power_down();

  // Protocol to shut down AS5311
  pinMode(CLK, INPUT);
  pinMode(DO, INPUT);
  pinMode(CS, INPUT);

  // Protocol to shut off Neopixel
  pinMode(LED, INPUT); 

  // Protocol to turn off Hypnos
  digitalWrite(13, LOW);
  digitalWrite(HYPNOS3, HIGH);
  //digitalWrite(HYPNOS5, LOW); 

  // Protocol to shut down SD
  pinMode(23, INPUT);
  pinMode(24, INPUT);
  pinMode(10, INPUT);
  
  getSleepManager(Feather).sleep();
  while (!flag);
}

// --- AS5311 functions ---

// Protocol to turn on AS5311
void init_AS(){
  pinMode(CS, OUTPUT);
  pinMode(CLK, OUTPUT);
  pinMode(DO, INPUT_PULLUP);
  digitalWrite(CS, HIGH);
  digitalWrite(CLK, LOW);
  delay(2000);
}

// Takes 16 data measurements and averages them for normal reading
int measure_average(){
  int average = 0;
  for (int j = 0; j < 16; j++)
  {
    average += getSerialPosition(CLK, CS, DO);
  }
  average /= 16;
  return average;
}

// Takes 16 measurements and averages them for the starting serial value (0-4095 value)
void serial_init_measure(){
  for (int j = 0; j < 16; j++)
  {
    start += getSerialPosition(CLK, CS, DO);
  }
  start /= 16; 
}

// Lights up LED if interrupt button is pressed
void verify_LED_button(){
  if (button)
  {
    uint32_t ledCheck = getErrorBits(CLK, CS, DO);

    if (ledCheck >= 16 && ledCheck <= 18)
      getNeopixel(Feather).set_color(2, 0, 200, 0, 0);   // Green
    else if (ledCheck == 19)
      getNeopixel(Feather).set_color(2, 0, 200, 200, 0); // Yellow
    else
      getNeopixel(Feather).set_color(2, 0, 0, 200, 0);   // Red

    delay(3000);
    getNeopixel(Feather).set_color(2, 0, 0, 0, 0);       // LED Off
  }
}

// Ensures that magnet starts in good position before continuing program
void verify_position(){
  uint32_t ledCheck = getErrorBits(CLK, CS, DO); // Tracking magnet position for indicator

  while (ledCheck < 16 || ledCheck > 18)
  {
    if (ledCheck == 19)
      getNeopixel(Feather).set_color(2, 0, 200, 200, 0); // Changes Neopixel to yellow
    else
      getNeopixel(Feather).set_color(2, 0, 0, 200, 0); // Changes Neopixel to red

    delay(3000); // Gives user 3 seconds to adjust magnet before next reading
    ledCheck = getErrorBits(CLK, CS, DO);
  }
}

// Flashes green on Neopixel 3 times
void green_flash(){
  for (int i = 0; i < 3; i++)
  {

    getNeopixel(Feather).set_color(2, 0, 200, 0, 0); // Changes Neopixel to green
    delay(500);

    getNeopixel(Feather).set_color(2, 0, 0, 0, 0); // Turns off Neopixel
    delay(500);
  }
}

// --- Interrupt Functions ---

// RTC interrupt
void ISR_RTC()
{
  detachInterrupt(RTC_INT_PIN);
  flag = true;
}

// Button interrupt
void ISR_BUT()
{
  detachInterrupt(INT_BUT);
  flag = true;
  button = true;
}
