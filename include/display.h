#pragma once
#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include "SSD1306Ascii.h"
#include "SSD1306AsciiAvrI2c.h"
#include <Arduino.h>

#define I2C_ADDRESS 0x3C

enum DisplayState {
    LIST = 1,
    BROWSE = 2,
    CONFIRM = 3,
    FLASHING = 5
};

enum FlashState {
    IDLE = 1,
    STARTED = 2,
    PROGRESS = 3,
    FINISHED = 4
};

namespace display {
struct DisplayData {
    String m_lower_rom;
    String m_upper_rom;
    String m_prev_filename;
    String m_current_filename;
    String m_next_filename;
    uint8_t m_flash_progress;
    bool m_bank_select;
    FlashState m_flash_state;
    bool m_flash_error;

    inline static DisplayData withLowerAndUpperRoms(const String &lower, const String &upper) {
        DisplayData dd;
        dd.m_lower_rom = lower;
        dd.m_upper_rom = upper;
        return dd;
    }

    inline static DisplayData withBrowseFiles(const String &prev, const String &current, const String &next) {
        DisplayData dd;
        dd.m_prev_filename = prev;
        dd.m_current_filename = current;
        dd.m_next_filename = next;
        return dd;
    }

    inline static DisplayData withFlashProgress(const String &current, const uint8_t &progress, FlashState state, bool error) {
        DisplayData dd;
        dd.m_current_filename = current;
        dd.m_flash_progress = progress;
        dd.m_flash_state = state;
        dd.m_flash_error = error;
        return dd;
    }

    inline static DisplayData withFlashProgress(FlashState state, const uint8_t &progress) {
        DisplayData dd;
        dd.m_flash_progress = progress;
        dd.m_flash_state = state;
        return dd;
    }

    inline static DisplayData withConfirmFile(const String &current, bool bank_select) {
        DisplayData dd;
        dd.m_current_filename = current;
        dd.m_bank_select = bank_select;
        return dd;
    }
};

void setup();

void update(const DisplayState &, const DisplayData &);
} // namespace display

#endif