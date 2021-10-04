#pragma once
#ifndef _CONTROL_H_
#define _CONTROL_H_

#define BTN_INT 2
#define BTN_BROWSE 14
#define BTN_CONFIRM 15
#define BTN_CANCEL 16

namespace controller {
    void init();
    void loop();
    void browseNext();
    void confirm();
    void cancel();
};

#endif // _CONTROL_H_