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

    if(just_changed_modes) {
        settingSelect = 0;
    }

    

    const bool GRAPHICS[3][9][3] = {
        // TIME GRAPHICS
        {
            {0, 0, 1},
            {1, 0, 0},
            {1, 1, 1},
            {1, 0, 0},
            {1, 1, 1},
            {0, 0, 0},
            {1, 1, 1},
            {1, 1, 0},
            {1, 1, 1}
        },
        // BRIGHTNESS GRAPHICS
        {
            {0, 1, 0},
            {1, 1, 1},
            {1, 1, 1},
            {0, 1, 1},
            {1, 1, 1},
            {1, 0, 0},
            {1, 0, 1},
            {1, 1, 1},
            {1, 0, 1}
        },
        // UPDATE GRAPHICS
        {
            {1, 0, 0},
            {1, 1, 1},
            {0, 0, 1},
            {1, 1, 1},
            {1, 1, 1},
            {1, 1, 0},
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
    }

    // Set the graphics for the current menu item
    if(select.justPressed || just_changed_modes) {
        just_changed_modes = false;
        uint8_t r = 9;
        while(r--) {
            matrix_driver::setRow(r, GRAPHICS[settingSelect][r]);
        }
    }
    
    if(select.justLongPressed) {
        just_changed_modes = true;
        switch(settingSelect) {
        case TIM:
            currentMode = MODE_SET_TIME;
            break;
        case BRI:
            currentMode = MODE_SET_BRIGHTNESS;
            break;
        case UPD:
            currentMode = MODE_SET_UPDATE;
            break;
        }
    } else if(back.justPressed || back.justLongPressed) {
        currentMode = MODE_TIME;
        just_changed_modes = true;
    }
}

AsyncDelay blink_light(ASYNC_MILLIS, 333);

void mode_set_time(Button select, Button back) {
    static uint8_t new_time[4];
    static uint8_t selected_time(0);
    const uint8_t max_times[4] = {2, 9, 5, 9};

    if(just_changed_modes) {
        just_changed_modes = false;

        selected_time = 0;

        DateTime now = clock.now();
        new_time[0] = now.hour() / 10;
        new_time[1] = now.hour() % 10;
        new_time[2] = now.minute() / 10;
        new_time[3] = now.minute() % 10;

        uint8_t i = 4;
        while(i--) {
            matrix_driver::setSection(i, new_time[i]);
        }
    }

    // process the blink
    static bool blink(false);
    if(blink_light.finished(true)) {
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
        new_time[selected_time] += select.justPressed ? 1 : -1;

        // wrap the time around 0 and max_time
        new_time[selected_time] = 
            new_time[selected_time] == 255 ? // negative overflow
                max_times[selected_time] :
            new_time[selected_time] == max_times[selected_time] + 1 ?
                0 : 
                new_time[selected_time];
        
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
        uint8_t i = 4;
        while(i--) {
            matrix_driver::setSection(i, new_time[i]);
        }

        selected_time++;
        if(selected_time > 3) {
            selected_time = 0;
        }

        matrix_driver::setSection(selected_time, 0);
    } else if(back.justLongPressed) {
        // on a long press of back, go back to the previous screen.
        currentMode = MODE_SETTINGS;
        just_changed_modes = true;

        clock.adjust(DateTime(0, 0, 0, new_time[0] * 10 + new_time[1], new_time[2] * 10 + new_time[3], 0));
    }
}

void mode_set_brightness(Button select, Button back) {
    static uint8_t brightness(0);

    if(just_changed_modes) {
        just_changed_modes = false;
        const bool b[3][3] = {
            {1, 1, 1},
            {1, 1, 1},
            {0, 1, 1}
        };
        matrix_driver::setRow(1, b[0]);
        matrix_driver::setRow(2, b[1]);
        matrix_driver::setRow(3, b[2]);

        brightness = round(settings::brightness::get() / 12.75);
        matrix_driver::setSection(2, brightness / 10);
        matrix_driver::setSection(3, brightness % 10);
    }


    if(select.justPressed || back.justPressed) {
        // on a short press of either button, increase or decrease the brightness
        brightness += select.justPressed ? 1 : -1;
        // wrap the brightness around 0 and 20
        brightness = 
            brightness == 21 ?
                0 :
            brightness == 255 ? // negative overflow
                20 :
                brightness;

        matrix_driver::setSection(2, brightness / 10);
        matrix_driver::setSection(3, brightness % 10);

        settings::brightness::set(round(brightness * 12.75));
    } else if(select.justLongPressed || back.justLongPressed) {
        currentMode = MODE_SETTINGS;
        just_changed_modes = true;
    }
}

void mode_set_update(Button select, Button back) {
    static uint8_t selected(0);

    // all numbers divisible by 60
    const uint8_t options_size = 12;
    const uint8_t options[options_size] = {1, 2, 3, 4, 5, 6, 10, 12, 15, 20, 30, 60};

    if(just_changed_modes) {
        just_changed_modes = false;

        uint8_t value = settings::update_time::get() / 1000;

        // find the closest index to the saved value
        selected = options_size - 1;
        for(uint8_t i = 0; i < options_size - 1; i++) {
            if(value <= options[i] + ((options[i + 1] - options[i]) / 2)) {
                selected = i;
                break;
            }
        }

        matrix_driver::setSection(1, options[selected] / 60);
        matrix_driver::setSection(2, (options[selected] % 60) / 10);
        matrix_driver::setSection(3, (options[selected] % 60) % 10);

        update_clock.start();
    }

    if(update_clock.finished()) {
        matrix_driver::randomizeLocations();
    }

    if(select.justPressed || back.justPressed) {
        selected += select.justPressed ? 1 : -1;

        // wrap the selected around the array
        selected = 
            selected == 255 ?
                options_size - 1 :
            selected == options_size ?
                0 :
                selected;
        
        settings::update_time::set(options[selected] * 1000);

        matrix_driver::setSection(1, options[selected] / 60);
        matrix_driver::setSection(2, (options[selected] % 60) / 10);
        matrix_driver::setSection(3, (options[selected] % 60) % 10);

        update_clock.start(options[selected] * 1000);
    } else if(select.justLongPressed || back.justLongPressed) {
        just_changed_modes = true;
        currentMode = MODE_SETTINGS;
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
    case MODE_SET_BRIGHTNESS:
        mode_set_brightness(select, back);
        break;
    case MODE_SET_UPDATE:
        mode_set_update(select, back);
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