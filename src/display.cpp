#include "display.h"
#include "log.h"
#include <arduino.h>

namespace display {
namespace {

SSD1306AsciiAvrI2c _display;

DisplayState currentState;

void printInit() {
    _display.clear();
    _display.setFont(System5x7);
    _display.println(F("=== ROM board 1.0 ==="));
    _display.println();
}

void printCurrentROMs(String lower_rom, String upper_rom) {
    _display.clear();

    _display.setFont(System5x7);
    _display.println(F("===== Lower ROM ====="));
    _display.setFont(Verdana12);
    _display.print("[ ");
    if (lower_rom == "") {
        _display.print("-");
    } else {
        _display.print(lower_rom);
    }
    _display.println(" ]");

    _display.setFont(System5x7);
    _display.println();

    _display.setFont(System5x7);
    _display.println(F("===== Upper ROM ====="));
    _display.setFont(Verdana12);
    _display.print("[ ");
    if (upper_rom == "") {
        _display.print("-");
    } else {
        _display.print(upper_rom);
    }
    _display.println(" ]");
}

void printFilename(String prev, String curr, String next) {
    if (curr == "") {
        _display.clear();
        _display.setFont(Verdana12);
        _display.println(F("No ROMs found!"));
        return;
    }

    if (prev == "") {
        _display.clear();
        _display.setFont(System5x7);
        _display.println(F("==== Select ROM ===="));
        _display.println();
    } else {
        _display.clear(0, 150, 2, 50);
        _display.setRow(2);
    }

    _display.setFont(Verdana12);
    _display.print("  ");
    _display.println(prev == "" ? "-" : prev);

    //_display.setFont(Callibri15);
    _display.print("[ ");
    _display.print(curr);
    _display.println(" ]");

    _display.print("  ");
    _display.println(next == "" ? "-" : next);
}

void printConfirmationMsg(String filename) {
    _display.clear();
    _display.setFont(System5x7);
    _display.println(F("=== Configure ROM ==="));
    _display.println();

    _display.setFont(Verdana12);
    _display.println(filename);

    _display.print("[ LOW ]");
    _display.print("  HIGH  ");
}

void printUpperLowerSelection(bool higher) {
    _display.setFont(Verdana12);
    _display.setRow(4);
    _display.setCol(0);
    if (higher) {
        _display.print("  LOW  ");
        _display.print("[ HIGH ]");
    } else {
        _display.print("[ LOW ]");
        _display.print("  HIGH  ");
    }
}

void printStartLoading(String filename) {
    _display.clear();
    _display.setFont(System5x7);
    _display.println("====== Loading ======");
    _display.println();
    _display.setFont(Verdana12);
    _display.print("[ ");
    _display.print(filename);
    _display.println(" ]");
    _display.print("[");
    _display.setCol(95);
    _display.println("]");
}

void printLoadingProgress(String filename, uint8_t progress) {
    _display.setRow(4);
    _display.setCol(4);

    for (uint8_t i = 0; i < progress; i++) {
        _display.print("#");
    }
}

void printLoadingComplete(bool error) {
    _display.clear();
    _display.setFont(System5x7);
    if (error) {
        _display.println("Load error!");
    } else {
        _display.println("Ready.");
    }
    _display.println();
}

void printSave() {
    _display.clear();
    _display.setFont(System5x7);
    _display.println("ROM configured.");
    _display.println();
    _display.println("Please restart CPC");
    _display.println();
}

} // namespace

void setup() { _display.begin(&Adafruit128x64, I2C_ADDRESS); currentState = INIT; }

void update(const DisplayState &state, const DisplayData &data) {
    switch (state) {
    case INIT: {
        printInit();
        break;
    }
    case LIST: {
        printCurrentROMs(data.m_lower_rom, data.m_upper_rom);
        break;
    }
    case BROWSE:
        DEBUG_PRINT("Prev: ");
        DEBUG_PRINTLN(data.m_prev_filename);
        DEBUG_PRINT("Current: ");
        DEBUG_PRINTLN(data.m_current_filename);
        DEBUG_PRINT("Next: ");
        DEBUG_PRINTLN(data.m_next_filename);

        printFilename(data.m_prev_filename, data.m_current_filename, data.m_next_filename);
        break;
    case CONFIRM:
        if (currentState == CONFIRM) {
            printUpperLowerSelection(data.m_bank_select);
        } else {
            printConfirmationMsg(data.m_current_filename);
        }
        break;
    case CONFIG:
        printSave();
    case LOADING:
        if (data.m_load_state == LoadState::STARTED) {
            printStartLoading(data.m_current_filename);
        } else if (data.m_load_state == LoadState::FINISHED) {
            printLoadingComplete(data.m_load_error);
        } else { // loading in progress
            printLoadingProgress(data.m_current_filename, data.m_load_progress);
        }
        break;
    default:
        break;
    }

    currentState = state; // keep record of the current state
}

} // namespace display