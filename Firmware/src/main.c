/**
 * @file main.c
 * @author Seb Madgwick
 * @brief Main file.
 *
 * Device:
 * PIC32MM0256GPM028
 *
 * Compiler:
 * XC32 v4.60, MPLAB Harmony 3
 */

//------------------------------------------------------------------------------
// Includes

#include "definitions.h"
#include "Led/Led.h"
#include "Periodic.h"
#include "ResetCause/ResetCause.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include "Thermometer/Thermometer.h"
#include "Timer/Timer.h"
#include "Uart/Uart2.h"
#include "Usb/UsbCdc.h"

//------------------------------------------------------------------------------
// Functions

int main(void) {
    SYS_Initialize(NULL);

    // Initialise debug UART
    Uart2Initialise(&uartSettingsDefault);

    // Print start up message
    ResetCausePrint(ResetCauseGet());
    printf("x-IMU3-Thermometer v1.1.2\n");

    // Initialise modules
    TimerInitialise();
    LedInitialise();
    ThermometerInitialise();

    // Main program loop
    while (true) {
        SYS_Tasks();

        // Module tasks
        UsbCdcTasks();

        // Ping response
        if (UsbCdcAvailableRead() > 0) {
            while (UsbCdcAvailableRead() > 0) {
                UsbCdcReadByte();
            }
            char string[256];
            const int numberOfBytes = snprintf(string, sizeof (string), "{\"ping\":{\"interface\":\"USB\",\"name\":\"x-IMU3 Thermometer\",\"sn\":\"%08X\"}}\n", ThermometerReadUniqueID());
            UsbCdcWrite(string, numberOfBytes);
            LedBlink();
        }

        // Send temperature
        if (PERIODIC_POLL(1.0f)) {
            char string[256];
            static unsigned int seconds;
            const int numberOfBytes = snprintf(string, sizeof (string), "T,%u000000,%0.4f\n", seconds++, ThermometerReadTemperature());
            UsbCdcWrite(string, numberOfBytes);
        }
    }
    return (EXIT_FAILURE);
}

//------------------------------------------------------------------------------
// End of file
