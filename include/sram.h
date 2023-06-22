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
#define SRAM_CHIP_ENABLED 2

#define SR_SERIAL_INPUT 3
#define SR_STORAGE_CLK 7
#define SR_CLK 8
#define SR_OUTPUT_ENABLED 10 // Data and Adress lines

namespace cpc_rom_board::sram {

class SRamListener {
public:
    virtual void on_progress(uint8_t prog) = 0;
};

bool load(File &, const bool, SRamListener& listener);

}

#endif // _SRAM_H_