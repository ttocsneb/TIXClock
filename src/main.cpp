#include <Arduino.h>

#include <RTClib.h>

#include "matrix_driver.h"
#include "modes.h"
#include "settings.h"
#include "async_delay.h"


#define MODE_TIME false
#define MODE_SET  true

#define COL_0 3
#define COL_1 5
#define COL_2 6

#define SEL_0 7
#define SEL_1 8
#define SEL_2 9
#define ROW_8 2
#define ENABLE 10

#define BTN_SEL 12
#define BTN_BAK 11

RTC_DS1307 clock;

bool draw_enable = true;
bool update = false;
bool current_backup_state[10][3];
bool current_state[10][3];

uint8_t current_hour;
uint8_t current_min;


void setup() {
    // We don't need to start Wire as it is started in clock.
    clock.begin();

    settings::begin();

    const uint8_t rows[4] = {SEL_0, SEL_1, SEL_2, ROW_8};
    const uint8_t cols[3] = {COL_0, COL_1, COL_2};
    matrix_driver::begin(rows, cols, ENABLE);

}

void updateTime() {
    // Update the current time
    DateTime datetime = clock.now();

    current_hour = datetime.hour();
    current_min = datetime.minute();
}

// Set Time Functions

#define LONG_PRESS_TIME 1500

AsyncDelay selDelay(ASYNC_MILLIS, 1500);
AsyncDelay bakDelay(ASYNC_MILLIS, 1500);
AsyncDelay pressDelay(ASYNC_MILLIS, 5);

void loop() {

    // Process the button presses

    static bool last_sel_state(false);
    static bool last_bak_state(false);

    Button select;
    Button back;

    // We invert the signals since a press is pulled low
    bool sel = !digitalRead(BTN_SEL);
    bool bak = !digitalRead(BTN_BAK);

    // Process back button
    if(bak != last_bak_state && pressDelay.finished(true)) {
        if(bak) {
            bakDelay.start();
        } else if(bakDelay.finished(false)) {
            // the press was shorter than a long press,
            // therefore it is a short press.
            back.justPressed = true;
            bakDelay.enable(false);
        }
        pressDelay.start();
    }
    if(bakDelay.finished(false)) {
        // they didn't stop holding the button for long enough,
        // this is a long press
        bakDelay.enable(false);
        back.justLongPressed = true;
    }
    // Process select button
    if(sel != last_sel_state && pressDelay.finished(true)) {
        if(sel) {
            selDelay.start();
        } else if(selDelay.finished(false)) {
            // the press was shorter than a long press,
            // therefore it is a short press.
            select.justPressed = true;
            selDelay.enable(false);
        }
        pressDelay.start();
    }
    if(selDelay.finished(false)) {
        // they didn't stop holding the button for long enough,
        // this is a long press
        selDelay.enable(false);
        select.justLongPressed = true;
    }

    modes::update(select, back);

    matrix_driver::update();
}