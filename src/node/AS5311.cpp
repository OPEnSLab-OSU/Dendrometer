#include "wiring_constants.h"
#include "AS5311.h"

const int AS5311::DATA_TIMING_US = 12;
// 1000us/bit was the value in the V3 code
// according to the datasheet the chip's limit is 1MHz

const int AS5311::AVERAGE_MEASUREMENTS = 16;

AS5311::AS5311(uint8_t cs_pin, uint8_t clk_pin, uint8_t do_pin)
    : CS_PIN(cs_pin), CLK_PIN(clk_pin), DO_PIN(do_pin) {}

/**
 *  Initialize pins for serial read procedure
 */
void AS5311::initializePins()
{
    // initalize pins
    digitalWrite(CS_PIN, HIGH);
    pinMode(CS_PIN, OUTPUT);
    digitalWrite(CLK_PIN, HIGH);
    pinMode(CLK_PIN, OUTPUT);
    pinMode(DO_PIN, INPUT);
}

/**
 * deinitialize pins after serial read
 */
void AS5311::deinitializePins()
{
    pinMode(CS_PIN, INPUT);
    digitalWrite(CS_PIN, LOW);
    pinMode(CLK_PIN, INPUT);
    digitalWrite(CLK_PIN, LOW);
    pinMode(DO_PIN, INPUT);
}

/**
 *  Returns the serial output from AS533
 * @return 32 bit value, of which the 18 least signifcant bits contain the sensor data
 */
uint32_t AS5311::bitbang()
{
    initializePins();

    // write clock high to select the angular position data
    digitalWrite(CLK_PIN, HIGH);
    delayMicroseconds(DATA_TIMING_US);
    // select the chip
    digitalWrite(CS_PIN, LOW);
    delayMicroseconds(DATA_TIMING_US);

    // begin the data transfer
    digitalWrite(CLK_PIN, LOW);

    uint32_t data = 0;
    const uint8_t BITS = 18;
    for (uint8_t i = 0; i < BITS; i++)
    {
        delayMicroseconds(DATA_TIMING_US);
        digitalWrite(CLK_PIN, HIGH);

        // don't set clock low on last bit
        if (i < (BITS - 1))
        {
            delayMicroseconds(DATA_TIMING_US);
            digitalWrite(CLK_PIN, LOW);
        }

        delayMicroseconds(DATA_TIMING_US);

        auto readval = digitalRead(DO_PIN);
        if (readval == HIGH)
        {
            data |= 1 << (BITS - 1) - i;
        }
    }

    digitalWrite(CS_PIN, HIGH);
    digitalWrite(CLK_PIN, HIGH);
    delayMicroseconds(DATA_TIMING_US);
    deinitializePins();

    return data;
}

/**
 * Determine the magnet alignment status
 * See pages 12 to 15 of the AS5311 datasheet for more information
 * @return magnetStatus enum
 */
magnetStatus AS5311::getMagnetStatus()
{
    uint32_t data = bitbang();

    // invalid data
    if (!(data & (1 << OCF)) || data & (1 << COF) || __builtin_parity(data)) //__builtin_parity returns 1 if odd parity
        return magnetStatus::error;

    // magnetic field out of range
    if (data & (1 << MAGINC) && data & (1 << MAGDEC) && data & (1 << LIN))
        return magnetStatus::red;

    // magnetic field borderline out of range
    if (data & (1 << MAGINC) && data & (1 << MAGDEC))
        return magnetStatus::yellow;

    return magnetStatus::green;
}

/**
 * Return the raw sensor binary data
 * @return raw sensor data
 */
uint32_t AS5311::getMagnetRaw()
{
    return bitbang();
}

/**
 * Right shift the raw sensor data to isolate the absolute position component
 * @return 12-bit absolute postion value
 */
uint16_t AS5311::getPosition()
{
    return bitbang() >> ANGLEDATAOFFSET;
}

/**
 * Takes multiple position measurements and average them
 * @return averaged 12-bit absolute position value
 */
uint16_t AS5311::getFilteredPosition()
{
    uint16_t average = 0;
    for (uint8_t i = 0; i < AVERAGE_MEASUREMENTS; i++)
    {
        average += getPosition();
    }
    average /= AVERAGE_MEASUREMENTS;
    return average;
}