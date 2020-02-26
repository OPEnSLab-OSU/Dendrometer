#include "AS5311.h"

AS5311 myAS5311(13,12,11,10); // data, clock, chip select, index

void setup()  {
  Serial.begin(9600);
    while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
}

void loop()
{
  long value;
  value = myAS5311.encoder_value();
  Serial.print("measured value: ");
  Serial.println(value);
  value = myAS5311.encoder_position();
  Serial.print("measured position: ");
  Serial.println(value);
  if (myAS5311.encoder_error())
  {
    Serial.println("error detected.");
    if (myAS5311.err_value.DECn) Serial.println("DECn error");
    if (myAS5311.err_value.INCn) Serial.println("INCn error");
    if (myAS5311.err_value.COF) Serial.println("COF error");
    if (!myAS5311.err_value.OCF) Serial.println("OCF error");
    if (myAS5311.err_value.LIN) Serial.println("LIN error");
  }
  delay(2000);
}
