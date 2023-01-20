#include <Arduino.h>

//////////////////////////
/* DEVICE CONFIGURATION */
//////////////////////////
static const String DEVICE_NAME = "Dend4";
static const uint8_t NODE_NUMBER = 123;
// These two time values are added together to determine the interval time
static const int8_t MEASUREMENT_INTERVAL_MINUTES = 15;
static const int8_t MEASUREMENT_INTERVAL_SECONDS = 0;
static const uint8_t TRANSMIT_INTERVAL = 16; // to save power, only transmit a packet every X measurements
//////////////////////////
//////////////////////////
//////////////////////////
#define DISABLE_LORA_TX // DEBUG ONLY. REMOVE WHEN NOT TESTING

#include <Loom_Manager.h>
#include <Hardware/Loom_Hypnos/Loom_Hypnos.h>
#include <Hardware/Actuators/Loom_Neopixel/Loom_Neopixel.h>
#include <Sensors/Loom_Analog/Loom_Analog.h>
#include <Sensors/I2C/Loom_SHT31/Loom_SHT31.h>
#include <Radio/Loom_LoRa/Loom_LoRa.h>

#include "AS5311.h"

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
int initialMagnetPosition = 0;
volatile bool buttonPressed = false; // Check to see if button was pressed

void sleepCycle();
void ISR_RTC();
void ISR_BUTTON();

void takeMeasurements();
void recordMagnetSensor();
void recordTempHumidSensor();

void checkMagnetSensor();
void alignMagnetSensor();
bool checkStableAlignment();
void displayMagnetStatus(magnetStatus);
void flashColor(uint8_t r, uint8_t g, uint8_t b);

void ISR_RTC()
{
    hypnos.wakeup();
}

void ISR_BUTTON()
{
    buttonPressed = true;
    hypnos.wakeup();
}

/**
 * Program setup
 */
void setup()
{
    // Enable pullup on button pin. this is necessary for the interrupt
    pinMode(BUTTON_PIN, INPUT_PULLUP);

    delay(1000);
    // wait for serial connection ONLY button is pressed (low reading)
    manager.beginSerial(!digitalRead(BUTTON_PIN));

    hypnos.setLogName(DEVICE_NAME + "--");
    hypnos.enable();
    manager.initialize();

    checkMagnetSensor();
    alignMagnetSensor();
    initialMagnetPosition = (int)magnetSensor.getFilteredPosition();

    hypnos.registerInterrupt(ISR_RTC);
}

/**
 * Main loop
 */
void loop()
{
    if (magnetSensor.getMagnetStatus() == magnetStatus::green)
        Serial.println(magnetSensor.getFilteredPosition());
    delay(50);
    return;

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
#ifndef DISABLE_LORA_TX
        lora.send(0);
#endif
        loopCounter = 0;
    }

    sleepCycle();
}

/**
 * Shut down the device for a specified time period to save power.
 */
void sleepCycle()
{
    hypnos.setInterruptDuration(TimeSpan(0, 0, MEASUREMENT_INTERVAL_MINUTES, MEASUREMENT_INTERVAL_SECONDS));
    // Reattach to the interrupt after we have set the alarm so we can have repeat triggers
    attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), ISR_BUTTON, FALLING);
    hypnos.reattachRTCInterrupt();

    // Put the device into a deep sleep, operation HALTS here until the interrupt is triggered
    hypnos.sleep();
    detachInterrupt(digitalPinToInterrupt(BUTTON_PIN));
}

/**
 * Perform all measurements for the dendrometer and put them into a packet.
 * Log to SD card.
 */
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

/**
 * Record the data from the magnet sensor, process it, and add it to the packet.
 */
void recordMagnetSensor()
{
/*
    static const int wrap_threshold = 2048;

    int reading = (int)magnetSensor.getFilteredPosition();

    static int lastReading = 0;

    static float displacement_um = 0;

    // STATIC POSITION??? add reading each time?? add difference?

    int difference = reading - lastPosition;
    if (abs(difference) > wrap_threshold)
    {
        if (difference < 0)
        { // high to low overflow
        }
        else
        { // low to high overflow
        }
    }
    else

        manager.addData("AS5311", "Serial_Value", reading);
    static const float um_per_tick = (2000.0 / 4095.0);
    manager.addData("Displacement_um", "um", position * um_per_tick);
    lastPosition = reading;
*/

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
    default:
        manager.addData("Magnet Alignment", "", "Error");
        break;
    }
}

/**
 * Log readings from the SHT30 sensor. Also calculate and log VPD
 */
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

/**
 * Magnet alignment procedure. Displays magnet sensor to user until
 * The magnet is determined to be properly aligned and maintains that alignment
 * for a certain amount of time
 */
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
    flashColor(0, 255, 0);
}

/**
 * Check the magnet sensor alignment status and display it on the multi-color LED
 * @param   status  the magnetStatus to display
 */
void displayMagnetStatus(magnetStatus status)
{
    switch (status)
    {
    case magnetStatus::yellow:
        statusLight.set_color(2, 0, 255, 255, 0); // yellow
        break;
    case magnetStatus::green:
        statusLight.set_color(2, 0, 0, 255, 0); // green
        break;
    case magnetStatus::error: // Fall through
    case magnetStatus::red:   // Fall through
    default:
        statusLight.set_color(2, 0, 255, 0, 0); // red
        break;                                  // do nothing
    }
}

/**
 * Flashes status light
 * @param   r   red color value (unsigned 8 bit number)
 * @param   g   green color value (unsigned 8 bit number)
 * @param   b   blue color value (unsigned 8 bit number)
 */
void flashColor(uint8_t r, uint8_t g, uint8_t b)
{
    for (int i = 0; i < 6; i++)
    {
        statusLight.set_color(2, 0, r, g, b);
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

/**
 * NOT COMPLETELY TESTED.
 * Checks to see if a magnet sensor is connected and functioning.
 */
void checkMagnetSensor()
{
    uint32_t data = magnetSensor.getMagnetRaw();
    // Serial.print("Checking magnet sensor... serial data is: ");
    // Serial.println(data, BIN);
    // Serial.print("Even parity? ");
    // Serial.println(__builtin_parity(data) == 0);
    if(__builtin_parity(data) == 0 && data != 0) //__builtin_parity() returns 0 if value has even parity
        return;
    while (1)
    {
        flashColor(255, 100, 0);
    }
}