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
    if(just_changed_modes) {
        update_clock.start(settings::update_time::get());
    }
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

void mode_set_time(Button select, Button back) {
    static uint8_t new_time[4];
    static uint8_t selected_time = 0;
    const uint8_t max_times[4] = {2, 9, 5, 9};

    if(just_changed_modes) {
        update_clock.start(500);

        DateTime now = clock.now();
        new_time[0] = now.hour() / 10;
        new_time[1] = now.hour() % 10;
        new_time[2] = now.minute() / 10;
        new_time[3] = now.minute() % 10;
    }

    // process the blink
    static bool blink(false);
    if(update_clock.finished()) {
        blink = !blink;
    }
    if(!blink) {
        matrix_driver::setSection(selected_time, 0);
    } else {
        matrix_driver::setSection(selected_time, new_time[selected_time]);
    }

    // process the button presses
    if(select.justPressed || back.justPressed) {
        // on a short press of either button, increase or decrease the time

        if(select.justPressed) {
            new_time[selected_time]++;
            
            if(new_time[selected_time] > max_times[selected_time] ||
                // edge case to wrap 24 hour days
                (selected_time == 1 && new_time[selected_time] > 3)) {
                new_time[selected_time] = 0;
            }
        } else {
            new_time[selected_time]--;
            
            // we don't need to worry about the wrapping 24h edge case as
            // the below case also covers it
            if(new_time[selected_time] > max_times[selected_time]) {
                new_time[selected_time] = max_times[selected_time];
            }
        }
        
        // an edge case to prevent more than 24 hour days
        if(new_time[0] == 2) {
            new_time[1] = min(new_time[1], 3);
        }

        if(blink) {
            matrix_driver::setSection(selected_time, new_time[selected_time]);
        }
    } else if(select.justLongPressed) {
        // on a long press of select, go the the next mode.
        blink = false;
        matrix_driver::setSection(selected_time, new_time[selected_time]);

        selected_time++;
        if(selected_time > 3) {
            selected_time = 0;
        }

        matrix_driver::setSection(selected_time, 0);
    } else if(back.justLongPressed) {
        // on a long press of back, go back to the previous screen.
        currentMode = MODE_SETTINGS;
        just_changed_modes = true;
    }
}

void modes::update(Button select, Button back) {
    switch(currentMode) {
    case MODE_SETTINGS:
        mode_settings(select, back);
        break;
    case MODE_SET_TIME:
        mode_set_time(select, back);
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

}