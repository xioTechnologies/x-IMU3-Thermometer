/**
 * @file Led.c
 * @author Seb Madgwick
 * @brief LED driver.
 */

//------------------------------------------------------------------------------
// Includes

#include "definitions.h"
#include "Led.h"
#include "PeripheralBusClockFrequency.h"
#include <stdint.h>

//------------------------------------------------------------------------------
// Definitions

/**
 * @brief State.
 */
typedef enum {
    StateIdle,
    StateBlinkBegin,
    StateBlinkEnd,
} State;

//------------------------------------------------------------------------------
// Variables

static State state;

//------------------------------------------------------------------------------
// Functions

/**
 * @brief Initialises the module.  This function must only be called once, on
 * system startup.
 */
void LedInitialise(void) {

    // Configure PWM
    CCP4CON1bits.MOD = 0b0100; // Dual Edge Compare mode
    CCP4CON1bits.ON = 1;

    // Configure state update timer
    CCP2CON1bits.T32 = 1;
    CCP2PR = PERIPHERAL_BUS_CLOCK_FREQUENCY / 10; // 10 Hz
    CCP2CON1bits.ON = 1;
    EVIC_SourceStatusClear(INT_SOURCE_CCT2);
    EVIC_SourceEnable(INT_SOURCE_CCT2);
}

/**
 * @brief CCT interrupt handler.  This function should be called by the ISR
 * implementation generated by MPLAB Harmony.
 */
void Cct2InterruptHandler(void) {
    const uint16_t normal = 0x1FFF;
    const uint16_t bright = 0xFFFF;
    switch (state) {
        case StateIdle:
            CCP4RB = normal;
            break;
        case StateBlinkBegin:
            CCP4RB = bright;
            state = StateBlinkEnd;
            break;
        case StateBlinkEnd:
            CCP4RB = normal;
            state = StateIdle;
            break;
    }
    EVIC_SourceStatusClear(INT_SOURCE_CCT2);
}

/**
 * @brief Blinks the LED.
 */
void LedBlink(void) {
    switch (state) {
        case StateIdle:
            state = StateBlinkBegin;
            break;
        case StateBlinkBegin:
        case StateBlinkEnd:
            break;
    }
}

//------------------------------------------------------------------------------
// End of file