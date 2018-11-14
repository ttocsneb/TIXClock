#ifndef __MODES_H__
#define __MODES_H__

enum Mode {
    MODE_TIME,
    MODE_SETTINGS,
    MODE_SET_TIME,
    MODE_SET_UPDATE,
    MODE_SET_BRIGHTNESS
};

struct Button {
    bool justPressed = false;
    bool justLongPressed = false;
};

namespace modes {
    Mode getMode();

    void update(Button select, Button back);

    void begin();
};

#endif