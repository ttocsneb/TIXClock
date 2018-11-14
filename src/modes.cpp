#include "modes.h"

#include <Arduino.h>

#include "settings.h"
#include "matrix_driver.h"
#include "async_delay.h"
#include "main.h"

Mode currentMode = MODE_TIME;

Mode modes::getMode() {
    return currentMode;
}

AsyncDelay update_clock(ASYNC_MILLIS);
bool just_changed_modes = true;

void mode_time() {
    if(just_changed_modes || update_clock.finished(true)) {
        just_changed_modes = false;

        DateTime now = clock.now();

        matrix_driver::setSection(0, now.hour() / 10);
        matrix_driver::setSection(1, now.hour() % 10);

        matrix_driver::setSection(2, now.minute() / 10);
        matrix_driver::setSection(3, now.minute() % 10);

        matrix_driver::randomizeLocations();
    }
}

void mode_settings(Button select, Button back) {
    #define TIM 0
    #define BRI 1
    #define UPD 2
    static uint8_t settingSelect(0);

    const bool GRAPHICS[3][9][3] = {
        // TIME GRAPHICS
        {
            {1, 0, 0},
            {0, 0, 1},
            {1, 1, 1},
            {0, 0, 1},
            {1, 1, 1},
            {0, 0, 0},
            {1, 1, 1},
            {0, 1, 1},
            {1, 1, 1}
        },
        // BRIGHTNESS GRAPHICS
        {
            {0, 1, 0},
            {1, 1, 1},
            {1, 1, 1},
            {1, 1, 0},
            {1, 1, 1},
            {0, 0, 1},
            {1, 0, 1},
            {1, 1, 1},
            {1, 0, 1}
        },
        // UPDATE GRAPHICS
        {
            {0, 0, 1},
            {1, 1, 1},
            {1, 0, 0},
            {1, 1, 1},
            {1, 1, 1},
            {0, 1, 1},
            {1, 1, 1},
            {1, 0, 1},
            {0, 1, 0}
        }
    };

    if(select.justPressed) {
        settingSelect++;
        if(settingSelect > 2) {
            settingSelect = 0;
        }
    } else if(select.justLongPressed) {
        switch(settingSelect) {
        case TIM:
            currentMode = MODE_SET_TIME;
        case BRI:
            currentMode = MODE_SET_BRIGHTNESS;
        case UPD:
            currentMode = MODE_SET_UPDATE;
        }
    } else if(back.justPressed || back.justLongPressed) {
        currentMode = MODE_TIME;
        just_changed_modes = true;
    }

    // Set the graphics for the current menu item
    if(select.justPressed || just_changed_modes) {
        just_changed_modes = false;
        uint8_t r = 9;
        while(r--) {
            matrix_driver::setRow(r, GRAPHICS[settingSelect][r]);
        }
    }
}

void modes::update(Button select, Button back) {
    switch(currentMode) {
    case MODE_SETTINGS:
        mode_settings(select, back);
        break;
    default:
    case MODE_TIME:
        mode_time();
        // process the button presses for this mode
        if(select.justLongPressed || back.justLongPressed) {
            currentMode = MODE_SETTINGS;
            just_changed_modes = true;
        }
    }
}

void modes::begin() {
    update_clock.setDelay(settings::update_time::get());
}