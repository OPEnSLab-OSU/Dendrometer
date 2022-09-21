#pragma once

#include <Arduino.h>


enum magnetStatus
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
    magnetStatus getMagnetRange();
    uint16_t getFilteredPosition();
    
    float computeElapsed(uint32_t curr, uint32_t &prevTwoSig, float elapsed);

private:
    const uint8_t CS_PIN;
    const uint8_t CLK_PIN;
    const uint8_t DO_PIN;

    uint32_t bitbang();
    uint16_t getPosition();
}


// bit definitions
#define PAR 0
#define MAGDEC 1
#define MAGINC 2
#define LIN 3 
#define COF 4
#define OCF 5

#define ANGLEDATAOFFSET 6
