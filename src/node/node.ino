#include <Arduino.h>

#include <Loom_Manager.h>

#include <Hardware/Loom_Hypnos/Loom_Hypnos.h>
#include <Hardware/Actuators/Loom_Neopixel/Loom_Neopixel.h>
#include <Sensors/Loom_Analog/Loom_Analog.h>
#include <Sensors/I2C/Loom_SHT31/Loom_SHT31.h>
#include <Radio/Loom_LoRa/Loom_LoRa.h>

#include "AS5311.h"

static const String DEVICE_NAME = "Dend4";
static const uint8_t NODE_NUMBER = 123;
// These two time values are added together to determine the interval time
static const int8_t MEASUREMENT_INTERVAL_MINUTES = 15;
static const int8_t MEASUREMENT_INTERVAL_SECONDS = 0;
static const uint8_t TRANSMIT_INTERVAL = 16; // to save power, only transmit a packet every X measurements

// Pins
#define AS5311_CS A3 // 9 for LB version
#define AS5311_CLK A5
#define AS5311_DO A4
#define LED_PIN A2

#define BUTTON_PIN A1
#define RTC_INT_PIN 12

Manager manager(DEVICE_NAME, NODE_NUMBER);
Loom_Hypnos hypnos(manager, HYPNOS_VERSION::V3_3, TIME_ZONE::PST);

Loom_Analog analog(manager);
Loom_SHT31 sht(manager);
Loom_Neopixel statusLight(manager);
Loom_LoRa lora(manager, NODE_NUMBER);
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
    // Enable pullup on button pin - this is necessary for interrupt
    pinMode(BUTTON_PIN, INPUT_PULLUP);

    manager.beginSerial();
    hypnos.setLogName(DEVICE_NAME + "--");
    hypnos.enable();
    manager.initialize();

    alignMagnetSensor();
    initialMeasurement = magnetSensor.getFilteredPosition();

    // Register interrupts
    hypnos.registerInterrupt(ISR_BUTTON, BUTTON_PIN);
    hypnos.registerInterrupt(ISR_RTC);
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

    static uint8_t loopCounter = TRANSMIT_INTERVAL - 2;

    loopCounter++;
    // Send data to hub after set amount of packets
    // Note that this counter in incremented when the button is pressed
    if (loopCounter >= TRANSMIT_INTERVAL)
    {
#warning radio disabled
        // lora.send(0);
        loopCounter = 0;
    }

    sleepCycle();
}

void sleepCycle()
{
    hypnos.setInterruptDuration(TimeSpan(0, 0, MEASUREMENT_INTERVAL_MINUTES, MEASUREMENT_INTERVAL_SECONDS));
    // Reattach to the interrupt after we have set the alarm so we can have repeat triggers
    hypnos.reattachRTCInterrupt(BUTTON_PIN);
    hypnos.reattachRTCInterrupt();

    // Put the device into a deep sleep, operation HALTS here until the interrupt is triggered
    hypnos.sleep();
}

void takeMeasurements()
{
    manager.measure();
    manager.package();

    recordTempHumidSensor();
    recordMagnetSensor();

    // Log whether system woke up from button or not
    manager.addData("Button", "Pressed?", buttonPressed);

    if (Serial)
        manager.display_data();

    hypnos.logToSD();
}

void recordMagnetSensor()
{
    // static float previousDistance_mm = 0;
    // static float previousDistance_um = 0;

    int32_t average = (int32_t)magnetSensor.getFilteredPosition();

    manager.addData("AS5311", "Serial_Value", average);
    // manager.addData("Displacement_mm", "mm", distance_mm);
    // manager.addData("Displacement_um", "um", distanceMicro_um);
    // manager.addData("Difference_mm", "mm", difference_mm);
    // manager.addData("Difference_um", "um", difference_um);

    magnetStatus status = magnetSensor.getMagnetStatus();
    switch (status)
    {
    case magnetStatus::red:
        manager.addData("Magnet Alignment", "", "Red");
        break;
    case magnetStatus::yellow:
        manager.addData("Magnet Alignment", "", "Yellow");
        break;
    case magnetStatus::green:
        manager.addData("Magnet Alignment", "", "Green");
        break;
    case magnetStatus::error: // fall through
    default:                  // do nothing
        manager.addData("Magnet Alignment", "", "Error");
        break;
    }
}

void recordTempHumidSensor()
{
    float SVP, VPD, temperature, humidity;
    float e = 2.71828;

    temperature = sht.getTemperature();
    humidity = sht.getHumidity();

    // Tetens equation
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