/**
 * @file Ximu3Device.c
 * @author Seb Madgwick
 * @brief x-IMU3 device.
 */

//------------------------------------------------------------------------------
// Includes

#include "Led/Led.h"
#include <stdint.h>
#include <stdio.h>
#include "Thermometer/Thermometer.h"
#include "Timer/Timer.h"
#include "Timestamp/Timestamp.h"
#include "Usb/UsbCdc.h"
#include "x-IMU3-Device/Ximu3.h"

//------------------------------------------------------------------------------
// Function declarations

static size_t UsbRead(void* const destination, size_t numberOfBytes, void* const context);
static void UsbWrite(const void* const data, const size_t numberOfBytes, void* const context);
static void Ping(const char* * const value, Ximu3CommandResponse * const response, void* const context);
static void Blink(const char* * const value, Ximu3CommandResponse * const response, void* const context);
static void Strobe(const char* * const value, Ximu3CommandResponse * const response, void* const context);
static void Note(const char* * const value, Ximu3CommandResponse * const response, void* const context);
static void Timestamp(const char* * const value, Ximu3CommandResponse * const response, void* const context);
static void Error(const char* const error, void* const context);

//------------------------------------------------------------------------------
// Variables

static Ximu3CommandInterface interfaces[] = {
    { .name = "USB", .read = UsbRead, .write = UsbWrite},
};
static const Ximu3CommandMap commands[] = {
    {"ping", Ping},
    {"blink", Blink},
    {"strobe", Strobe},
    {"note", Note},
    {"timestamp", Timestamp},
};
static Ximu3CommandBridge bridge = {
    .interfaces = interfaces,
    .numberOfInterfaces = sizeof (interfaces) / sizeof (Ximu3CommandInterface),
    .commands = commands,
    .numberOfCommands = sizeof (commands) / sizeof (Ximu3CommandMap),
    .error = Error,
};

//------------------------------------------------------------------------------
// Functions

/**
 * @brief Module tasks. This function should be called repeatedly within the
 * main program loop.
 */
void Ximu3DeviceTasks(void) {
    Ximu3CommandTasks(&bridge);
}

/**
 * @brief Reads data from the read buffer.
 * @param destination Destination.
 * @param numberOfBytes Number of bytes.
 * @param context Context.
 * @return Number of bytes read.
 */
size_t UsbRead(void* const destination, size_t numberOfBytes, void* const context) {
    return UsbCdcRead(destination, numberOfBytes);
}

/**
 * @brief Writes data to the write buffer.
 * @param data Data.
 * @param numberOfBytes Number of bytes.
 * @param context Context.
 */
void UsbWrite(const void* const data, const size_t numberOfBytes, void* const context) {
    UsbCdcWrite(data, numberOfBytes);
}

/**
 * @brief Ping command.
 * @param value Value.
 * @param response Response.
 * @param context Context.
 */
static void Ping(const char* * const value, Ximu3CommandResponse * const response, void* const context) {
    if (Ximu3CommandParseNull(value, response) != Ximu3ResultOk) {
        return;
    }
    char serialNumber[16];
    snprintf(serialNumber, sizeof (serialNumber), "%08X", ThermometerReadUniqueID());
    Ximu3CommandRespondPing(response, "x-IMU3 Thermometer", serialNumber);
}

/**
 * @brief Blink command.
 * @param value Value.
 * @param response Response.
 * @param context Context.
 */
void Blink(const char* * const value, Ximu3CommandResponse * const response, void* const context) {
    if (Ximu3CommandParseNull(value, response) != Ximu3ResultOk) {
        return;
    }
    LedBlink();
    Ximu3CommandRespond(response);
}

/**
 * @brief Strobe command.
 * @param value Value.
 * @param response Response.
 * @param context Context.
 */
void Strobe(const char* * const value, Ximu3CommandResponse * const response, void* const context) {
    if (Ximu3CommandParseNull(value, response) != Ximu3ResultOk) {
        return;
    }
    LedStrobe();
    Ximu3CommandRespond(response);
}

/**
 * @brief Note command.
 * @param value Value.
 * @param response Response.
 * @param context Context.
 */
static void Note(const char* * const value, Ximu3CommandResponse * const response, void* const context) {
    char string[XIMU3_VALUE_SIZE];
    if (Ximu3CommandParseString(value, response, string, sizeof (string), NULL) != Ximu3ResultOk) {
        return;
    }
    const Ximu3DataNotification data = {
        .timestamp = TimestampGet(),
        .string = string,
    };
    char message[256];
    const int numberOfBytes = Ximu3DataNotificationAscii(message, sizeof (message), &data);
    UsbCdcWrite(message, numberOfBytes);
    Ximu3CommandRespond(response);
}

/**
 * @brief Timestamp command.
 * @param value Value.
 * @param response Response.
 * @param context Context.
 */
void Timestamp(const char* * const value, Ximu3CommandResponse * const response, void* const context) {
    uint64_t timestamp;
    if (Ximu3CommandParseNumberU64(value, response, &timestamp) != Ximu3ResultOk) {
        return;
    }
    TimestampSet(timestamp);
    Ximu3CommandRespond(response);
}

/**
 * @brief Error handler.
 * @param error error.
 * @param context Context.
 */
static void Error(const char* const error, void* const context) {
    const Ximu3DataError data = {
        .timestamp = TimerGetTicks64() / TIMER_TICKS_PER_MICROSECOND,
        .string = error,
    };
    char message[256];
    const int numberOfBytes = Ximu3DataErrorAscii(message, sizeof (message), &data);
    UsbWrite(message, numberOfBytes, context);
}

//------------------------------------------------------------------------------
// End of file
