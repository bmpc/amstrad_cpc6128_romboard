#include "controller.h"
#include "display.h"
#include "eepromflash.h"
#include "sram.h"
#include "fileiterator.h"
#include "localstorage.h"
#include "log.h"

namespace controller {
namespace {
DisplayState m_state = LIST;
bool m_bank_selector = 0;

FlashState m_flash_state = FlashState::IDLE;
uint8_t m_flash_progress = 0;
bool m_flash_error = false;

volatile uint8_t m_int_btn = 0;
uint64_t m_lastFire = 0;

void updateDisplay() {
    switch (m_state) {
    case LIST: {
        local_storage::RomPair pair = local_storage::load();
        display::update(m_state, display::DisplayData::withLowerAndUpperRoms(pair.lowerRom, pair.upperRom));
    } break;
    case BROWSE:
        display::update(m_state, display::DisplayData::withBrowseFiles(file_iterator::getPreviousFilename(), file_iterator::getCurrentFilename(), file_iterator::getNextFilename()));
        break;
    case CONFIRM:
        display::update(m_state, display::DisplayData::withConfirmFile(file_iterator::getCurrentFilename(), m_bank_selector));
        break;
    case FLASHING:
        display::update(m_state, display::DisplayData::withFlashProgress(file_iterator::getCurrentFilename(), m_flash_progress, m_flash_state, m_flash_error));
        break;
    }
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

} // anonymous namespace

void init() {
    file_iterator::setupSD();
    display::setup();

    pinMode(BTN_INT, INPUT_PULLUP);
    pinMode(BTN_BROWSE, OUTPUT);
    digitalWrite(BTN_BROWSE, LOW);

    attachInterrupt(digitalPinToInterrupt(BTN_INT), &intButtonHandler, FALLING);

    updateDisplay();
}

void browseNext() {
    if (m_state == LIST) {
        file_iterator::reset();
        file_iterator::moveNext();
        m_bank_selector = 0;
        m_state = BROWSE; // Select ROM
        updateDisplay();
    } else if (m_state == BROWSE) {
        file_iterator::moveNext(); // Move to next filename

        updateDisplay();
    } else if (m_state == CONFIRM) {
        m_bank_selector = !m_bank_selector;
        updateDisplay();
    } else if (m_state == FLASHING && m_flash_state == FlashState::FINISHED) {
        m_state = LIST;
        m_flash_state = FlashState::IDLE;
        updateDisplay();
    }
}

void confirm() {
    if (m_state == BROWSE) {
        m_state = CONFIRM; // Confirm flashing
        m_bank_selector = 0;
        updateDisplay();
    } else if (m_state == CONFIRM) {
        File currentFile = file_iterator::getCurrentFile();
        if (!currentFile) { // if the current file can't be found/opened for some reason
            m_state = LIST;
            file_iterator::reset();
            updateDisplay();

            DEBUG_PRINTLN("Invalid ROM file!");
            return;
        }

        String filename = file_iterator::getCurrentFilename();

        DEBUG_PRINT("Flashing file [");
        DEBUG_PRINT(filename);
        DEBUG_PRINT("] in bank ");
        DEBUG_PRINTLN(m_bank_selector ? "HIGH" : "LOW");

        m_state = FLASHING; // flashing
        m_flash_state = FlashState::STARTED;
        updateDisplay();

        bool success = sram::load(currentFile, !m_bank_selector, [](uint8_t prog) {
            display::update(FLASHING, display::DisplayData::withFlashProgress(FlashState::PROGRESS, prog));
        });

        /*
            // Simulate flashing...
            for (int i = 1; i <= 10; i++) {
                pr.progress(i);
                delay(200);
            }
            */

        currentFile.close();
        if (success) {
            m_flash_error = false;
            DEBUG_PRINTLN("Flashing finished with success.");
            char currentName[13];
            filename.toCharArray(currentName, 13);
            DEBUG_PRINT("Saving filename [");
            DEBUG_PRINT(currentName);
            DEBUG_PRINTLN("]");
            local_storage::save(currentName, !m_bank_selector);
        } else {
            m_flash_error = true;
            DEBUG_PRINTLN("Flashing finished with error!");
            local_storage::erase(!m_bank_selector);
        }

        m_state = FLASHING;
        m_flash_state = FlashState::FINISHED;
        m_flash_progress = 0;
        m_bank_selector = 0;
        file_iterator::reset();
        updateDisplay();

    } else if (m_state == FLASHING && m_flash_state == FlashState::FINISHED) {
        m_state = LIST;
        m_flash_state = FlashState::IDLE;
        updateDisplay();
    }
}

void cancel() {
    if (m_state == BROWSE || m_state == CONFIRM || (m_state == FLASHING && m_flash_state == FlashState::FINISHED)) {
        file_iterator::reset();
        m_bank_selector = 0;
        m_flash_state = FlashState::IDLE;
        m_state = LIST;
        updateDisplay();
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
