/**
 * @file Thermometer.c
 * @author Seb Madgwick
 * @brief Texas Instruments TMP117 driver.
 */

//------------------------------------------------------------------------------
// Includes

#include "I2C/I2C2.h"

//------------------------------------------------------------------------------
// Definitions

#define I2C_ADDRESS (0x48)

//------------------------------------------------------------------------------
// Function declarations

static uint16_t ReadRegister(const uint8_t address);

//------------------------------------------------------------------------------
// Functions

/**
 * @brief Initialises the module. This function must only be called once, on
 * system startup.
 */
void ThermometerInitialise(void) {
    I2C2Initialise(I2CClockFrequency400kHz);
}

/**
 * @brief Returns the temperature in degrees Celsius.
 * @return Temperature in degrees Celsius.
 */
float ThermometerReadTemperature(void) {
    return (float) (int16_t) ReadRegister(0x00) * 0.0078125f;
}

/**
 * @brief Returns the 48-bit unique ID.
 * @return 48-bit unique ID.
 */
uint64_t ThermometerReadUniqueId(void) {
    const uint16_t word2 = ReadRegister(0x05);
    const uint16_t word1 = ReadRegister(0x06);
    const uint16_t word0 = ReadRegister(0x08);
    return ((uint64_t) word2 << 32) | ((uint64_t) word1 << 16) | (uint64_t) word0;
}

/**
 * @brief Reads a register.
 * @param address Register address.
 * @return Register value.
 */
static uint16_t ReadRegister(const uint8_t address) {
    I2C2Start();
    I2C2TransmitAddressWrite(I2C_ADDRESS);
    I2C2Transmit(address);
    I2C2RepeatedStart();
    I2C2TransmitAddressRead(I2C_ADDRESS);
    const uint8_t msb = I2C2ReceiveAck();
    const uint8_t lsb = I2C2ReceiveNack();
    I2C2Stop();
    return ((uint16_t) msb << 8) | (uint16_t) lsb;
}

//------------------------------------------------------------------------------
// End of file
