#pragma once
#ifndef _FLASH_H_
#define _FLASH_H_

#include <Arduino.h>
#include <SD.h>
#include <SPI.h>

#define EEPROM_SIZE 32768L
#define ROM_SIZE 16384L
#define STREAM_BUFFER_SIZE 64

#define EEPROM_CHIP_ENABLED 0
#define EEPROM_WRITE_ENABLED 5
//#define EEPROM_OUTPUT_ENABLED 17
//#define IO7 9

#define SR_SERIAL_INPUT 3
#define SR_ADDR_OUTPUT_ENABLED 6
#define SR_STORAGE_CLK 7
#define SR_CLK 8
#define SR_DATA_OUTPUT_ENABLED 10

namespace eeprom {
bool flash(File &, const bool, void (*)(uint8_t));
}

#endif // _FLASH_H_