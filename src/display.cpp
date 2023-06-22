#include "display.h"
#include "log.h"
#include <arduino.h>

namespace cpc_rom_board {

void Display::printInit() {
    m_display.clear();
    m_display.setFont(System5x7);
    m_display.println(F("=== ROM board 1.0 ==="));
    m_display.println();
}

void Display::printDebug(const String& msg) {
    m_display.clear();
    m_display.setFont(System5x7);
    m_display.println(F("=== ROM Debug ==="));
    m_display.println();
    m_display.println(msg);
}

void Display::printCurrentROMs(const String& lower_rom, const String& upper_rom) {
    m_display.clear();

    m_display.setFont(System5x7);
    m_display.println(F("===== Lower ROM ====="));
    m_display.setFont(Verdana12);
    m_display.print("[ ");
    if (lower_rom == "") {
        m_display.print("-");
    } else {
        m_display.print(lower_rom);
    }
    m_display.println(" ]");

    m_display.setFont(System5x7);
    m_display.println();

    m_display.setFont(System5x7);
    m_display.println(F("===== Upper ROM ====="));
    m_display.setFont(Verdana12);
    m_display.print("[ ");
    if (upper_rom == "") {
        m_display.print("-");
    } else {
        m_display.print(upper_rom);
    }
    m_display.println(" ]");
}

void Display::printFilename(const String& prev, const String& curr, const String& next) {
    if (curr == "") {
        m_display.clear();
        m_display.setFont(Verdana12);
        m_display.println(F("No ROMs found!"));
        return;
    }

    if (prev == "") {
        m_display.clear();
        m_display.setFont(System5x7);
        m_display.println(F("==== Select ROM ===="));
        m_display.println();
    } else {
        m_display.clear(0, 150, 2, 50);
        m_display.setRow(2);
    }

    m_display.setFont(Verdana12);
    m_display.print("  ");
    m_display.println(prev == "" ? "-" : prev);

    //m_display.setFont(Callibri15);
    m_display.print("[ ");
    m_display.print(curr);
    m_display.println(" ]");

    m_display.print("  ");
    m_display.println(next == "" ? "-" : next);
}

void Display::printConfirmationMsg(const String& filename) {
    m_display.clear();
    m_display.setFont(System5x7);
    m_display.println(F("=== Configure ROM ==="));
    m_display.println();

    m_display.setFont(Verdana12);
    m_display.println(filename);

    m_display.print("[ LOW ]");
    m_display.print("  HIGH  ");
}

void Display::printUpperLowerSelection(bool higher) {
    m_display.setFont(Verdana12);
    m_display.setRow(4);
    m_display.setCol(0);
    if (higher) {
        m_display.print("  LOW  ");
        m_display.print("[ HIGH ]");
    } else {
        m_display.print("[ LOW ]");
        m_display.print("  HIGH  ");
    }
}

void Display::printStartLoading(const String& filename) {
    m_display.clear();
    m_display.setFont(System5x7);
    m_display.println("====== Loading ======");
    m_display.println();
    m_display.setFont(Verdana12);
    m_display.print("[ ");
    m_display.print(filename);
    m_display.println(" ]");
    m_display.print("[");
    m_display.setCol(95);
    m_display.println("]");
}

void Display::printLoadingProgress(const String& filename, uint8_t progress) {
    m_display.setRow(4);
    m_display.setCol(4);

    for (uint8_t i = 0; i < progress; i++) {
        m_display.print("#");
    }
}

void Display::printLoadingComplete(bool error) {
    m_display.clear();
    m_display.setFont(System5x7);
    if (error) {
        m_display.println("Load error!");
    } else {
        m_display.println("Ready.");
    }
    m_display.println();
}

void Display::printSave() {
    m_display.clear();
    m_display.setFont(System5x7);
    m_display.println("ROM configured.");
    m_display.println();
    m_display.println("Please restart CPC");
    m_display.println();
}

void Display::setup() { m_display.begin(&Adafruit128x64, I2C_ADDRESS); m_current_state = INIT; }

void Display::update(const DisplayState &state, const DisplayData &data) {
    switch (state) {
    case INIT: {
        printInit();
        break;
    }
    case LIST: {
        printCurrentROMs(data.m_lower_rom, data.m_upper_rom);
        break;
    }
    case BROWSE: {
        DEBUG_PRINT("Prev: ");
        DEBUG_PRINTLN(data.m_prev_filename);
        DEBUG_PRINT("Current: ");
        DEBUG_PRINTLN(data.m_current_filename);
        DEBUG_PRINT("Next: ");
        DEBUG_PRINTLN(data.m_next_filename);

        printFilename(data.m_prev_filename, data.m_current_filename, data.m_next_filename);
        break;
    }
    case CONFIRM: {
        if (m_current_state == CONFIRM) {
            printUpperLowerSelection(data.m_bank_select);
        } else {
            printConfirmationMsg(data.m_current_filename);
        }
        break;
    }
    case CONFIG:
        printSave();
    case LOADING: {
        if (data.m_load_state == LoadState::STARTED) {
            printStartLoading(data.m_current_filename);
        } else if (data.m_load_state == LoadState::FINISHED) {
            printLoadingComplete(data.m_load_error);
        } else { // loading in progress
            printLoadingProgress(data.m_current_filename, data.m_load_progress);
        }
        break;
    }
    case DEBUG: {
        printDebug(data.m_debug_msg);
    }
    default:
        break;
    }

    m_current_state = state; // keep the current state
}

} // namespace