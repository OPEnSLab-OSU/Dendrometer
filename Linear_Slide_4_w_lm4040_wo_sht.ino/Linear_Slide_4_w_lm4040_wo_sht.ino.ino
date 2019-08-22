#include <SD.h> // SD card library
//#include <SPI.h>
#include <Wire.h>
#include "RTClib.h" // real time clock library
//#include "Adafruit_SHT31.h" // temp humidity library

#define analogPin 0 
#define maxVal 1024 //based on 10 bit adc in feather
#define avgTime 100 //time between readings for average
#define _pause 300000 //time to wait before beginning to read again

const int chipSelect = 10; // for SD card
float rawValue; // to store analog value read from sensor
float slopeInc=(12.7/1024); // for every .01246 mm moved, pin reads increment of 1

float distTrav; // to store converted analog to distance in mm
float trunkRadius = 0; // calculated with first value read upon start up. Data stored is then relative to this point. 


DateTime now; // for RTC 


RTC_PCF8523 rtc; //initializes real time clock
// Adafruit_SHT31 sht31 = Adafruit_SHT31(); //SHT


void setup() {
  analogReference(AR_EXTERNAL);
  digitalWrite(13,LOW); // set LED light off
  Serial.begin(9600); // Open serial communications and wait for port to open
  Wire.begin();
//  
//  while (!Serial) {
//    ; // wait for serial port to connect. Needed for native USB port only
//  }


  Serial.print("Initializing SD card...");

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
  }   else {
  Serial.println("card initialized.");
  }
  delay(1000);
  Serial.println("Checking for existing file...");
  
  if (SD.exists("data.txt")) {
    
    Serial.print("Deleting file..."); 
    SD.remove("data.txt");
    Serial.println("File Deleted");
    
  }  else {
    
    Serial.println("No File found.");
    
  }
  digitalWrite(13,HIGH); //turn light on to show sensor is taking zero point
  Serial.println("Taking zero...");
  delay(3000);
 
  trunkRadius = analogRead(analogPin); // comment out to get rid of callibration zero upon startup
  trunkRadius = (maxVal - trunkRadius) * slopeInc; // subtracts to get distance from center and converts to distance unit, mm
  digitalWrite(13,LOW); // turn light off to show sensor is done taking zero point

//     if (! sht31.begin(0x44)) {   //Initializes temperature/humidity sensor
//        Serial.println("Couldn't find SHT31"); 
////        digitalWrite(13,HIGH); //turns LED on if sensor cannot be initialized
//        while (1) delay(1);
//         } 
//            
 //sht31.heater(false);
}

void loop() {
  delay(_pause);
  digitalWrite(13,HIGH);
    delay(100); 
  digitalWrite(13,LOW);
  float dataTotal = 0;
  float calcAvgVal; // stores total for taking average
//  float temp = sht31.readTemperature(); //read temp
//  float hmdty = sht31.readHumidity(); //read humidity

  // read sensor 3 times and append to the string:
  for (int i=0; i < 30; i++) {
      rawValue = analogRead(analogPin); // stores sensor reading
      //distTrav = rawValue*slopeInc; //converts to distance unit
       distTrav = (maxVal - rawValue) * slopeInc; // subtracts to get the distance from center(gets rid of negative values for growth) and converts to distance unit, mm, use line above if negative
      delay(avgTime); //adds delay
      dataTotal = distTrav+dataTotal; // add to total for the average
    
  }


  //take average, 
  calcAvgVal=(dataTotal/30); //1. Calculates average by taking the sum of previous 3 sensor values and dividing by 3
  calcAvgVal= calcAvgVal - trunkRadius; // 2. uncomment to use callibration constant, trunkRadius
  
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
    dataFile.println(calcAvgVal,4); //write average data to file
//    dataFile.print(",");
//    dataFile.print(temp);
//    dataFile.print(",");
//    dataFile.println(hmdty);

    dataFile.close(); //close file
     //print to serial port
    
//    Serial.print(now.month());
//    Serial.print("-");
//    Serial.print(now.day());
//    Serial.print("-");
//    Serial.print(now.year());
//    Serial.print(" ");

    Serial.print("Time:");
    Serial.print(now.hour(), DEC);
    Serial.print(":");
    Serial.print(now.minute(),DEC);
    Serial.print(":");
    Serial.print(now.second(),DEC);
    Serial.print("  Distance:");
    Serial.println(calcAvgVal,4);
//    Serial.print(", temp:");
//    Serial.print(temp);
//    Serial.print(", humidity:");
//    Serial.println(hmdty);

    
  }

  else {
    Serial.println("error opening datalog.txt"); //error if file writing doesn't work or data file isn't available
  }

}
