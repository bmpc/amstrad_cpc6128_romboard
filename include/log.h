#pragma once
#ifndef _LOG_H_
#define _LOG_H_

//#define DEBUGING 1 // debug on USB Serial

#ifdef DEBUGING
#define DEBUG_PRINT(x) Serial.print(x)
#define DEBUG_PRINT_HEX(x) Serial.print(x, HEX)
#define DEBUG_PRINTLN(x) Serial.println(x)
#define DEBUG_PRINTLN_HEX(x) Serial.println(x, HEX)
#else
#define DEBUG_PRINT(x)
#define DEBUG_PRINT_HEX(x)
#define DEBUG_PRINTLN(x)
#define DEBUG_PRINTLN_HEX(x)
#endif

#include <Arduino.h>

inline void setupSerialDebug() {
    #ifdef DEBUGING
    Serial.begin(9600);
    while (!Serial) {
        ; // wait for serial port to connect. Needed for native USB port only
    }
    #endif
}

#endif // _LOG_H_