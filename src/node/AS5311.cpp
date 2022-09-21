#include "AS5311.h"

define DATA_TIMING_US 1000 // 1000 is pretty slow, maybe we can go faster?
// according to the datasheet the chip's limit is 1MHz

AS5311::AS5311(uint8_t cs_pin, uint8_t clk_pin, uint8_t do_pin)
    : CS_PIN(cs_pin),
      CLK_PIN(clk_pin),
      DO_PIN(do_pin),
{
}

// Returns the serial output from AS533
uint32_t AS5311::bitbang()
{
    // initalize pins
    digitalWrite(CS_PIN, HIGH);
    pinMode(CS_PIN, OUTPUT);
    digitalWrite(CLK_PIN, HIGH);
    pinMode(CLK_PIN, OUTPUT);
    pinMode(DO_PIN, INPUT);

    // write clock high to select the angular position data
    digitalWrite(CLK, HIGH);
    delayMicroseconds(DATA_TIMING_US);
    // select the chip
    digitalWrite(CS, LOW);
    delayMicroseconds(DATA_TIMING_US);

    digitalWrite(CLK, LOW);

    uint32_t value = 0;
    const uint8_t BITS = 18;
    for (uint8_t i = 0; i < BITS; i++)
    {
        delayMicroseconds(DATA_TIMING_US);
        digitalWrite(CLK, HIGH);

        // don't setc lock low on last bit
        if (i < (BITS - 1))
        {
            delayMicroseconds(DATA_TIMING_US);
            digitalWrite(CLK, LOW);
        }

        delayMicroseconds(DATA_TIMING_US);

        auto readval = digitalRead(DO);
        if (readval == HIGH)
            value |= 1U << (BITS - 1) - i;
    }

    digitalWrite(CS, HIGH);
    digitalWrite(CLK, HIGH);
    delayMicroseconds(DATA_TIMING_US);

    // deinitalize pins
    pinMode(CS_PIN, INPUT);
    digitalWrite(CS_PIN, LOW);
    pinMode(CLK_PIN, INPUT);
    digitalWrite(CLK_PIN, LOW);
    pinMode(DO_PIN, INPUT);

    return value;
}

magnetStatus AS5311::getMagnetRange()
{
    uint32_t data = bitbang();

    // invalid data
    if (!(data & (1 << OCF)) || data & ((1 << COF) | (1 << LIN)))
        return magnetStatus::error;

    // magnetic field out of range
    if (data & (1 << MAGINC) && data & (1 << MAGDEC) && data & (1 << LIN))
        return magnetStatus::red;

    // magnetic field borderline out of range
    if (data & (1 << MAGINC) && data & (1 << MAGDEC))
        return magnetStatus::yellow;

    return magnetStatus::green;
}

// Todo: Make more robust than just checking first two bits
float AS5311::computeElapsed(uint32_t curr, uint32_t &prevTwoSig, float elapsed)
{
    uint32_t currTwoSig = curr & 0xC00;
    if ((currTwoSig == 0xC00 && prevTwoSig == 0x0))
    {
        Serial.println("ROLLOVER UNDERFLOW");
        elapsed -= 2.0;
    }
    else if (prevTwoSig == 0xC00 && currTwoSig == 0x0)
    {
        Serial.println("ROLLOVER OVERFLOW");
        elapsed += 2.0;
    }
    prevTwoSig = currTwoSig;
    return elapsed;
}

uint16_t AS5311::getPosition()
{
    return bitbang() >> ANGLEDATAOFFSET;
}

// Takes 16 data measurements and averages them for normal reading
uint16_t AS5311::getFilteredPosition()
{
    uint16_t average = 0;
    for (uint8_t i = 0; i < 16; i++)
    {
        average += getPosition();
    }
    average /= 16;
    return average;
}