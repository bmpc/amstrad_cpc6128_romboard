#include "eepromflash.h"
#include "log.h"

/*
 64K = N
 32K = S
 16K = A

 N S A A A A A A A A A A A A A A D D D D D D D D
 _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _

*/
namespace eeprom {
namespace {

void setAddress(uint16_t address, bool lower) {
    shiftOut(SR_SERIAL_INPUT, SR_CLK, LSBFIRST, (uint8_t)address);
    
    uint8_t addr = (address >> 8) | (lower ? 0 : B01000000);
    shiftOut(SR_SERIAL_INPUT, SR_CLK, LSBFIRST, addr);
}

void setData(uint8_t data) {
    shiftOut(SR_SERIAL_INPUT, SR_CLK, LSBFIRST, data);
}

void waitForWrite(uint8_t latched, uint16_t addr) {
    digitalWrite(SR_DATA_OUTPUT_ENABLED, HIGH);
    digitalWrite(EEPROM_OUTPUT_ENABLED, LOW);
    delayMicroseconds(1);

    while (digitalRead(IO7) != (latched >> 7)) {
        delayMicroseconds(1);
    }

    digitalWrite(EEPROM_OUTPUT_ENABLED, HIGH);
    digitalWrite(SR_DATA_OUTPUT_ENABLED, LOW);
    delayMicroseconds(1);
}

void fastWriteEEPROMByte(uint16_t addr, uint8_t data, bool lower) {
    __asm__ __volatile__(
        ".equ PORTD, 0x0B                    \n"
        ".equ PORTB, 0x05                    \n"
        ".equ PORTC, 0x08                    \n"
        ".equ SR_SERIAL_INPUT, 0x03          \n"
        ".equ SR_CLK, 0x08                   \n"
        "CLI                                 \n" // Clear Global Interrupt
        "SBI "
        "OUT PORTD, r24"
        
        "SEI                                 \n" // Set Global Interrupt
        ::"r" (addr, data, lower):
    );
}

void writeEEPROMByte(uint16_t addr, uint8_t data, bool lower) {
    setData(data);
    setAddress(addr, lower);

    digitalWrite(SR_STORAGE_CLK, HIGH);
    delayMicroseconds(1);
    digitalWrite(SR_STORAGE_CLK, LOW);
    delayMicroseconds(1);
    
    // This is in fact a page write cycle for each byte. 
    // The code within this block takes much longer than 150 micros.
    digitalWrite(EEPROM_WRITE_ENABLED, LOW);
    delayMicroseconds(1);
    digitalWrite(EEPROM_WRITE_ENABLED, HIGH);
    
    waitForWrite(data, addr);
}

void writeEEPROMBytePage(uint8_t *page, size_t size, uint16_t address, bool lower) {
    for (size_t i = 0; i < size; i++) {       
        writeEEPROMByte(address, page[i], lower);
        address++;
    }
    
    // there is no point in waiting here as each byte is written within a page write 
    // delay(20);
    // waitForWrite(page[i], address);
}

} // namespace

bool flash(File &romRile, const bool lower, void (*progress)(uint8_t)) {
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
    
    digitalWrite(EEPROM_WRITE_ENABLED, HIGH);
    pinMode(EEPROM_WRITE_ENABLED, OUTPUT);

    digitalWrite(EEPROM_OUTPUT_ENABLED, HIGH);
    pinMode(EEPROM_OUTPUT_ENABLED, OUTPUT);
    
    pinMode(IO7, INPUT);

    // FIXME: we must also use this signal to tri-state the address and data lines
    // from the amstrad CPC (using tri-state buffers??)
    digitalWrite(SR_DATA_OUTPUT_ENABLED, LOW);
    digitalWrite(SR_ADDR_OUTPUT_ENABLED, LOW);

    uint16_t address = 0;

    while (address < size) {
        size_t bytes = romRile.readBytes(buffer, STREAM_BUFFER_SIZE);

        writeEEPROMBytePage(buffer, bytes, address, lower);
        address += bytes;

        uint8_t upt = (address * 10L) / size;
        progress(upt);
    }

    digitalWrite(SR_DATA_OUTPUT_ENABLED, HIGH);
    digitalWrite(SR_ADDR_OUTPUT_ENABLED, HIGH);

    return true;
}

} // namespace eeprom
