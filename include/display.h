#pragma once
#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include "SSD1306Ascii.h"
#include "SSD1306AsciiAvrI2c.h"
#include <Arduino.h>

#define I2C_ADDRESS 0x3C

namespace cpc_rom_board {

enum DisplayState {
    INIT = 1,
    LIST = 2,
    BROWSE = 3,
    CONFIRM = 4,
    LOADING = 5,
    CONFIG = 6,
    DEBUG = 7
};

enum LoadState {
    IDLE = 1,
    STARTED = 2,
    PROGRESS = 3,
    FINISHED = 4
};

struct DisplayData {
    String m_lower_rom;
    String m_upper_rom;
    String m_prev_filename;
    String m_current_filename;
    String m_next_filename;
    uint8_t m_load_progress;
    bool m_bank_select;
    LoadState m_load_state;
    bool m_load_error;
    String m_debug_msg;

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

    inline static DisplayData withLoadProgress(const String &current, const uint8_t &progress, LoadState state, bool error) {
        DisplayData dd;
        dd.m_current_filename = current;
        dd.m_load_progress = progress;
        dd.m_load_state = state;
        dd.m_load_error = error;
        return dd;
    }

    inline static DisplayData withLoadProgress(LoadState state, const uint8_t &progress) {
        DisplayData dd;
        dd.m_load_progress = progress;
        dd.m_load_state = state;
        return dd;
    }

    inline static DisplayData withConfirmFile(const String &current, bool bank_select) {
        DisplayData dd;
        dd.m_current_filename = current;
        dd.m_bank_select = bank_select;
        return dd;
    }

    inline static DisplayData withDebugMsg(const String &msg) {
        DisplayData dd;
        dd.m_debug_msg = msg;
        return dd;
    }
};

class Display {

public:
    Display() = default;
    void setup();
    void update(const DisplayState &, const DisplayData &);

private:
    SSD1306AsciiAvrI2c m_display;
    DisplayState m_current_state;

    void printInit();
    void printDebug(const String& msg);
    void printCurrentROMs(const String& lower_rom, const String& upper_rom);
    void printFilename(const String& prev, const String& curr, const String& next);
    void printConfirmationMsg(const String& filename);
    void printUpperLowerSelection(bool higher);
    void printStartLoading(const String& filename);
    void printLoadingProgress(const String& filename, uint8_t progress);
    void printLoadingComplete(bool error);
    void printSave();

};
}

#endif