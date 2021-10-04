#include "localstorage.h"
#include "log.h"
#include <EEPROM.h>

namespace local_storage {
String _loadRom(uint8_t rom_ix) {
    if (EEPROM.read(rom_ix) == 1) {
        char filename[13];
        uint8_t i = 0, wr = rom_ix + 1;
        char c;
        
        while (((c = EEPROM.read(wr++)) != '\0') && i < 13) {
            filename[i++] = c;
        }

        // make sure we have a string terminator
        if (filename[i] != '\0')
            filename[i] = '\0';

        return String(filename);
    }

    return "";
}

RomPair load() {
    RomPair pair;
    if (EEPROM.read(0) != INIT_MEGA_FLG) {
        return pair;
    }
    pair.lowerRom = _loadRom(LOWER_ROM_IX);
    pair.upperRom = _loadRom(UPPER_ROM_IX);

    return pair;
}

void save(const char *filename, bool lower) {
    EEPROM.write(0, INIT_MEGA_FLG); // set magic number

    uint8_t sto = lower ? LOWER_ROM_IX : UPPER_ROM_IX;

    EEPROM.write(sto, 1); // there is a rom

    uint8_t i = 0, wr = sto + 1;
    char c;

    while (((c = *(filename + i)) != '\0') && i < 13) {
        EEPROM.write(wr, c);
        DEBUG_PRINT("[");
        DEBUG_PRINT(wr);
        DEBUG_PRINT("] = ");
        DEBUG_PRINTLN(c);
        i++;
        wr++;
    }

    // make sure we store the string terminator for each filename
    EEPROM.write(wr, '\0');
    DEBUG_PRINT("[");
    DEBUG_PRINT(wr);
    DEBUG_PRINT("] = ");
    DEBUG_PRINTLN("0");
}

void erase(bool lower) {
    EEPROM.write(0, INIT_MEGA_FLG); // set magic number

    uint8_t sto = lower ? LOWER_ROM_IX : UPPER_ROM_IX;

    EEPROM.write(sto, 0);
}
} // namespace local_storage