/**
 * @file Timestamp.h
 * @author Seb Madgwick
 * @brief Timestamp in microseconds.
 */

#ifndef TIMESTAMP_H
#define TIMESTAMP_H

//------------------------------------------------------------------------------
// Includes

#include <stdint.h>

//------------------------------------------------------------------------------
// Function declarations

uint64_t TimestampGet(void);
void TimestampSet(const uint64_t timestamp);
uint64_t TimestampFrom(const uint64_t ticks);

#endif

//------------------------------------------------------------------------------
// End of file
