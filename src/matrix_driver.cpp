#include "matrix_driver.h"

// Calculate the delay(us) required to get the propper frequency
const unsigned int FREQUENCY_DELAY = 1000000 / (FREQUENCY * 9);

const unsigned int DUTY_TIME = 1000000 / (DUTY_FREQUENCY);

#include <Arduino.h>

#include "bus.h"
#include "async_delay.h"
#include "settings.h"


bool outputs[27];

Bus<uint8_t> *row_bus = NULL;
uint8_t row_8;
uint8_t col_pins[3];
uint8_t enable_pin;

bool leds[9][3];

uint8_t sections[4];
const uint8_t section_index[5] = {0, 1, 4, 6, 9};


void matrix_driver::begin(const uint8_t _row_pins[4], const uint8_t _col_pins[3], uint8_t _enable_pin) {

    // set the row pins
    memcpy(col_pins, _col_pins, sizeof(uint8_t) * 3);
    row_8 = _row_pins[3];
    enable_pin = _enable_pin;

    // Setup all the pins to be outputs
    if(row_bus) {
        // prevent memory leaks from multiple begin calles
        delete row_bus;
        row_bus = NULL;
    }
    row_bus = new Bus<uint8_t>(3, _row_pins);
    row_bus->begin(OUTPUT);
    row_bus->write(0);

    uint8_t i = 4;
    while(i--) {
        pinMode(col_pins[i], OUTPUT);
        digitalWrite(col_pins[i], LOW);
    }
    pinMode(enable_pin, OUTPUT);
    digitalWrite(enable_pin, LOW);

    pinMode(row_8, OUTPUT);
    digitalWrite(row_8, LOW);


    //Make sure that all the leds are off by default
    i = 9;
    while(i--) {
        uint8_t ii = 3;
        while(ii--) {
            leds[i][ii] = false;
        }
    }

}

void matrix_driver::setRow(uint8_t row, const bool set[3]) {
    if(row >= 9)
        return;
    uint8_t i = 3;
    while(i--) {
        leds[row][i] = set[i];
    }
}

void matrix_driver::setSection(uint8_t section, uint8_t value) {
    if(section >= 4)
        return;
    const uint8_t sections_max[4] = {3, 9, 6, 9};
    sections[section] = min(value, sections_max[section]);

    // set the leds
    for(uint8_t i = section_index[section]; i < section_index[section + 1]; i++) {
        bool row[3] = {false, false, false};
        uint8_t count = 0;
        while(count < 3 && count < value) {
            row[count] = true;
            count++;
        }
        value -= count;

        setRow(i, row);
    }
}

void randomize_section(uint8_t sec) {
    const uint8_t sec_size = section_index[sec + 1] - section_index[sec];
    const uint8_t led_size = sec_size * 3;

    // load the pointers of the section for easier processing
    bool *led_pointers[led_size];
    for(uint8_t i = 0; i < sec_size; i++) {
        uint8_t l = 3;
        while(l--) {
            led_pointers[i * 3 + l] = &leds[i + section_index[sec]][l];
            leds[i + section_index[sec]][l] = false;
        }
    }

    // light sections[sec] random leds
    for(uint8_t i = 0; i < sections[sec]; i++) {
        uint8_t index = random(led_size);

        // if the randomly chosen led is already lit, move to the
        // next one until it finds an unlit led
        while(*led_pointers[index]) {
            index++;
            // overflow on the led size
            if(index >= led_size) {
                index = 0;
            }
        }

        *led_pointers[index] = true;
    }
}

void matrix_driver::randomizeLocations() {
    uint8_t i = 4;
    while(i--) {
        randomize_section(i);
    }
}

AsyncDelay dutyCycle(ASYNC_MICROS);
uint8_t delayCache = 0;

void matrix_driver::update() {

    if(!dutyCycle.finished(false)) {
        delayMicroseconds(dutyCycle.timeLeft());
    }

    uint8_t i = 9;
    while(i--) {

        // disable the leds while changing rows.
        uint8_t col = 3;
        while(col--)
            digitalWrite(col_pins[col], LOW);


        // enable the current row
        row_bus->write(i);
        // row_8 is enabled on the value 8
        bool row_8 = i == 8;
        digitalWrite(row_8, row_8);
        // disable the decoder when drawing to row_8
        digitalWrite(enable_pin, !row_8);


        // write the current row's leds.
        col = 3;
        while(col--)
            digitalWrite(col_pins[col], leds[i][col]);
        

        // don't delay on the last loop, allow time for calculations after the function
        if(i)
            delayMicroseconds(FREQUENCY_DELAY);
    }

    if(delayCache != settings::update_time::get()) {
        delayCache = settings::update_time::get();
        dutyCycle.setDelay((255 - delayCache) / 255.0 * DUTY_TIME + FREQUENCY_DELAY);
    }
    dutyCycle.start();
}