#include <Arduino.h>

#include <RTClib.h>

#include "bus.h"


#define MODE_TIME false
#define MODE_SET  true


#define ROW_8 2
#define ROW_9 5

#define COL_0 3
#define COL_1 5
#define COL_2 6

#define SEL_0 7
#define SEL_1 8
#define SEL_2 9
#define ENABLE 10

Bus<uint8_t> select(3, SEL_2, SEL_1, SEL_0);

#define BTN_SEL 13
#define BTN_BAK 12

RTC_DS1307 clock;

bool draw_enable = true;
bool update = false;
bool current_backup_state[10][3];
bool current_state[10][3];

uint8_t current_hour;
uint8_t current_min;

bool btn_select_just_pressed = false;
bool btn_select_just_long_pressed = false;
bool btn_back_just_pressed = false;
bool btn_back_just_long_pressed = false;


void setup() {
    // We don't need to start Wire as it is started in clock.
    clock.begin();

    pinMode(ROW_8, OUTPUT);
    pinMode(ROW_9, OUTPUT);

    pinMode(COL_0, OUTPUT);
    pinMode(COL_1, OUTPUT);
    pinMode(COL_2, OUTPUT);
    
    select.begin(OUTPUT);
    pinMode(ENABLE, OUTPUT);

    pinMode(BTN_SEL, INPUT_PULLUP);
    pinMode(BTN_BAK, INPUT_PULLUP);

    for(uint8_t i = 0; i < 10; i++) {
        for(uint8_t ii = 0; ii < 3; ii++) {
            current_state[i][ii] = false;
        }
    }

}

#define update_frequency 1000
#define DELAY_TIME static_cast<unsigned long>(1.0 / update_frequency * 100000)

void draw() {
    static unsigned long last_update_time(0);
    // Draw the current time

    //keep the draw time as close as possible to 1kHz
    if(micros() - last_update_time < DELAY_TIME)
        delayMicroseconds(DELAY_TIME - (micros() - last_update_time));

    // turn off the last row from the previous draw
    digitalWrite(ROW_9, LOW);

    digitalWrite(ENABLE, HIGH);
    for(uint8_t i=0; i < 10; i++) {
        digitalWrite(COL_0, LOW);
        digitalWrite(COL_1, LOW);
        digitalWrite(COL_2, LOW);

        // Enable the current row
        if(i == 8) {
            digitalWrite(ENABLE, LOW);
            digitalWrite(ROW_8, HIGH);
        } else if(i == 9) {
            digitalWrite(ROW_8, LOW);
            digitalWrite(ROW_9, HIGH);
        } else {
            select.write(i);
        }

        //write the current row's leds
        digitalWrite(COL_0, current_state[i][0]);
        digitalWrite(COL_1, current_state[i][1]);
        digitalWrite(COL_2, current_state[i][2]);

        if(i != 9)
            delayMicroseconds(DELAY_TIME);
        else
            last_update_time = micros();
    }

}


void updateTime() {
    // Update the current time
    DateTime datetime = clock.now();

    current_hour = datetime.hour();
    current_min = datetime.minute();
}

// Set Time Functions

#define MODE_HOUR false
#define MODE_MIN true

void add_time(int8_t time, bool mode) {
    if(mode == MODE_HOUR) {
        current_hour += time;
    } else if(mode == MODE_MIN) {
        current_min += time;
    }
}

bool setTime() {

    #define BLINK_ON_TIME 667
    #define BLINK_OFF_TIME 333
    static unsigned long last_blink_time(0);
    static bool blink(true);
    static bool mode(MODE_HOUR);

    // Set the current time

    //blink the selected bits
    if((blink && millis() - last_blink_time > BLINK_ON_TIME)
        || (!blink && millis() - last_blink_time > BLINK_OFF_TIME)) {
        for(uint8_t i = (mode == MODE_MIN ? 4 : 0); i < (mode == MODE_MIN ? 10 : 4); i++) {
            
            for(uint8_t ii = 0; ii < 3; ii++) {
                if(blink) {
                    current_state[i][ii] = false;
                } else {
                    current_state[i][ii] = current_backup_state[i][ii];
                }
            }
        }
    }

    //process button presses
    if(btn_select_just_pressed) {
        add_time(1, mode);
        update = true;
    } else if(btn_select_just_long_pressed) {
        add_time(-1, mode);
        update = true;
    } else if(btn_back_just_pressed) {
        update = true;
        if(mode == MODE_HOUR) {
            mode = MODE_MIN;
        } else if(mode == MODE_MIN) {
            DateTime datetime(0, 0, 0, current_hour, current_min, 0);
            clock.adjust(datetime);
            return true;
        }
    } else if(btn_back_just_long_pressed) {
        mode = MODE_HOUR;
        update = true;
    }

    return false;
}

