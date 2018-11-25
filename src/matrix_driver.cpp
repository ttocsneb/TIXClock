#include "matrix_driver.h"

// Calculate the delay(us) required to get the propper frequency
const unsigned long FREQUENCY_DELAY = 1000000 / (FREQUENCY * 9)  / 3;

const unsigned int DUTY_TIME = 1000000 / (DUTY_FREQUENCY);

#include <Arduino.h>

#include <avr/interrupt.h>
#include <avr/io.h>

#include "bus.h"
#include "async_delay.h"
#include "settings.h"


bool outputs[27];

Bus<uint8_t> *row_bus = NULL;
uint8_t col_pins[3];
uint8_t enable_pin;

bool leds[9][3];

uint8_t sections[4];
const uint8_t section_index[5] = {0, 1, 4, 6, 9};


void matrix_driver::begin(const uint8_t _row_pins[4], const uint8_t _col_pins[3], uint8_t _enable_pin) {
    // set the row pins
    memcpy(col_pins, _col_pins, sizeof(uint8_t) * 3);
    enable_pin = _enable_pin;

    // Setup all the pins to be outputs
    if(row_bus) {
        // prevent memory leaks from multiple begin calles
        delete row_bus;
        row_bus = NULL;
    }

    row_bus = new Bus<uint8_t>(4,_row_pins);
    row_bus->begin(OUTPUT);
    row_bus->write(0);

    uint8_t i = 4;
    while(i--) {
        pinMode(col_pins[i], OUTPUT);
        digitalWrite(col_pins[i], LOW);
    }
    pinMode(enable_pin, OUTPUT);
    digitalWrite(enable_pin, LOW);


    //Make sure that all the leds are off by default
    i = 9;
    while(i--) {
        uint8_t ii = 3;
        while(ii--) {
            leds[i][ii] = false;
        }
    }

    // Setup Timers
    
    // temporarily disable interrupts
    SREG &= ~(1 << 7);

    // disable any external pins on the timer
    TCCR1A &= ~(0b1111 << 4); 

    // Set the WGM bits
    TCCR1A &= ~0b11; // disable WGM11 and WGM10
    TCCR1B |= 0b11 << 3; // enable WGM13 and WGM12

    // Set the Timer Clock to be 0b011 (64x prescaler ie. 1 count every 4us)
    TCCR1C |= 0b11; // enable CS11 and CS10
    TCCR1C &= ~(1 << 2); // disable CS12

    // Set the Timer compare value
    ICR1 = FREQUENCY_DELAY / 4;


    // Enable the Interrupt register
    TIMSK1 |= 1 << 1;

    // reset the timer
    TCNT1 = 0;

    // enable interrupts again
    SREG |= 1 << 7;
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
            // clear the section.
            *led_pointers[i * 3 + l] = false;
        }
    }

    // light sections[sec] random leds
    for(uint8_t i = 0; i < sections[sec]; i++) {
        uint8_t index = random(led_size);

        // if the randomly chosen led is already lit, move to the
        // next one until it finds an unlit led
        uint8_t count = 0;
        while(*led_pointers[index] == true) {
            // overflow on the led size
            index++;
            if(index >= led_size) {
                index = 0;
            }
            //prevent infinite loops
            if(count > led_size) {
                break;
            }
            count++;
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

ISR(TIMER1_COMPA_vect) {
    static uint8_t row(0);
    static uint8_t col(0);


    digitalWrite(col_pins[col], LOW);

    // update the current led
    if(!col--) {
        col = 2;
        if(!row--) {
            row = 8;
        }

        // disable the decoder when drawing to row_8
        digitalWrite(enable_pin, row != 8);

        // enable the current row
        row_bus->write(row);
    }

    digitalWrite(col_pins[col], leds[row][col]);
}