#include <Arduino.h>

#include <RTClib.h>
#include <avr/wdt.h>

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
    wdt_disable();

    // We don't need to start Wire as it is started in clock.
    clock.begin();

    pinMode(BTN_BAK, INPUT_PULLUP);
    pinMode(BTN_SEL, INPUT_PULLUP);

    //Set the time to 2359 if the time has stopped
    if(!clock.isrunning()) {
        clock.adjust(DateTime(2000, 1, 1, 23, 59, 0));
    }

    settings::begin();

    const uint8_t rows[4] = {SEL_0, SEL_1, SEL_2, ROW_8};
    const uint8_t cols[3] = {COL_0, COL_1, COL_2};
    matrix_driver::begin(rows, cols, ENABLE);

    // Setup the watchdog
    wdt_enable(WDTO_500MS);
}

void updateTime() {
    // Update the current time
    DateTime datetime = clock.now();

    current_hour = datetime.hour();
    current_min = datetime.minute();
}

// Set Time Functions

AsyncDelay selDelay(ASYNC_MILLIS, 1000, false);
AsyncDelay bakDelay(ASYNC_MILLIS, 1000, false);
AsyncDelay pressDelay(ASYNC_MILLIS, 5);

void loop() {
    wdt_reset();

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
        last_bak_state = bak;
        if(bak) {
            bakDelay.start();
        } else if(bakDelay.isEnabled() && !bakDelay.finished(false)) {
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
        last_sel_state = sel;
        if(sel) {
            selDelay.start();
        } else if(selDelay.isEnabled() && !selDelay.finished(false)) {
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

    // update at 60Hz
    delay(16);

    // matrix_driver::update();
}