#include <Arduino.h>

#include <Loom_Manager.h>

#include <Hardware/Loom_Hypnos/Loom_Hypnos.h>
#include <Hardware/Actuators/Loom_Neopixel/Loom_Neopixel.h>
#include <Sensors/Loom_Analog/Loom_Analog.h>
#include <Sensors/I2C/Loom_SHT31/Loom_SHT31.h>
#include <Radio/Loom_LoRa/Loom_LoRa.h>

#include "AS5311.h"

static const uint8_t DENDROMETER_NODE_ID 123;
static const int8_t MEASUREMENT_INTERVAL_MINUTES 15;
static const int8_t MEASUREMENT_INTERVAL_SECONDS 0;

// Pins
#define AS5311_CS A3
#define AS5311_CLK A5
#define AS5311_DO A4
#define LED_PIN A2

#define BUTTON_PIN A1
#define RTC_INT_PIN 12

// Global Variables
volatile bool buttonPressed = false; // Check to see if button was pressed
uint32_t initialMeasurement = 0;
//!!!uint32_t prevTwoSig = 0;
//!!!!float elapsed = 0;

uint8_t loopCounter = 14;
uint8_t loraTransmitInterval = 16; // to save power, only transmit a packet every X measurements

Manager manager("Dendrometer", DENDROMETER_NODE_ID);

Loom_Hypnos hypnos(manager, HYPNOS_VERSION::V3_3, TIME_ZONE::PST);

Loom_Analog analog(manager);
Loom_SHT31 sht(manager);
Loom_Neopixel statusLight(manager);

// Address: DENDROMETER_NODE_ID, power level: 23 Retry Count: 7, Timeout: 500
Loom_LoRa lora(manager, DENDROMETER_NODE_ID, 23, 7, 500);

AS5311 magnetSensor(AS5311_CS, AS5311_CLK, AS5311_DO);


void ISR_RTC()
{
    detachinterrupt(BUTTON_PIN);
    detachinterrupt(RTC_INT_PIN);
    hypnos.wakeup();
}

void ISR_BUTTON()
{
    detachinterrupt(BUTTON_PIN);
    detachinterrupt(RTC_INT_PIN);
    hypnos.wakeup();
    buttonPressed = true;
}

void takeMeasurements()
{
    static float previousDistance_mm = 0;
    static float previousDistance_um = 0;

    int average = getFilteredPosition();
    magnetStatus status = magnetSensor.getMagnetRange();

    // Also updates prevTwoSig to two most significant bits of first param, is being passed by ref
    elapsed = computeElapsed(average, prevTwoSig, elapsed);

    // Computes total distance in mm and um
    float distance = (elapsed + ((2.0 * ((int)average - (int)start)) / 4095.0));
    float distanceMicro = (elapsed * 1000) + ((2000 * ((int)average - (int)start)) / 4095.0);
    float difference = 0;
    float differenceMicro = 0;

    // Reads the movement if any, else it sets the distance to 0
    if (distance != prev)
        difference = distance - prev;

    if (distanceMicro != prevMicro)
        differenceMicro = distanceMicro - prevMicro;

    manager.measure();
    manager.package();

    manager.addData("AS5311", "Serial_Value", average);
    manager.addData("Displacement_mm", "mm", distance_mm);
    manager.addData("Displacement_um", "um", distanceMicro_um);
    manager.addData("Difference_mm", "mm", difference_mm);
    manager.addData("Difference_um", "um", difference_um);

    
    switch (status)
    {
    case error:
        manager.addData("Status", "Color", "Error");
        break;
    case red:
        manager.addData("Status", "Color", "Red");
        break;
    case yellow:
        manager.addData("Status", "Color", "Yellow");
        break;
    case green:
        manager.addData("Status", "Color", "Green");
        break;
    default: // do nothing
    }

    float SVP, VPD, temperature, humidity;
    float e = 2.71828;

    temperature = sht.getTemperature();
    humidity = sht.getHumidity();

    SVP = (0.61078 * pow(e, (17.2694 * temperature) / (temperature + 237.3)));
    VPD = SVP * (1 - (humidity / 100));

    manager.addData("VPD", "VPD", VPD);

    // Log RSSI value from LoRa communication
    float rssi = Feather.get<Loom::LoRa>()->get_signal_strength();
    manager.addData("RSSI", "RSSI", rssi);

    // Log whether system woke up from button or not
    manager.addData("Button", "Pressed?", button);

    previousDistance_mm = distance_mm;
    previousDistance_um = distance_um;

    manager.display_data();
    hypnos.logToSD();
}

void alignMagnetSensor()
{
    magnetStatus status;

checkLoop:
    delay(100);
    status = magnetSensor.getMagnetRange();

    switch (status)
    {
    case error:
    case red:
        statusLight.set_color(2, 0, 0, 255, 0); // red
        goto checkLoop;

    case yellow:
        statusLight.set_color(2, 0, 255, 255, 0); // yellow
        goto checkLoop;

    case green:
        statusLight.set_color(2, 0, 255, 0, 0); // green

        // Make sure calibration is stable before proceeding
                for
                    int(i = 0; i < 50; i++)
                    {
                        status = magnetSensor.getMagnetRange();
                        if (status != magnetStatus.green)
                            goto checkLoop;
                        delay(100);
                    }
                break;
    default: // do nothing
    }

    // Flashes Neopixel green
    for (int i = 0; i < 6; i++)
    {
        statusLight.set_color(2, 0, 255, 0, 0); // green
        delay(250);

        statusLight.set_color(2, 0, 0, 0, 0); // off
        delay(250);
    }
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
    if (buttonPressed)  // if interrupt button was pressed, display staus of magnet sensor
    {
        buttonPressed = false;

        magnetStatus status = magnetSensor.getMagnetRange();

        switch (status)
        {
        case error:
        case red:
            statusLight.set_color(2, 0, 0, 255, 0); // red
            break;
        case yellow:
            statusLight.set_color(2, 0, 255, 255, 0); // yellow
            break;
        case green:
            statusLight.set_color(2, 0, 255, 0, 0); // green
            break;
        default: // do nothing
        }

        delay(3000);
        statusLight.set_color(2, 0, 0, 0, 0); // LED Off
    }

    takeMeasurements();

    loopCounter++;

    // Send data to address 0 after set amount of packets
    if (loopCounter >= loraCount)
    {
        lora.send(0);
        loopCounter = 0;
    }

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