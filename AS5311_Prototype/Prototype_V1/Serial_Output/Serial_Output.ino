#define DELAY_IN_SECONDS 3

#include "AS5311.h"

#define CS 9
#define CLK A5
#define DO A4

#define HYPNOS3 5   // Hypnos 3V Rail
#define HYPNOS5 6   // Hypnos 5V Rail

uint32_t start = 0;
uint32_t prevTwoSig = 0;
float elapsed = 0;
float prev = 0;
float prevMicro = 0;
int count = 0;

void setup() {

  while(!Serial) {}           // Gives user time to place magnet before readings start

  pinMode(HYPNOS3, OUTPUT);
  digitalWrite(HYPNOS3, LOW); // Sets pin 5, the pin with the 3.3V rail, to output and enables the rail
  pinMode(HYPNOS5, OUTPUT);
  digitalWrite(HYPNOS5, HIGH); // Sets pin 6, the pin with the 5V rail, to output and enables the rail

  // Pins to communicate with AS5311
  pinMode(CS, OUTPUT);
  pinMode(CLK, OUTPUT);
  pinMode(DO, INPUT_PULLUP);
  digitalWrite(CS, HIGH);
  digitalWrite(CLK, LOW);

  delay(2000);

  // LED indicator
  uint32_t ledCheck = getErrorBits(CLK, CS, DO);                                           // Tracking magnet position for indicator
  
  while(ledCheck < 16 || ledCheck > 18) {

    if (ledCheck >= 16 && ledCheck <= 18) { // Error bits: 10000, 10001, 10010
      Serial.println("Status: Green");
    }
    else if (ledCheck == 19) {               // Error bits: 10011
      Serial.println("Status: Yellow");
   }
   else if (ledCheck == 23) {               // Error bits: 10111
     Serial.println("Status: Red");
   }
   else {
     Serial.println("Status: Error (You should not see this)");
   }

    Serial.println("Magnet is not in a good position. Please try again.");
    delay(3000);                                                                           // Gives user 3 seconds to adjust magnet before next reading

    ledCheck = getErrorBits(CLK, CS, DO);

  }

  digitalWrite(HYPNOS3, HIGH);

  // Takes 16 measurements and averages them for the starting Serial value (0-4095 value)
  for(int j = 0; j < 16; j++)
  {
    start += getSerialPosition(CLK, CS, DO);
  }
  start /= 16;

  Serial.println("Starting Serial Value: " + String(start));

  // Save 2 most significant bits of start
  prevTwoSig = start & 0xC00;

}

// Notes:
/*
    - It takes a minimum of 56 ms to take a one point reading
*/

void loop() {

  int actualMiliSeconds = (DELAY_IN_SECONDS - 1) * 1000;
  delay(actualMiliSeconds);

  // delay for another 1 sec in total
  delay(104);

  // Note: Sensor will take 16 points over the last 896 seconds
  // TODO: Verify how quickly these points are being taken
  // 16 point average of Serial Position
  // 56ms * 16 = 896ms

  uint32_t average = 0;   // Average of 16 measurements
//  int remember = 0;  // Previous measured value
//  int test = 0;      // Current measured value

  for(int j = 0; j < 16; j++)
  {
    average += getSerialPosition(CLK, CS, DO);
    /*
 // For troubleshooting
    if (remember == 0) {
      Serial.println("Difference: " + String(average));
    }
    else {
      test = average - remember;
      Serial.println("Difference: " + String(test));
    }
    remember = average;
  */

  }

  average /= 16;
  Serial.println("Average: " + String(average));

  uint32_t errorBits = getErrorBits(CLK, CS, DO);                                           // Tracking error bits
//  Serial.println("Error Bit Value: " + String(errorBits));

  // Also updates prevTwoSig to two most significant bits of first param, is being passed by ref
  // Verifies that reading is somewhat accurate
  elapsed = computeElapsed(average, prevTwoSig, elapsed);
//  Serial.println("Elapsed: " + String(elapsed));

  // Computes total distance
  float distance = (elapsed + ((2.0 * ((int) average - (int) start))/4096.0));               // Distance in millimeters
  float distanceMicro = (elapsed * 1000) + ((2000 * ((int) average - (int) start))/4096.0);  // Distance in micrometers
  float difference = 0;                                                                      // Millimeters since last measurement
  float differenceMicro = 0;                                                                 // Micrometers since last measurement

  // Reads the movement if any, else it sets the changed distance to 0
  if (distance != prev) {
    difference = distance - prev;
  }

  if (distanceMicro != prevMicro) {
    differenceMicro = distanceMicro - prevMicro;
  }

  Serial.println("=============== " + String(count) + " ===============");
  Serial.println("Changed Distance mm: " + String(difference) + "mm");
  Serial.println("Changed Distance um: " + String(differenceMicro) + "um");
  Serial.println("Total distance: " + String(distance) + "mm");
  Serial.println("Total distance um: " + String(distanceMicro) + "um");

  // Prints the status of the magnet position (whether the data is good or not) {Green = Good readings, Red = Bad readings}
  // "Error" occurs when something other than magnet placement causes a problem
  // Ignores the parity bit (last bit)
  if (errorBits >= 16 && errorBits <= 18) { // Error bits: 10000, 10001, 10010
    Serial.println("Status: Green");
  }
  else if (errorBits == 19) {               // Error bits: 10011
    Serial.println("Status: Yellow");
  }
  else if (errorBits == 23) {               // Error bits: 10111
    Serial.println("Status: Red");
  }
  else {
    Serial.println("Status: Error (You should not see this)");
  }

  Serial.println("=============== " + String(count) + " ===============");

  Serial.println();
  Serial.println();
  Serial.println();
  prev = distance;
  prevMicro = distanceMicro;
  count += 1;

}