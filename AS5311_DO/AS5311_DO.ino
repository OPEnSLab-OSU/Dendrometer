#include <SPI.h>
#include "AS5311.h"

const uint16_t _clock = 12;        // clock pin: output from arduino to as5311
const uint16_t _cs = 11;           // chip select: output
const uint16_t _data = 13;         // data pin: input
const uint16_t _index = 10;    // index pin: input
uint32_t sens = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
pinMode(_clock, OUTPUT);
pinMode(_data, INPUT);
}

/// function to read sensor ///
      uint32_t AS5311::read_chip()
      {
        uint32_t raw_value = 0;
        uint16_t inputstream = 0;
        uint16_t c;
        digitalWrite(_cs, HIGH);
        digitalWrite(_clock, HIGH);
        delay(100);
        digitalWrite(_cs, LOW);
        delay(10);
        digitalWrite(_clock, LOW);
        delay(10);
        for (c = 0; c < 18; c++)
        {
          digitalWrite(_clock, HIGH);
          delay(10);
          inputstream = digitalRead(_data);
          raw_value = ((raw_value << 1) + inputstream);
          digitalWrite(_clock, LOW);
          delay(10);
        }
       return raw_value;
      } 

void loop() {
  // put your main code here, to run repeatedly:
  sens = AS5311::read_chip();
}
