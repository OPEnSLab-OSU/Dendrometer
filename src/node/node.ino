#include <Arduino.h>

#include <Loom_Manager.h>

#include <Hardware/Loom_Hypnos/Loom_Hypnos.h>
#include <Hardware/Actuators/Loom_Neopixel/Loom_Neopixel.h>
#include <Sensors/Loom_Analog/Loom_Analog.h>
#include <Sensors/I2C/Loom_SHT31/Loom_SHT31.h>
#include <Radio/Loom_LoRa/Loom_LoRa.h>

#include "AS5311.h"

static const char *DEVICE_NAME = "DendrometerV4";
static const uint8_t DENDROMETER_NODE_ID = 123;
// These two time values are added together to determine the interval time
static const int8_t MEASUREMENT_INTERVAL_MINUTES = 15;
static const int8_t MEASUREMENT_INTERVAL_SECONDS = 0;
static const uint8_t TRANSMIT_INTERVAL = 16; // to save power, only transmit a packet every X measurements

// Pins
#define AS5311_CS A3
#define AS5311_CLK A5
#define AS5311_DO A4
#define LED_PIN A2

#define BUTTON_PIN A1
#define RTC_INT_PIN 12

Manager manager(DEVICE_NAME, DENDROMETER_NODE_ID);
Loom_Hypnos hypnos(manager, HYPNOS_VERSION::V3_3, TIME_ZONE::PST);

Loom_Analog analog(manager);
Loom_SHT31 sht(manager);
Loom_Neopixel statusLight(manager);
Loom_LoRa lora(manager, DENDROMETER_NODE_ID); 
AS5311 magnetSensor(AS5311_CS, AS5311_CLK, AS5311_DO);

// Global Variables
volatile bool buttonPressed = false; // Check to see if button was pressed
uint16_t initialMeasurement = 0;

void sleepCycle();
void ISR_RTC();
void ISR_BUT();

void takeMeasurements();
void recordMagnetSensor();
void recordTempHumidSensor();

void alignMagnetSensor();
bool checkStableAlignment();
void displayMagnetStatus(magnetStatus);
void flashGreen();

void ISR_RTC()
{
    detachInterrupt(BUTTON_PIN);
    detachInterrupt(RTC_INT_PIN);
    hypnos.wakeup();
}

void ISR_BUTTON()
{
    detachInterrupt(BUTTON_PIN);
    detachInterrupt(RTC_INT_PIN);
    hypnos.wakeup();
    buttonPressed = true;
}

void setup()
{
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);
    // Enable pullup on button pin - this is necessary for interrupt
    pinMode(BUTTON_PIN, INPUT_PULLUP);

    manager.beginSerial();
    hypnos.setLogName("Dendrometer");
    hypnos.enable();
    // Initialize the modules
    manager.initialize();

    alignMagnetSensor();
    initialMeasurement = magnetSensor.getFilteredPosition();

    // Register interrupts
    hypnos.registerInterrupt(ISR_RTC);
    hypnos.registerInterrupt(ISR_BUTTON, BUTTON_PIN);
}

void loop()
{
    takeMeasurements();

    if (buttonPressed) // if interrupt button was pressed, display staus of magnet sensor
    {
        buttonPressed = false;
        displayMagnetStatus(magnetSensor.getMagnetStatus());
        delay(3000);
        statusLight.set_color(2, 0, 0, 0, 0); // LED Off
    }

    static uint8_t loopCounter = 14;

    loopCounter++;
    // Send data to hub after set amount of packets
    // Note that this counter in incremented when the button is pressed
    if (loopCounter >= TRANSMIT_INTERVAL)
    {
        lora.send(0);
        loopCounter = 0;
    }

    sleepCycle();
}

