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
#include "Ximu3Device/x-IMU3-Device/Ximu3Data.h"
#include "Ximu3Device/Ximu3Device.h"

//------------------------------------------------------------------------------
// Functions

int main(void) {
    SYS_Initialize(NULL);

    // Initialise debug UART
    Uart2Initialise(&uartSettingsDefault);

    // Print start up message
    ResetCausePrint(ResetCauseGet());
    printf("x-IMU3 Thermometer v1.2.0\n");

    // Initialise modules
    TimerInitialise();
    LedInitialise();
    ThermometerInitialise();

    // Main program loop
    while (true) {
        SYS_Tasks();

        // Module tasks
        UsbCdcTasks();
        Ximu3DeviceTasks();

        // Send temperature
        if (PERIODIC_POLL(1.0f)) {
            const Ximu3DataTemperature data = {
                .timestamp = TimerGetTicks64() / TIMER_TICKS_PER_MICROSECOND,
                .temperature = ThermometerReadTemperature(),
            };
            char message[256];
            const int numberOfBytes = Ximu3DataTemperatureAscii(message, sizeof (message), &data);
            UsbCdcWrite(message, numberOfBytes);
        }
    }
    return (EXIT_FAILURE);
}

//------------------------------------------------------------------------------
// End of file
