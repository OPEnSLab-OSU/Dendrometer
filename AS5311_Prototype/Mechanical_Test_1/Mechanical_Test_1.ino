// Source for Mechanical Iteration 1
//  - Takes AS5311 Sensor measurements every ___ and averages it ____


#include "AS5311.h"

#define CS 9
#define CLK A5
#define DO A4
#define PULSE 11


uint32_t start = 0;
uint32_t prevTwoSig = 0;
float elapsed = 0;
float prev = 0;
int count = 0;
void setup() {
  // put your setup code here, to run once:

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

}

void loop() {
  // put your main code here, to run repeatedly:

  // TODO: Verify how quickly these points are being taken
  // 16 point average of Serial Position
  int average = 0;
  for(int j = 0; j < 16; j++)
  {
    average += getSerialPosition(CLK, CS, DO);
  }
  average /= 16;
  // Serial.println("Average Serial Pos: " + String(average));

  // Also updates prevTwoSig to two most significant bits of first param, is being passed by ref
  elapsed = computeElapsed(average, prevTwoSig, elapsed);

  // Computes total distance
  Serial.println("=============== " + String(count) + " ===============");
  float distance = (elapsed + ((2.0 * ((int) average - (int) start))/4095.0));
  float difference = distance - prev;
  Serial.println("Changed Distance: " + String(difference) + "mm");
  Serial.println("Total distance: " + String(distance) + "mm");
  Serial.println("=============== " + String(count) + " ===============");

  Serial.println();
  Serial.println();
  Serial.println();
  prev = distance;
  count += 1;

}