void sleepCycle()
{
    // deinitalize modules in order to enter sleep safely
    manager.power_down();
    digitalWrite(LED_BUILTIN, LOW);

    hypnos.setInterruptDuration(TimeSpan(0, 0, MEASUREMENT_INTERVAL_MINUTES, MEASUREMENT_INTERVAL_SECONDS));
    // Reattach to the interrupt after we have set the alarm so we can have repeat triggers
    hypnos.reattachRTCInterrupt();
    hypnos.reattachRTCInterrupt(BUTTON_PIN);

    // Put the device into a deep sleep, operation HALTS here until the interrupt is triggered
    hypnos.sleep();

    // initialize modules before running next loop iteration
    digitalWrite(LED_BUILTIN, HIGH);
    manager.power_up();
}

void takeMeasurements()
{
    manager.measure();
    manager.package();

    recordMagnetSensor();
    recordTempHumidSensor();

    // Log whether system woke up from button or not
    manager.addData("Button", "Pressed?", buttonPressed);

    if (Serial)
        manager.display_data();

    hypnos.logToSD();
}

void recordMagnetSensor()
{
    //static float previousDistance_mm = 0;
    //static float previousDistance_um = 0;

    int32_t average = (int32_t)magnetSensor.getFilteredPosition();

    manager.addData("AS5311", "Serial_Value", average);
    // manager.addData("Displacement_mm", "mm", distance_mm);
    // manager.addData("Displacement_um", "um", distanceMicro_um);
    // manager.addData("Difference_mm", "mm", difference_mm);
    // manager.addData("Difference_um", "um", difference_um);

    magnetStatus status = magnetSensor.getMagnetStatus();
    switch (status)
    {
    case magnetStatus::error:
        manager.addData("Magnet Alignment", "", "Error");
        break;
    case magnetStatus::red:
        manager.addData("Magnet Alignment", "", "Not aligned");
        break;
    case magnetStatus::yellow:
        manager.addData("Magnet Alignment", "", "Poor");
        break;
    case magnetStatus::green:
        manager.addData("Magnet Alignment", "", "OK");
        break;
    default: // do nothing
        break;
    }
}

void recordTempHumidSensor()
{
    float SVP, VPD, temperature, humidity;
    float e = 2.71828;

    temperature = sht.getTemperature();
    humidity = sht.getHumidity();

    SVP = (0.61078 * pow(e, (17.2694 * temperature) / (temperature + 237.3)));
    VPD = SVP * (1 - (humidity / 100));

    manager.addData("VPD", "VPD", VPD);
}

void alignMagnetSensor()
{
    magnetStatus status;
    while (1)
    {
        status = magnetSensor.getMagnetStatus();
        displayMagnetStatus(status);
        delay(100);
        if (status == magnetStatus::green && checkStableAlignment())
            break;
    }
    flashGreen();
}

void displayMagnetStatus(magnetStatus status)
{
    switch (status)
    {
    case magnetStatus::yellow:
        statusLight.set_color(2, 0, 255, 255, 0); // yellow
        break;
    case magnetStatus::green:
        statusLight.set_color(2, 0, 255, 0, 0); // green
        break;
    case magnetStatus::error: // Fall through
    case magnetStatus::red:   // Fall through
    default:
        statusLight.set_color(2, 0, 0, 255, 0); // red
        break;                                  // do nothing
    }
}

// Flashes status light green
void flashGreen()
{
    for (int i = 0; i < 6; i++)
    {
        statusLight.set_color(2, 0, 255, 0, 0); // green
        delay(250);
        statusLight.set_color(2, 0, 0, 0, 0); // off
        delay(250);
    }
}

/**
 * Make sure calibration is stable before proceeding
 * Returns true if the sensor remains aligned for the next five seconds
 */
bool checkStableAlignment()
{
    const unsigned int CHECK_TIME = 5000;
    magnetStatus status;
    bool aligned = true;

    for (int i = 0; i < (CHECK_TIME / 100); i++)
    {
        status = magnetSensor.getMagnetStatus();
        if (status != magnetStatus::green)
        {
            aligned = false;
            break;
        }
        delay(100);
    }

    return aligned;
}