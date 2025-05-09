/**
 * @file I2C2.h
 * @author Seb Madgwick
 * @brief I2C driver for PIC32 devices.
 */

#ifndef I2C2_H
#define I2C2_H

//------------------------------------------------------------------------------
// Includes

#include "I2C.h"
#include "I2CClientAddress.h"
#include "I2CMessage.h"
#include <stdbool.h>
#include <stdint.h>

//------------------------------------------------------------------------------
// Function declarations

void I2C2Initialise(const I2CClockFrequency clockFrequency);
void I2C2Deinitialise(void);
void I2C2Start(void);
void I2C2RepeatedStart(void);
void I2C2Stop(void);
bool I2C2Send(const uint8_t byte);
uint8_t I2C2Receive(const bool ack);
void I2C2BeginMessage(I2CMessage * const message_);
bool I2C2MessageInProgress(void);

#endif

//------------------------------------------------------------------------------
// End of file
