#include "settings.h"

#include <EEPROM.h>

uint8_t INITIALIZED_ADDR = 0x00;
uint8_t INITIALIZED_VAL = 0xAA;

void settings::begin() {
    EEPROM.begin();

    if(EEPROM.read(INITIALIZED_ADDR) != INITIALIZED_VAL) {
        brightness::set(255);
        update_time::set(15000);
        EEPROM.write(INITIALIZED_ADDR, INITIALIZED_VAL);
    }
}

void write(uint8_t len, uint8_t location, long value) {
    uint8_t byte = len;
    while(byte--) {
        EEPROM.update(location + byte, (value >> byte * 8) & 0xFF);
    }
}

long read(uint8_t len, uint8_t location) {
    long value = 0;

    uint8_t byte = len;
    while(byte--) {
        uint8_t read = EEPROM.read(location + byte);
        value = value | (read << byte * 8);
    }
    return value;
}

// Brightness settings

const uint8_t BRIGHTNESS_ADDR = 0x42;

uint8_t settings::brightness::get() {
    return EEPROM.read(BRIGHTNESS_ADDR);
}

void settings::brightness::set(uint8_t value) {
    EEPROM.update(BRIGHTNESS_ADDR, value);
}

// Update Time Settings

const uint8_t UPDATE_ADDR = 0x55;

uint32_t settings::update_time::get() {
    return static_cast<uint32_t>(read(4, UPDATE_ADDR));
}

void settings::update_time::set(uint32_t value) {
    write(4, UPDATE_ADDR, value);
}
