/**
 * @file Timestamp.c
 * @author Seb Madgwick
 * @brief Timestamp in microseconds.
 */

//------------------------------------------------------------------------------
// Includes

#include "Timer/Timer.h"
#include "Timestamp.h"

//------------------------------------------------------------------------------
// Variables

static uint64_t offset;

//------------------------------------------------------------------------------
// Functions

/**
 * @brief Gets the timestamp.
 * @return Timestamp.
 */
uint64_t TimestampGet(void) {
    return TimestampFrom(TimerGetTicks64());
}

/**
 * @brief Sets the timestamp.
 * @param timestamp Timestamp.
 */
void TimestampSet(const uint64_t timestamp) {
    offset += timestamp - TimestampGet();
}

/**
 * @brief Returns the timestamp.
 * @param ticks Timer ticks.
 * @return Timestamp.
 */
uint64_t TimestampFrom(const uint64_t ticks) {
    return offset + (ticks / TIMER_TICKS_PER_MICROSECOND);
}

//------------------------------------------------------------------------------
// End of file