void process_time() {

    for(uint8_t i = 0; i < 10; i++) {
        for(uint8_t ii = 0; i < 3; i++) {
            current_state[i][ii] = false;
        }
    }

    if(draw_enable) {

        // set the current time
        uint8_t sec[4];
        sec[0] = current_hour / 10;
        sec[1] = current_hour % 10;
        sec[2] = current_min  / 10;
        sec[3] = current_min  % 10;

        // set the first section lights
        if(sec[0] > 0) {
            // Set a random box
            uint8_t select = random(3);
            current_state[0][select] = true;
            if(sec[0] > 1) {
                // If there are 2 boxes, select another random box
                uint8_t select1 = random(2);
                if(select1 == select) {
                    select1++;
                }
                current_state[0][select1] = true;
                if(sec[0] > 2) {
                    // If there are 3 boxes, set the last box
                    if((select1 + 1) % 3 == select) {
                        current_state[0][(select1 - 1) % 3] = true;
                    } else {
                        current_state[0][(select1 + 1) % 3] = true;
                    }
                }
            }
        }

        // set the 3 sections lights
        for(uint8_t i = 1; i < 4; i++) {
            uint8_t sum = 9;
            for(; sec[i] > 0; sec[i]--) {
                // Choose a random box to light up
                uint8_t index = random(sum);
                bool *select = &current_state[index / 3 + (i-1) * 3 + 1][index % 3];
                // move to the next box while the current box is already lit up.
                while(*select) {
                    index++;
                    if(index >= 9) {
                        index = 0;
                    }
                    select = &current_state[index / 3 + (i-1) * 3 + 1][index % 3];
                }
                // set the current box to be true
                *select = true;
                sum--;
            }
        }

        // backup the new state
        for(uint8_t i = 0; i < 10; i++) {
            for(uint8_t ii = 0; ii < 3; ii++) {
                current_backup_state[i][ii] = current_state[i][ii];
            }
        }
    }

}

#define LONG_PRESS_TIME 1500

void loop() {
    static bool mode(MODE_TIME);
    static unsigned long last_display_change;

    static bool last_sel_state(false);
    static bool last_bak_state(false);
    static unsigned long last_btn_press(0);
    static unsigned long sel_press_time(0);
    static unsigned long bak_press_time(0);

    bool sel = digitalRead(BTN_SEL);
    bool bak = digitalRead(BTN_BAK);

    if((sel != last_sel_state || bak != last_bak_state)
        && millis() - last_btn_press > 25) {
        
        last_btn_press = millis();
        
        if(sel != last_sel_state) {
            last_sel_state = sel;

            if(sel) {
                if(millis() - sel_press_time > LONG_PRESS_TIME) {
                    btn_select_just_long_pressed = true;
                } else {
                    btn_select_just_pressed = true;
                }
            } else {
                sel_press_time = millis();
            }
        } else {
            last_bak_state = bak;

            if(bak) {
                if(millis() - bak_press_time > LONG_PRESS_TIME) {
                    btn_back_just_long_pressed = true;
                } else {
                    btn_back_just_pressed = true;
                }
            } else {
                bak_press_time = millis();
            }
        }
    }

    if(mode == MODE_SET) {
        if(setTime()) {
            mode = MODE_TIME;
        }
    }

    //Change the tiles once every 10 seconds
    if(update || millis() - last_display_change > 10000) {  
        if(mode == MODE_TIME) {
            updateTime();
        }
        update = false;
        last_display_change = millis();
        process_time();
    }

    draw();

    btn_back_just_pressed = false;
    btn_select_just_pressed = false;
    btn_select_just_long_pressed = false;
    btn_back_just_long_pressed = false;
}