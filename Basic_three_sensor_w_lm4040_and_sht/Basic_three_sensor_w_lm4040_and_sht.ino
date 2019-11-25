#include <SD.h> // SD card library
#include <SPI.h>
#include <Wire.h>
#include "RTClib.h" // real time clock library
#include "Adafruit_SHT31.h" // temp humidity library


#define analogPin 0
#define analogPin2 1
#define analogPin3 2
#define _pause 100
#define maxVal 1024 //based on 10 bit adc in feather
#define avgTime 100000 //time between readings for average

const int chipSelect = 10;
float rawValue; 
float rawValue2;
float rawValue3;
float crap;

float slopeInc3048=(0.012414); // for every .01246 mm moved, pin reads increment of 1, 12.7/1023
float slopeIncLMC=(0.017527); // for LMC , 11/1023
float slopeInc9610=(0.024829); // for 9600 series, 25.4/1023

float distTrav;
float distTrav2; 
float distTrav3; // to store converted analog to distance in mm\

DateTime now; // for RTC

RTC_PCF8523 rtc; //initializes real time clock
Adafruit_SHT31 sht31 = Adafruit_SHT31(); //SHT

  void setup() {
  
    analogReference(AR_EXTERNAL); //for external analog reference
    digitalWrite(13,LOW);
    Serial.begin(9600);
    Wire.begin();

//    while(!Serial) {
//    ;// wait for serial 
//    }

    
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

     if (! sht31.begin(0x44)) {   //Initializes temperature/humidity sensor
        Serial.println("Couldn't find SHT31"); 
//        digitalWrite(13,HIGH); //turns LED on if sensor cannot be initialized
        while (1) delay(1);
         }
    
  }




void loop() {
         
        delay(_pause);
        digitalWrite(13,HIGH);
        delay(100);
        digitalWrite(13,LOW);

        float dataTotal = 0;
        float dataTotal2 = 0;
        float dataTotal3 = 0;
        float calcAvgVal;
        float calcAvgVal2;
        float calcAvgVal3;




// P3 sensor:       
      for (int i=0; i < 30; i++) {
            delay(10);
            crap = analogRead(analogPin);
           // Serial.println(testValue);
            delay(10);
            rawValue = analogRead(analogPin); // stores sensor reading
            distTrav = rawValue*slopeIncLMC; //converts to distance unit
            //distTrav = (maxVal - rawValue) * slopeInc; // subtracts to get the distance from center(gets rid of negative values for growth) and converts to distance unit, mm, use line above if negative
      
            dataTotal = distTrav+dataTotal; // add to total for the average
           delay(10);
        }


  // 9600 series:
      for (int i=0; i < 30; i++) {
            delay(10);
            crap = analogRead(analogPin2);
           // Serial.println(testValue);
            delay(10);
            rawValue2 = analogRead(analogPin2); // stores sensor reading
            distTrav2 = rawValue2*slopeInc9610; //converts to distance unit
            //distTrav = (maxVal - rawValue) * slopeInc; // subtracts to get the distance from center(gets rid of negative values for growth) and converts to distance unit, mm, use line above if negative
      
            dataTotal2 = distTrav2+dataTotal2; // add to total for the average
            delay(10);
        }



  //lm3048

        for (int i=0; i < 30; i++) {
            delay(10);
            crap = analogRead(analogPin3);
           // Serial.println(testValue);
            delay(10);
            rawValue3 = analogRead(analogPin3); // stores sensor reading
            distTrav3 = rawValue3*slopeInc3048; //converts to distance unit
            //distTrav = (maxVal - rawValue) * slopeInc; // subtracts to get the distance from center(gets rid of negative values for growth) and converts to distance unit, mm, use line above if negative
      
            dataTotal3 = distTrav3+dataTotal3; // add to total for the average
           delay(10);
        }

  calcAvgVal=((dataTotal)/30); //1. Calculates average by taking the sum of previous 3 sensor values and dividing by 3
  calcAvgVal2=((dataTotal2)/30);
  calcAvgVal3=((dataTotal3)/30);
 delay(avgTime);
        
//        delay(10);
//        crap = analogRead(analogPin);
//        delay(10);
//        rawValue = analogRead(analogPin);
        
//        delay(10);
//        crap = analogRead(analogPin2);
//        delay(10);
//        rawValue2 = analogRead(analogPin2);
//        
//        delay(10);
//        crap = analogRead(analogPin3);
//        delay(10);
//        rawValue3 = analogRead(analogPin3);
//        
//        distTrav = rawValue*slopeIncLMC;
//        distTrav2 = rawValue2*slopeInc9610;
//        distTrav3 = rawValue3*slopeInc3048;


float temp = sht31.readTemperature(); //read temp
float hmdty = sht31.readHumidity(); //read humidity



    Serial.print("Time:");
    Serial.print(now.hour(), DEC);
    Serial.print(":");
    Serial.print(now.minute(),DEC);
    Serial.print(":");
    Serial.print(now.second(),DEC);
    Serial.print("  P3:");
    Serial.print(calcAvgVal,4);
    Serial.print("  9600 series: ");
    Serial.print(calcAvgVal2,4);
    Serial.print("  lm3048: ");
    Serial.print(calcAvgVal3,4);
    Serial.print(", temp:");
    Serial.print(temp);
    Serial.print(", humidity:");
    Serial.println(hmdty);




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

    dataFile.print(calcAvgVal2,4); //
    dataFile.print(",");

    dataFile.print(calcAvgVal3,4); 
    dataFile.print(",");
    
    dataFile.print(temp);
    dataFile.print(",");
    dataFile.println(hmdty);

    dataFile.close();
  }
  else {
    Serial.println("error opening datalog.txt"); //error if file writing doesn't work or data file isn't available
  }


}
