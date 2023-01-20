#pragma once

#include <Arduino.h>

enum class magnetStatus
{
    red,
    green,
    yellow,
    error
};

class AS5311
{
public:
    AS5311(uint8_t cs_pin, uint8_t clk_pin, uint8_t do_pin);
    magnetStatus getMagnetStatus();
    uint16_t getFilteredPosition();
    uint32_t getMagnetRaw();

private:
    const uint8_t CS_PIN;
    const uint8_t CLK_PIN;
    const uint8_t DO_PIN;

    static const int DATA_TIMING_US;
    static const int AVERAGE_MEASUREMENTS;

    void initializePins();
    void deinitializePins();
    uint16_t getPosition();
    uint32_t bitbang();
};

// bit definitions
#define PAR 0
#define MAGDEC 1
#define MAGINC 2
#define LIN 3
#define COF 4
#define OCF 5

#define ANGLEDATAOFFSET 6
