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
    Bus(uint8_t num_pins, ...) {
        size = num_pins;
        pins = new uint8_t[size];
        
        va_list ap;
        va_start(ap, num_pins);
        for(uint8_t i = size - 1; i >= 0; i--) {
            pins[i] = static_cast<uint8_t>(va_arg(ap, int));
        }
    }
    ~Bus() {
        delete[] pins;
    }

    /**
     * Setup the Bus
     */
    bool begin(uint8_t pinmode) {
        for(uint8_t i = 0; i < size; i++) {
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
        for(uint8_t i = 0; i < size; i++) {
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

        for (uint8_t i=0; i < size; i++) {
            value = value | (digitalRead(pins[i]) << i);
        }

        return value;
    }

};

#endif