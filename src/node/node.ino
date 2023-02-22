#include <Loom_Manager.h>
#include <Hardware/Loom_Hypnos/Loom_Hypnos.h>
#include <Hardware/Actuators/Loom_Neopixel/Loom_Neopixel.h>
#include <Sensors/Loom_Analog/Loom_Analog.h>
#include <Sensors/I2C/Loom_SHT31/Loom_SHT31.h>
#include <Radio/Loom_LoRa/Loom_LoRa.h>

#include "AS5311.h"

//////////////////////////
/* DEVICE CONFIGURATION */
//////////////////////////
static const uint8_t NODE_NUMBER = 123;
static const String DEVICE_NAME = "Dend4";
// These two time values are added together to determine the measurement interval
static const int8_t MEASUREMENT_INTERVAL_MINUTES = 15;
static const int8_t MEASUREMENT_INTERVAL_SECONDS = 0;
static const uint8_t TRANSMIT_INTERVAL = 16; // to save power, only transmit a packet every X measurements
//////////////////////////
//////////////////////////

// Pins
#define AS5311_CS A3 // 9 for LB version
#define AS5311_CLK A5
#define AS5311_DO A4
#define BUTTON_PIN A1

//Loom
Manager manager(DEVICE_NAME, NODE_NUMBER);
Loom_Hypnos hypnos(manager, HYPNOS_VERSION::V3_3, TIME_ZONE::PST);
//Loom Sensors
Loom_Analog analog(manager);
Loom_SHT31 sht(manager);
Loom_Neopixel statusLight(manager); // using channel 2 (physical pin A2)
Loom_LoRa lora(manager, NODE_NUMBER);

AS5311 magnetSensor(AS5311_CS, AS5311_CLK, AS5311_DO);

// Global Variables
volatile bool buttonPressed = false; // Check to see if button was pressed

void sleepCycle();
void ISR_RTC();
void ISR_BUTTON();

void measure();
void measureVPD();
void transmitLora();

void checkMagnetSensor();
void alignMagnetSensor();
bool checkStableAlignment();
void displayMagnetStatus(magnetStatus);
void flashColor(uint8_t r, uint8_t g, uint8_t b);

/**
 * Program setup
 */
void setup()
{
    pinMode(BUTTON_PIN, INPUT_PULLUP); // Enable pullup on button pin. this is necessary for the interrupt

    delay(1000);
    manager.beginSerial(!digitalRead(BUTTON_PIN)); // wait for serial connection ONLY button is pressed (low reading)

    hypnos.setLogName(DEVICE_NAME + "--");
    hypnos.enable();
    manager.initialize();

    checkMagnetSensor();
    alignMagnetSensor();

    hypnos.registerInterrupt(ISR_RTC);
}

/**
 * Main loop
 */
void loop()
{
    measure();
    if (buttonPressed) // if interrupt button was pressed, display staus of magnet sensor
    {
        displayMagnetStatus(magnetSensor.getMagnetStatus());
        delay(3000);
        statusLight.set_color(2, 0, 0, 0, 0); // LED Off
        buttonPressed = false;
    }
    transmitLora();
    sleepCycle(); // bug: device will display status for two sleep cycles instead of one when the button is pressed
}

/**
 * Perform all measurements for the dendrometer and put them into a packet.
 * Log to SD card.
 */
void measure()
{
    manager.measure();
    manager.package();
    measureVPD();
    magnetSensor.measure(manager);

    // Log whether system woke up from button or not
    manager.addData("Button", "Pressed?", buttonPressed);

    manager.display_data();

    hypnos.logToSD();
}

/**
 * Log readings from the SHT31 sensor. Also calculate and log VPD.
 */
void measureVPD()
{
    float SVP, VPD, temperature, humidity;
    float e = 2.71828;

    temperature = sht.getTemperature();
    humidity = sht.getHumidity();

    // Tetens equation
    SVP = (0.61078 * pow(e, (17.2694 * temperature) / (temperature + 237.3)));
    VPD = SVP * (1 - (humidity / 100));

    manager.addData("SHT31", "VPD", VPD);
}

/**
 * transmit the current data packet over LoRa
 * loop counter starts high so an initial transmission can be triggered by pressing the button
 * (the first transmission will happen the second time this function is called)
 */
void transmitLora()
{
    static uint8_t loopCounter = TRANSMIT_INTERVAL - 2;
    loopCounter++;
    if (loopCounter >= TRANSMIT_INTERVAL)
    {
        lora.send(0);
        loopCounter = 0;
    }
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

// Interrupt routines
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
 * Magnet alignment procedure. Displays magnet sensor to user until
 * the magnet is determined to be properly aligned and maintains that alignment
 * for a certain amount of time
 */
void alignMagnetSensor()
{
    magnetStatus status;
    while (1)
    {
        // Watchdog.reset();
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
    for (auto _ = 6; _--;)
    {
        // Watchdog.reset();
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
    const unsigned int CHECK_TIME = 3000;
    magnetStatus status;
    bool aligned = true;

    for (int i = 0; i < (CHECK_TIME / 100); i++)
    {
        // Watchdog.reset();
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
 * Checks to see if a magnet sensor is connected and functioning.
 */
void checkMagnetSensor()
{
    uint32_t data = magnetSensor.getRawData();
    if (__builtin_parity(data) == 0 && data != 0) //__builtin_parity() returns 0 if value has even parity
        return;
    for (auto _ = 6; _--;)
        flashColor(255, 100, 0);
}