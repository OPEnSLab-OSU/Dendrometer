#include <Loom_Manager.h> //4.6
#include <Hardware/Loom_Hypnos/Loom_Hypnos.h>
#include <Hardware/Actuators/Loom_Neopixel/Loom_Neopixel.h>
#include <Sensors/Loom_Analog/Loom_Analog.h>
#include <Sensors/I2C/Loom_SHT31/Loom_SHT31.h>
#include <Radio/Loom_LoRa/Loom_LoRa.h>
#include "AS5311.h"

Manager manager(DEVICE_NAME, NODE_NUMBER);
Loom_Hypnos hypnos(manager, HYPNOS_VERSION::V3_3, TIME_ZONE::PST);
Loom_Analog analog(manager);
Loom_SHT31 sht(manager);
Loom_Neopixel statusLight(manager, false, false, true, NEO_GRB);

AS5311 magnetSensor(AS5311_CS, AS5311_CLK, AS5311_DO);
Loom_LoRa lora(manager, NODE_NUMBER);

void isrTrigger(){
    hypnos.wakeup();
}

void setup()
{
    manager.beginSerial();
    hypnos.enable();
    manager.initialize();
    
    checkMagnetSensor();
    displayMagnetStatus(status);

    hypnos.registerInterrupt(isrTrigger);
}

void loop() {
    displayMagnetStatus(status);
    hypnos.setInterruptDuration(TimeSpan(0, 0, 0, 10));

    // Measure and package data
    manager.measure();
    manager.package();
    
    // Print the current JSON packet
    manager.display_data();            

    // Log the data to the SD card              
    hypnos.logToSD();

    // Send packet over LoRa
    lora.send(0);

    hypnos.reattachRTCInterrupt();
    hypnos.sleep();
}


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

void checkMagnetSensor()
{
    uint32_t data = magnetSensor.getRawData();
    if (__builtin_parity(data) == 0 && data != 0) //__builtin_parity() returns 0 if value has even parity
        return;
    for (auto _ = 6; _--;)
        flashColor(255, 100, 0); // if the check didn't pass, alert the user by flashing the LED
}