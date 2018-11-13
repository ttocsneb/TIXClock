#ifndef __BUS_H__
#define __BUS_H__

#include <Arduino.h>

template <typename T>
class Bus {
private:
    uint8_t* pins;
    uint8_t size;
public:
    /**
     * Create a new bus.
     * 
     * @param num_pins The number of pins the bus is made of
     * 
     * @param ... each pin the Bus is made from. (MSB is first)
     */
    Bus(uint8_t num_pins, const uint8_t *_pins) {
        size = num_pins;
        pins = new uint8_t[size];
        memcpy(pins, _pins, sizeof(uint8_t) * size);
    }
    ~Bus() {
        delete[] pins;
    }

    /**
     * Setup the Bus
     */
    bool begin(uint8_t pinmode) {
        uint8_t i = size;
        while(i--) {
            pinMode(pins[i], pinmode);
        }
        return true;
    }

    /**
     * Write a value to the bus
     * 
     * @param data the data to write.
     */
    void write(T data) {
        uint8_t i = size;
        while(i--) {
            digitalWrite(pins[i], (data >> i) & 1);
        }
    }

    /**
     * Read from the bus
     * 
     * @return type T value
     */
    T read() {
        T value = 0;

        uint8_t i = size;
        while(i--) {
            value = value | (digitalRead(pins[i]) << i);
        }

        return value;
    }

};

#endif