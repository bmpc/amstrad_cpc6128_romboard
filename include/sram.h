#pragma once
#ifndef _SRAM_H_
#define _SRAM_H_

#include <Arduino.h>
#include <SD.h>
#include <SPI.h>

#define SRAM_SIZE 32768L
#define ROM_SIZE 16384L
#define STREAM_BUFFER_SIZE 64

#define SRAM_WRITE_ENABLED 5
#define SRAM_CHIP_ENABLED 0

#define SR_SERIAL_INPUT 3
#define SR_STORAGE_CLK 7
#define SR_CLK 8
#define SR_BUS_OUTPUT_ENABLED 10 // Data and Adress lines

namespace sram {
bool load(File &, const bool, void (*)(uint8_t));
}

#endif // _SRAM_H_