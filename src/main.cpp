#include <Arduino.h>

#include <RTClib.h>

#include "matrix_driver.h"
#include "modes.h"


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

void loop() {

    // Process the button presses

    static bool last_sel_state(false);
    static bool last_bak_state(false);
    static unsigned long last_btn_press(0);
    static unsigned long sel_press_time(0);
    static unsigned long bak_press_time(0);

    Button select;
    Button back;

    bool sel = digitalRead(BTN_SEL);
    bool bak = digitalRead(BTN_BAK);

    if((sel != last_sel_state || bak != last_bak_state)
        && millis() - last_btn_press > 25) {
        
        last_btn_press = millis();
        
        if(sel != last_sel_state) {
            last_sel_state = sel;

            if(sel) {
                if(millis() - sel_press_time > LONG_PRESS_TIME) {
                    select.justLongPressed = true;
                } else {
                    select.justPressed = true;
                }
            } else {
                sel_press_time = millis();
            }
        } else {
            last_bak_state = bak;

            if(bak) {
                if(millis() - bak_press_time > LONG_PRESS_TIME) {
                    back.justLongPressed = true;
                } else {
                    back.justPressed = true;
                }
            } else {
                bak_press_time = millis();
            }
        }
    }

    modes::update(select, back);

    matrix_driver::update();
}