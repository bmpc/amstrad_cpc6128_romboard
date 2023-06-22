#pragma once
#ifndef _CONTROL_H_
#define _CONTROL_H_

#define BTN_UP 6  // D6
#define BTN_DOWN 14  // A0
#define BTN_CONFIRM 15 // A1
#define BTN_CANCEL 16  // A2

#define PIN_BUSREQ 17
#define PIN_BUSACK 9

#include "ezButton.h"
#include "display.h"
#include "fileiterator.h"
#include "sram.h"

namespace cpc_rom_board {
class Controller : public sram::SRamListener {

public:
    Controller() = default;
    void init();
    void loop();
    void next();
    void previous();
    void confirm();
    void cancel();
    virtual void on_progress(uint8_t) override;

private:
    DisplayState m_state = INIT;
    bool m_bank_selector = 0;
    FileIterator m_file_iterator;
    Display m_display;

    ezButton m_btn_up = BTN_UP;
    ezButton m_btn_down = BTN_DOWN;
    ezButton m_btn_confirm = BTN_CONFIRM;
    ezButton m_btn_cancel = BTN_CANCEL;

    void updateDisplay(DisplayState, DisplayData);
    DisplayData getListDisplayData();
    DisplayData getBrowseDisplayData();
    void loadRoms();
    bool loadRom(const char* filename, bool lower);
};
}

#endif // _CONTROL_H_