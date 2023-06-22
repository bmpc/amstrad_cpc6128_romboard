#include "controller.h"
#include "eepromflash.h"
#include "localstorage.h"
#include "log.h"

namespace cpc_rom_board {

void Controller::updateDisplay(DisplayState state, DisplayData display_data) {
    m_state = state;
    m_display.update(m_state, display_data);
}

void Controller::on_progress(uint8_t prog) {
    updateDisplay(LOADING, DisplayData::withLoadProgress(LoadState::PROGRESS, prog));
}

DisplayData Controller::getListDisplayData() {
    local_storage::RomPair pair = local_storage::load();
    return DisplayData::withLowerAndUpperRoms(pair.lowerRom, pair.upperRom);
}

DisplayData Controller::getBrowseDisplayData() {
    return DisplayData::withBrowseFiles(m_file_iterator.getPreviousFilename(), m_file_iterator.getCurrentFilename(), m_file_iterator.getNextFilename());
}

bool Controller::loadRom(const char* filename, bool lower) {
    DEBUG_PRINT("Loading file [");
    DEBUG_PRINT(filename);
    DEBUG_PRINT("] in bank ");
    DEBUG_PRINTLN(lower ? "1" : "2");

    updateDisplay(LOADING, DisplayData::withLoadProgress(filename, 0, LoadState::STARTED, false));

    File romFile = m_file_iterator.findFile(filename);
    if (!romFile) { // if the rom file can't be found/opened for some reason
        DEBUG_PRINTLN("ROM file not found!");
        local_storage::erase(lower);
        return false;
    } else {
        // bool success = eeprom::flash(romFile, lower, [](uint8_t prog) {
        bool success = sram::load(romFile, lower, *this);

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

void Controller::loadRoms() {
    // this wait should not be necessary as ages have passed after we put the BUSREQ LOW...
    while (digitalRead(PIN_BUSACK) != LOW) {
        delay(10);
    }

    local_storage::RomPair pair = local_storage::load();

    bool success = true;
    if (*pair.lowerRom != '\0' || *pair.upperRom != '\0') {
        if (*pair.lowerRom != '\0') {
            success &= loadRom(pair.lowerRom, true);
        }
        if (*pair.upperRom != '\0') {
            success &= loadRom(pair.upperRom, false);
        }

        updateDisplay(LOADING, DisplayData::withLoadProgress("", 0, LoadState::FINISHED, !success));

        digitalWrite(PIN_BUSREQ, HIGH);

        delay(2000); // wait for 1s so that user can see if there was an error loading a ROM
    } else {
        digitalWrite(PIN_BUSREQ, HIGH);
    }

    updateDisplay(LIST, getListDisplayData());
}

void Controller::init() {
    DEBUG_PRINTLN("Initializing ROM Board...");

    m_btn_up.setDebounceTime(50);
    m_btn_down.setDebounceTime(50);
    m_btn_confirm.setDebounceTime(50);
    m_btn_cancel.setDebounceTime(50);

    m_display.setup();
    updateDisplay(INIT, DisplayData());

    m_file_iterator.setupSD();

    pinMode(PIN_BUSACK, INPUT_PULLUP);
    digitalWrite(PIN_BUSREQ, LOW);
    pinMode(PIN_BUSREQ, OUTPUT);

    delay(1000);

    loadRoms();
}

void Controller::next() {
    if (m_state == LIST) {
        m_file_iterator.reset();
        m_file_iterator.moveNext();
        m_bank_selector = 0;
        updateDisplay(BROWSE, getBrowseDisplayData());
    } else if (m_state == BROWSE) {
        m_file_iterator.moveNext(); // Move to next filename

        updateDisplay(BROWSE, getBrowseDisplayData());
    } else if (m_state == CONFIRM) {
        m_bank_selector = !m_bank_selector;
        updateDisplay(CONFIRM, DisplayData::withConfirmFile(m_file_iterator.getCurrentFilename(), m_bank_selector));
    }
}

void Controller::previous() {
    if (m_state == LIST) {
        m_file_iterator.reset();
        m_file_iterator.moveNext();
        m_bank_selector = 0;
        updateDisplay(BROWSE, getBrowseDisplayData());
    } else if (m_state == BROWSE) {
        m_file_iterator.movePrevious(); // Move to the previous filename

        updateDisplay(BROWSE, getBrowseDisplayData());
    } else if (m_state == CONFIRM) {
        m_bank_selector = !m_bank_selector;
        updateDisplay(CONFIRM, DisplayData::withConfirmFile(m_file_iterator.getCurrentFilename(), m_bank_selector));
    }
}

void Controller::confirm() {
    if (m_state == BROWSE) {
        m_bank_selector = 0;
        updateDisplay(CONFIRM, DisplayData::withConfirmFile(m_file_iterator.getCurrentFilename(), m_bank_selector));
    } else if (m_state == CONFIRM) {
        File currentFile = m_file_iterator.getCurrentFile();
        if (!currentFile) { // if the current file can't be found/opened for some reason
            m_file_iterator.reset();
            updateDisplay(LIST, getListDisplayData());

            DEBUG_PRINTLN("Invalid ROM file!");
            return;
        }

        currentFile.close();

        String filename = m_file_iterator.getCurrentFilename();

        char currentName[13];
        filename.toCharArray(currentName, 13);
        DEBUG_PRINT("Saving filename [");
        DEBUG_PRINT(currentName);
        DEBUG_PRINTLN("]");
        local_storage::save(currentName, !m_bank_selector);

        m_bank_selector = 0;
        m_file_iterator.reset();

        updateDisplay(CONFIG, DisplayData());

        delay(2000);

        updateDisplay(LIST, getListDisplayData());
    }
}

void Controller::cancel() {
    if (m_state == BROWSE || m_state == CONFIRM) {
        m_file_iterator.reset();
        m_bank_selector = 0;
        updateDisplay(LIST, getListDisplayData());
    }
}

void Controller::loop() {
    m_btn_up.loop();
    m_btn_down.loop();
    m_btn_confirm.loop();
    m_btn_cancel.loop();

    if (m_btn_cancel.isPressed()) {
        cancel();
    } else if (m_btn_up.isPressed()) {
        previous();
    } else if (m_btn_down.isPressed()) {
        next();
    } else if (m_btn_confirm.isPressed()){
        confirm();
    }
}

} // namespace
