#include "controller.h"
#include "display.h"
#include "eepromflash.h"
#include "sram.h"
#include "fileiterator.h"
#include "localstorage.h"
#include "log.h"

using namespace display;

namespace controller {
namespace {
DisplayState m_state = INIT;
bool m_bank_selector = 0;

volatile uint8_t m_int_btn = 0;
uint64_t m_lastFire = 0;

void updateDisplay(DisplayState state, DisplayData display_data) {
    m_state = state;
    update(m_state, display_data);
}

DisplayData getListDisplayData() {
    local_storage::RomPair pair = local_storage::load();
    return DisplayData::withLowerAndUpperRoms(pair.lowerRom, pair.upperRom);
}

DisplayData getBrowseDisplayData() {
    return DisplayData::withBrowseFiles(file_iterator::getPreviousFilename(), file_iterator::getCurrentFilename(), file_iterator::getNextFilename());
}

void configDistinct() {
    pinMode(BTN_INT, OUTPUT);
    digitalWrite(BTN_INT, LOW);
    pinMode(BTN_BROWSE, INPUT_PULLUP);
    pinMode(BTN_CONFIRM, INPUT_PULLUP);
    pinMode(BTN_CANCEL, INPUT_PULLUP);
}

void configCommon() {
    pinMode(BTN_INT, INPUT_PULLUP);

    pinMode(BTN_BROWSE, OUTPUT);
    digitalWrite(BTN_BROWSE, LOW);
    pinMode(BTN_CONFIRM, OUTPUT);
    digitalWrite(BTN_CONFIRM, LOW);
    pinMode(BTN_CANCEL, OUTPUT);
    digitalWrite(BTN_CANCEL, LOW);
}

void intButtonHandler() {
    if (millis() - m_lastFire < 200) { // Debounce
        return;
    }
    m_lastFire = millis();

    configDistinct();

    if (!digitalRead(BTN_BROWSE)) {
        DEBUG_PRINTLN("INTERRUPT BTN_BROWSE!");
        m_int_btn = 1;
    }

    if (!digitalRead(BTN_CONFIRM)) {
        DEBUG_PRINTLN("INTERRUPT BTN_CONFIRM!");
        m_int_btn = 2;
    }

    if (!digitalRead(BTN_CANCEL)) {
        DEBUG_PRINTLN("INTERRUPT BTN_CANCEL!");
        m_int_btn = 3;
    }

    configCommon();
}

bool loadRom(String &filename, bool lower) {
    DEBUG_PRINT("Loading file [");
    DEBUG_PRINT(filename);
    DEBUG_PRINT("] in bank ");
    DEBUG_PRINTLN(lower ? "1" : "2");

    updateDisplay(LOADING, DisplayData::withLoadProgress(filename, 0, LoadState::STARTED, false));

    File romFile = file_iterator::findFile(filename);
    if (!romFile) { // if the rom file can't be found/opened for some reason
        DEBUG_PRINTLN("ROM file not found!");
        local_storage::erase(lower);
        return false;
    } else {
        // bool success = eeprom::flash(romFile, lower, [](uint8_t prog) {
        bool success = sram::load(romFile, lower, [](uint8_t prog) {
            updateDisplay(LOADING, DisplayData::withLoadProgress(LoadState::PROGRESS, prog));
        });

        romFile.close();
        if (success) {
            DEBUG_PRINTLN("Finished loading ROM.");
        } else {
            DEBUG_PRINTLN("Error loading ROM!");
            local_storage::erase(lower);
        }

        return success;
    }
}

void loadRoms() {
    // this wait should not be necessary as ages have passed after we put the BUSREQ LOW...
    while (digitalRead(PIN_BUSACK) != LOW) {
        delay(10);
    }

    local_storage::RomPair pair = local_storage::load();

    bool success = true;
    if (pair.lowerRom != "" || pair.upperRom != "") {
        if (pair.lowerRom != "") {
            success &= loadRom(pair.lowerRom, true);
        }
        if (pair.upperRom != "") {
            success &= loadRom(pair.upperRom, false);
        }

        updateDisplay(LOADING, DisplayData::withLoadProgress("", 0, LoadState::FINISHED, !success));

        digitalWrite(PIN_BUSREQ, HIGH);

        delay(2000); // wait for 1s so that user can see if there was an error loading a ROM
    }

    updateDisplay(LIST, getListDisplayData());
}

} // anonymous namespace

void init() {
    DEBUG_PRINTLN("Initializing ROM Board...");

    configCommon();

    display::setup();
    updateDisplay(INIT, DisplayData());

    file_iterator::setupSD();

    pinMode(PIN_BUSACK, INPUT_PULLUP);
    digitalWrite(PIN_BUSREQ, LOW);
    pinMode(PIN_BUSREQ, OUTPUT);

    attachInterrupt(digitalPinToInterrupt(BTN_INT), &intButtonHandler, FALLING);

    delay(1000);

    loadRoms();
}

void browseNext() {
    if (m_state == LIST) {
        file_iterator::reset();
        file_iterator::moveNext();
        m_bank_selector = 0;
        updateDisplay(BROWSE, getBrowseDisplayData());
    } else if (m_state == BROWSE) {
        file_iterator::moveNext(); // Move to next filename

        updateDisplay(BROWSE, getBrowseDisplayData());
    } else if (m_state == CONFIRM) {
        m_bank_selector = !m_bank_selector;
        updateDisplay(CONFIRM, DisplayData::withConfirmFile(file_iterator::getCurrentFilename(), m_bank_selector));
    }
}

void confirm() {
    if (m_state == BROWSE) {
        m_bank_selector = 0;
        updateDisplay(CONFIRM, DisplayData::withConfirmFile(file_iterator::getCurrentFilename(), m_bank_selector));
    } else if (m_state == CONFIRM) {
        File currentFile = file_iterator::getCurrentFile();
        if (!currentFile) { // if the current file can't be found/opened for some reason
            file_iterator::reset();
            updateDisplay(LIST, getListDisplayData());

            DEBUG_PRINTLN("Invalid ROM file!");
            return;
        }

        currentFile.close();

        String filename = file_iterator::getCurrentFilename();

        char currentName[13];
        filename.toCharArray(currentName, 13);
        DEBUG_PRINT("Saving filename [");
        DEBUG_PRINT(currentName);
        DEBUG_PRINTLN("]");
        local_storage::save(currentName, !m_bank_selector);

        m_bank_selector = 0;
        file_iterator::reset();

        updateDisplay(CONFIG, DisplayData());

        delay(2000);

        updateDisplay(LIST, getListDisplayData());
    }
}

void cancel() {
    if (m_state == BROWSE || m_state == CONFIRM) {
        file_iterator::reset();
        m_bank_selector = 0;
        updateDisplay(LIST, getListDisplayData());
    }
}

void loop() {
    if (m_int_btn > 0) {
        switch (m_int_btn) {
        case 1:
            browseNext();
            break;
        case 2:
            confirm();
            break;
        case 3:
            cancel();
            break;
        }

        m_int_btn = 0;
    }
}

} // namespace controller
