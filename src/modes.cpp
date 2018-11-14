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

void modes::update(Button select, Button back) {
    if(currentMode == MODE_TIME) {
        mode_time();
    }

}

void modes::begin() {
    update_clock.setDelay(settings::update_time::get());
}