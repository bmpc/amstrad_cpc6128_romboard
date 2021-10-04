#pragma once
#ifndef _LOCALSTO_H_
#define _LOCALSTO_H_

#include <arduino.h>

#define INIT_MEGA_FLG 111 // just a hint to detect if the arduino EEPROM data was written before
#define LOWER_ROM_IX 1
#define UPPER_ROM_IX 15

/** 
 * Handles reading and writing the loaded ROM filenames to the ATMEGA EEPROM
 **/
namespace local_storage {
struct RomPair {
    String lowerRom;
    String upperRom;
};

RomPair load();
void save(const char *filename, bool lower);
void erase(bool lower);
} // namespace local_storage

#endif