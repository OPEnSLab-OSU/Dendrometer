#include <SD.h>
#include <SPI.h>
#include <Wire.h>
#include "RTClib.h"

#define analogPin 0
#define maxVal 1024

const int chipSelect = 10;
float rawValue;
float slopeInc=(12.7/1024); // for every .01246 mm moved, pin reads increment of 1
float distTrav;
int dataString[] = {0, 0, 0};

DateTime now;

//
RTC_PCF8523 rtc; //initializes real time clock
//DateTime now = rtc.now();
//DateTime now; //sets time

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
     Wire.begin();
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }


  Serial.print("Initializing SD card...");

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
  }
  Serial.println("card initialized.");
  delay(1000);
}

void loop() {
float  dataTotal=0;
float calcAvgVal; 

  // read sensor 3 times and and append to the string:
  for (int i=0; i < 3; i++) {
      rawValue = analogRead(analogPin);
      distTrav = (maxVal - rawValue) * slopeInc;
      delay(500); //delays 1.5 seconds between recording data
//    delay(300000); //delays 5 minutes between readings, 15 minutes between recording data
      dataTotal = distTrav+dataTotal;
    
  }

  //take average 
  calcAvgVal=(dataTotal/3);
  
  File dataFile = SD.open("data.txt", FILE_WRITE); //open file

  // if the file is available, write to it:
  if (dataFile) {
   
      now=rtc.now();
    dataFile.print(now.month());
    dataFile.print("-");
    dataFile.print(now.day());
    dataFile.print("-");
    dataFile.print(now.year());
    dataFile.print(" ");
    dataFile.print(now.hour(), DEC);
    dataFile.print(":");
    dataFile.print(now.minute(), DEC);
    dataFile.print(":");
    dataFile.print(now.second(), DEC);
    dataFile.print(",");
    dataFile.println(calcAvgVal); //write data to file

    dataFile.close(); //close file
     //print to serial port
    
    Serial.print(now.month());
    Serial.print("-");
    Serial.print(now.day());
    Serial.print("-");
    Serial.print(now.year());
    Serial.print(" ");
    Serial.print(now.hour(), DEC);
    Serial.print(":");
    Serial.print(now.minute(),DEC);
    Serial.print(":");
    Serial.print(now.second(),DEC);
    Serial.print(",");
    Serial.println(calcAvgVal);
  }

  else {
    Serial.println("error opening datalog.txt"); //error if file doesn't work
  }
  delay(2000);
}
