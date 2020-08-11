#define DELAY_IN_SECONDS 3

#include "AS5311.h"

#define CS 9
#define CLK A5
#define DO A4
//#define PULSE 11

#define HYPNOS3 5   // Hypnos 3V Rail
#define HYPNOS5 6   // Hypnos 5V Rail

uint32_t start = 0;
uint32_t prevTwoSig = 0;
float elapsed = 0;
float prev = 0;
int count = 0;

void setup() {

  while(!Serial) {} // Gives user time to put magnet in place before opening Serial Monitor

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

  int average = 0;   // Average of 16 measurements
  // int remember = 0;  // Previous measured value
  // int test = 0;      // Current measured value

  for(int j = 0; j < 16; j++)
  {
    average += getSerialPosition(CLK, CS, DO);
/* // For troubleshooting
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
  // Serial.println("Average: " + String(average));

  // Also updates prevTwoSig to two most significant bits of first param, is being passed by ref
  // Verifies that reading is somewhat accurate
  elapsed = computeElapsed(average, prevTwoSig, elapsed);
  // Serial.println("Elapsed: " + String(elapsed));

  // Computes total distance
  float distance = (elapsed + ((2.0 * ((int) average - (int) start))/4095.0));               // Distance in millimeters
  float distanceMicro = (elapsed * 1000) + ((2000 * ((int) average - (int) start))/4095.0);  // Distance in micrometers
  float difference = 0;                                                                      // Millimeters since last measurement

  // Reads the movement if any, else it sets the changed distance to 0
  if (distance != prev) {
    difference = distance - prev;
  }

  Serial.println("=============== " + String(count) + " ===============");
  Serial.println("Changed Distance: " + String(difference) + "mm");
  Serial.println("Total distance: " + String(distance) + "mm");
  Serial.println("Total distance um: " + String(distanceMicro) + "um");
  Serial.println("=============== " + String(count) + " ===============");

  Serial.println();
  Serial.println();
  Serial.println();
  prev = distance;
  count += 1;

}