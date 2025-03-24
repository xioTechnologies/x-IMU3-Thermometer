/**
 * @file Thermometer.c
 * @author Seb Madgwick
 * @brief Driver for the Texas Instruments TMP117.
 */

//------------------------------------------------------------------------------
// Includes

#include "I2C/I2C2.h"

//------------------------------------------------------------------------------
// Definitions

#define I2C_SLAVE_ADDRESS (0x48)

//------------------------------------------------------------------------------
// Function declarations

static uint16_t ReadRegister(const uint8_t address);

//------------------------------------------------------------------------------
// Functions

/**
 * @brief Initialises the module.  This function must only be called once, on
 * system startup.
 */
void ThermometerInitialise(void) {
    I2C2Initialise(I2CClockFrequency400kHz);
}

/**
 * @brief Reads temperature in degree Celsius.
 * @return Temperature in degree Celsius.
 */
float ThermometerReadTemperature(void) {
    return ((float) ReadRegister(0x00)) * 0.0078125f;
}

/**
 * @brief Reads the unique ID
 * @return Unique ID
 */
uint32_t ThermometerReadUniqueID(void) {
    const uint16_t upperWord = ReadRegister(0x06);
    const uint16_t lowerWord = ReadRegister(0x08);
    return ((uint32_t) upperWord << 16) | (uint32_t) lowerWord;
}

/**
 * @brief Reads a register.
 * @param address Register address.
 * @return Register value.
 */
static uint16_t ReadRegister(const uint8_t address) {
    I2C2Start();
    I2C2Send(I2CClientAddressWrite(I2C_SLAVE_ADDRESS));
    I2C2Send(address);
    I2C2RepeatedStart();
    I2C2Send(I2CClientAddressRead(I2C_SLAVE_ADDRESS));
    const uint8_t msb = I2C2Receive(true);
    const uint8_t lsb = I2C2Receive(false);
    I2C2Stop();
    return ((uint16_t) msb << 8) | (uint16_t) lsb;
}

//------------------------------------------------------------------------------
// End of file
