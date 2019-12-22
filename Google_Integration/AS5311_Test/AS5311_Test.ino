#include <dummy.h>

int input = D7;

void setup() {
  // put your setup code here, to run once:
  pinMode(input, INPUT);
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
//  int val = digitalRead(input);
  //PWM Output range 1...4097 microseconds, correlates to 0-2mm
  
  int val = pulseIn(input, HIGH);
  Serial.println(val);
  delay(100);
}
