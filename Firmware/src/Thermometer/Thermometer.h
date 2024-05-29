/**
 * @file Thermometer.h
 * @author Seb Madgwick
 * @brief Driver for the Texas Instruments TMP117.
 */

#ifndef THERMOMETER_H
#define THERMOMETER_H

//------------------------------------------------------------------------------
// Includes

#include <stdint.h>

//------------------------------------------------------------------------------
// Function declarations

void ThermometerInitialise(void);
float ThermometerReadTemperature(void);
uint32_t ThermometerReadUniqueID(void);

#endif

//------------------------------------------------------------------------------
// End of file
