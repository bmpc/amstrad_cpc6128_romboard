#pragma once
#ifndef _CONTROL_H_
#define _CONTROL_H_

#define BTN_UP 6  // D6
#define BTN_DOWN 14  // A0
#define BTN_CONFIRM 15 // A1
#define BTN_CANCEL 16  // A2

#define PIN_BUSREQ 17
#define PIN_BUSACK 9

namespace controller {
    void init();
    void loop();
    void next();
    void confirm();
    void cancel();
};

#endif // _CONTROL_H_