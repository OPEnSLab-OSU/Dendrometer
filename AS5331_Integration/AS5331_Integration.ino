#include <Adafruit_SHT31.h>

//Adafruit_SHT31 sht31 = Adafruit_SHT31();
void setup() {
//     if (! sht31.begin(0x44)) {   //Initializes temperature/humidity sensor
//        Serial.println("Couldn't find SHT31"); 
//     }
//     Serial.begin(9600);
  pinMode(11, INPUT);
  attachInterrupt(digitalPinToInterrupt(11), test, RISING);
  Serial.begin(9600);
}

void loop() {
//  float temp = sht31.readTemperature(); //read temp
//  float hmdty = sht31.readHumidity(); //read humidity
//  Serial.println(temp);
// Serial.println(hmdty);
//  delay(1000);

//  int val = pulseIn(11, HIGH);
//  Serial.println(val);
//  delay(100);
}

void test() {
  Serial.println("HIT");
}
