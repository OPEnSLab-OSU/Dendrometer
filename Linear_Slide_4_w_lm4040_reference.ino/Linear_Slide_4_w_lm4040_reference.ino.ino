#include <SD.h> // SD card library
#include <SPI.h>
#include <Wire.h>
#include "RTClib.h" // real time clock library
#include "Adafruit_SHT31.h" // temp humidity library

#define analogPin 0 
#define analogPinTwo 2
#define maxVal 1024 //based on 10 bit adc in feather
#define avgTime 100 //time between readings for average
#define _pause 1000 //time to wait before beginning to read again

const int chipSelect = 10; // for SD card
float rawValue; 
float rawValueTwo; // to store analog value read from sensor

float slopeInc=(12.7/1023); // for every .01246 mm moved, pin reads increment of 1
float slopeIncLMC= (11/1023); // for LMC 
float slopeInc9610=(25.4/1023); // for 9600 series

float distTrav;
float distTravTwo; // to store converted analog to distance in mm
float trunkRadius = 0; // calculated with first value read upon start up. Data stored is then relative to this point. 
float trunkRadiusTwo = 0; // calculated first value of reference dendrometer upon start up. 

DateTime now; // for RTC
 


RTC_PCF8523 rtc; //initializes real time clock
Adafruit_SHT31 sht31 = Adafruit_SHT31(); //SHT


void setup() {
  analogReference(AR_EXTERNAL);
  pinMode(analogPin, INPUT);
  pinMode(analogPinTwo, INPUT);
  digitalWrite(13,LOW); // set LED light off
  Serial.begin(9600); // Open serial communications and wait for port to open
  Wire.begin();
  
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }


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

    SD.remove("data.txt");
    Serial.println("File Deleted");
    
  }  else {
    
    Serial.println("No File found.");
    
  }
  digitalWrite(13,HIGH); //turn light on to show sensor is taking zero point
  Serial.println("Taking zero...");
  delay(3000);
 
  trunkRadius = analogRead(analogPin); // comment out to get rid of callibration zero upon startup
  //trunkRadius = (maxVal - trunkRadius) * slopeInc; // subtracts to get distance from center and converts to distance unit, mm
  trunkRadius = (trunkRadius*slopeIncLMC); //subtracts for when negative numbers are being given, use either this line or above line



  //trunkRadiusTwo = analogRead(analogPinTwo);
  //trunkRadiusTwo = (trunkRadiusTwo*slopeInc9610); //converts to distance unit


  digitalWrite(13,LOW); // turn light off to show sensor is done taking zero point
  
  
     if (! sht31.begin(0x44)) {   //Initializes temperature/humidity sensor
        Serial.println("Couldn't find SHT31"); 
//        digitalWrite(13,HIGH); //turns LED on if sensor cannot be initialized
        while (1) delay(1);
         } 
            
 //sht31.heater(false);
}

void loop() {
  delay(_pause);
  digitalWrite(13,HIGH);
    delay(100); 
  digitalWrite(13,LOW);
  float dataTotal = 0;
  float dataTotalTwo = 0;
  float calcAvgVal;
  float calcAvgValTwo; // stores total for taking average
  float temp = sht31.readTemperature(); //read temp
  float hmdty = sht31.readHumidity(); //read humidity

  // read sensor 3 times and add to total:
//  for (int i=0; i < 30; i++) {
//     
//      rawValue = analogRead(analogPin); // stores sensor reading
//      distTrav = rawValue*slopeIncLMC; //converts to distance unit
//      //distTrav = (maxVal - rawValue) * slopeInc; // subtracts to get the distance from center(gets rid of negative values for growth) and converts to distance unit, mm, use line above if negative
//      delay(avgTime); //adds delay
//      dataTotal = distTrav+dataTotal; // add to total for the average
//    
//  }


delay(100);

//for (int l=0; l < 30; l++) {
//      delay(10);
//      //testValue = analogRead(analogPinTwo);
//     // Serial.println(testValue);
//      delay(10);
//      rawValueTwo = analogRead(analogPinTwo); // stores sensor reading
//      distTravTwo = rawValueTwo*slopeInc9610; //converts to distance unit
//      //distTrav = (maxVal - rawValue) * slopeInc; // subtracts to get the distance from center(gets rid of negative values for growth) and converts to distance unit, mm, use line above if negative
//
//      dataTotalTwo = distTravTwo+dataTotalTwo; // add to total for the average
//     delay(avgTime);
//  }



//  //take average, 
//  calcAvgVal=(dataTotal/30); //1. Calculates average by taking the sum of previous 3 sensor values and dividing by 3
//  calcAvgVal= calcAvgVal - trunkRadius; // 2. uncomment to use callibration constant, trunkRadius
//
//
//  calcAvgValTwo = (dataTotalTwo/30);
//  calcAvgValTwo = calcAvgValTwo-trunkRadiusTwo;

  rawValue = analogRead(analogPin);
  distTrav = rawValue*slopeIncLMC;
  calcAvgVal = distTrav-trunkRadius;

  rawValueTwo = analogRead(analogPinTwo);
  distTravTwo = rawValueTwo*slopeInc9610;
  calcAvgValTwo = distTravTwo-trunkRadiusTwo;
  
  
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
    
    dataFile.print(calcAvgVal,4); //write average data to file
    dataFile.print(",");

    dataFile.print(calcAvgValTwo,4); //write second average data to file
    dataFile.print(",");
    
    dataFile.print(temp);
    dataFile.print(",");
    dataFile.println(hmdty);

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
    Serial.print("  P3:");
    Serial.print(calcAvgVal,4);
    Serial.print("  9600 series: ");
    Serial.print(calcAvgValTwo,4);
    Serial.print(", temp:");
    Serial.print(temp);
    Serial.print(", humidity:");
    Serial.println(hmdty);

    
  }

  else {
    Serial.println("error opening datalog.txt"); //error if file writing doesn't work or data file isn't available
  }

}
