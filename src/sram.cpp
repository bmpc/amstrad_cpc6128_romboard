#include "sram.h"
#include "log.h"

/*
 64K = N
 32K = S
 16K = A

 N S A A A A A A A A A A A A A A D D D D D D D D
 _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _

*/
namespace sram {
namespace {

void setAddress(uint16_t address, bool lower) {
    shiftOut(SR_SERIAL_INPUT, SR_CLK, LSBFIRST, (uint8_t)address);
    
    uint8_t addr = (address >> 8) | (lower ? 0 : B01000000);
    shiftOut(SR_SERIAL_INPUT, SR_CLK, LSBFIRST, addr);
}

void setData(uint8_t data) {
    shiftOut(SR_SERIAL_INPUT, SR_CLK, LSBFIRST, data);
}

void writeSRAMByte(uint16_t addr, uint8_t data, bool lower) {
    setData(data);
    setAddress(addr, lower);

    digitalWrite(SR_STORAGE_CLK, HIGH);
    delayMicroseconds(1);
    digitalWrite(SR_STORAGE_CLK, LOW);
    delayMicroseconds(1);
    
    digitalWrite(SRAM_WRITE_ENABLED, LOW);
    delayMicroseconds(1);
    digitalWrite(SRAM_WRITE_ENABLED, HIGH);
}

} // namespace anonymous

bool load(File &romRile, const bool lower, void (*progress)(uint8_t)) {
    uint8_t buffer[STREAM_BUFFER_SIZE];

    uint32_t size = romRile.size();

    DEBUG_PRINT("ROM size: ");
    DEBUG_PRINTLN(size);

    if (size > ROM_SIZE)
        return false; // big file!

    pinMode(SR_DATA_OUTPUT_ENABLED, OUTPUT);
    pinMode(SR_ADDR_OUTPUT_ENABLED, OUTPUT);

    pinMode(SR_SERIAL_INPUT, OUTPUT);

    digitalWrite(SR_CLK, LOW);
    pinMode(SR_CLK, OUTPUT);

    digitalWrite(SR_STORAGE_CLK, LOW);
    pinMode(SR_STORAGE_CLK, OUTPUT);
    
    digitalWrite(SRAM_WRITE_ENABLED, HIGH);
    pinMode(SRAM_WRITE_ENABLED, OUTPUT);

    digitalWrite(SRAM_OUTPUT_ENABLED, HIGH);
    pinMode(SRAM_OUTPUT_ENABLED, OUTPUT);
    
    pinMode(IO7, INPUT);

    // FIXME: we must also use this signal to tri-state the address and data lines
    // from the amstrad CPC (using tri-state buffers??)
    digitalWrite(SR_DATA_OUTPUT_ENABLED, LOW);
    digitalWrite(SR_ADDR_OUTPUT_ENABLED, LOW);

    uint16_t address = 0;

    while (address < size) {
        size_t bytes = romRile.readBytes(buffer, STREAM_BUFFER_SIZE);

        for (size_t i = 0; i < bytes; i++) {
            writeSRAMByte(address, buffer[i], lower);
            address++;
        }
        address += bytes;

        uint8_t upt = (address * 10L) / size;
        progress(upt);
    }

    digitalWrite(SR_DATA_OUTPUT_ENABLED, HIGH);
    digitalWrite(SR_ADDR_OUTPUT_ENABLED, HIGH);

    return true;
}

} // namespace sram
