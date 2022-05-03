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

// https://timodenk.com/blog/port-manipulation-and-arduino-digitalwrite-performance/
void fastDigitalWrite(uint8_t pin, uint8_t x) {
    if (pin / 8) { // pin >= 8
        PORTB ^= (-x ^ PORTB) & (1 << (pin % 8));
    } else {
        PORTD ^= (-x ^ PORTD) & (1 << (pin % 8));
    }
}

void fastShiftOut(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, uint8_t val) {
    uint8_t i;

    for (i = 0; i < 8; i++) {
        if (bitOrder == LSBFIRST) {
            fastDigitalWrite(dataPin, val & 1);
            val >>= 1;
        } else {
            fastDigitalWrite(dataPin, (val & 128) != 0);
            val <<= 1;
        }

        fastDigitalWrite(clockPin, HIGH);
        fastDigitalWrite(clockPin, LOW);
    }
}

void setAddress(uint16_t address, bool lower) {
    fastShiftOut(SR_SERIAL_INPUT, SR_CLK, LSBFIRST, (uint8_t)address);

    uint8_t addr = (address >> 8) | (lower ? 0 : B01000000);
    fastShiftOut(SR_SERIAL_INPUT, SR_CLK, LSBFIRST, addr);
}

void setData(uint8_t data) {
    fastShiftOut(SR_SERIAL_INPUT, SR_CLK, LSBFIRST, data);
}

void writeSRAMByte(uint16_t addr, uint8_t data, bool lower) {
    setData(data);
    setAddress(addr, lower);

    fastDigitalWrite(SR_STORAGE_CLK, LOW);
    fastDigitalWrite(SR_STORAGE_CLK, HIGH);
    fastDigitalWrite(SR_STORAGE_CLK, LOW);

    fastDigitalWrite(SRAM_WRITE_ENABLED, HIGH);
    fastDigitalWrite(SRAM_WRITE_ENABLED, LOW);
    fastDigitalWrite(SRAM_WRITE_ENABLED, HIGH);
}

} // namespace

bool load(File &romRile, const bool lower, void (*progress)(uint8_t)) {
    uint8_t buffer[STREAM_BUFFER_SIZE];

    uint32_t size = romRile.size();

    DEBUG_PRINT("ROM size: ");
    DEBUG_PRINTLN(size);

    if (size > ROM_SIZE)
        return false; // big file!

    digitalWrite(SRAM_CHIP_ENABLED, HIGH);
    pinMode(SRAM_CHIP_ENABLED, OUTPUT);

    pinMode(SR_OUTPUT_ENABLED, OUTPUT);

    pinMode(SR_SERIAL_INPUT, OUTPUT);

    digitalWrite(SR_CLK, LOW);
    pinMode(SR_CLK, OUTPUT);

    digitalWrite(SR_STORAGE_CLK, LOW);
    pinMode(SR_STORAGE_CLK, OUTPUT);

    digitalWrite(SRAM_WRITE_ENABLED, HIGH);
    pinMode(SRAM_WRITE_ENABLED, OUTPUT);

    digitalWrite(SRAM_CHIP_ENABLED, LOW);
    digitalWrite(SR_OUTPUT_ENABLED, LOW);

    uint16_t address = 0;

    while (address < size) {
        size_t bytes = romRile.readBytes(buffer, STREAM_BUFFER_SIZE);

        for (size_t i = 0; i < bytes; i++) {
            writeSRAMByte(address, buffer[i], lower);
            address++;
        }

        uint8_t upt = (address * 10L) / size;
        progress(upt);
    }

    digitalWrite(SRAM_CHIP_ENABLED, HIGH);
    digitalWrite(SR_OUTPUT_ENABLED, HIGH);

    return true;
}

} // namespace sram
