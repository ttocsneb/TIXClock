#include "async_delay.h"

#include <Arduino.h>

AsyncDelay::AsyncDelay(AsyncDelayType delayType) {
    type = delayType;
    start_time = 0;
}

void AsyncDelay::setDelay(unsigned long delay) {
    this->delay = delay;
}

unsigned long get_time(AsyncDelayType type) {
    switch(type) {
    case ASYNC_MICROS:
        return micros();
    case ASYNC_MILLIS:
    default:
        return millis();
    }
}

void AsyncDelay::start() {
    start_time = get_time(type);
}

void AsyncDelay::start(unsigned long delay) {
    start();
    setDelay(delay);
}

bool AsyncDelay::finished(bool resetOnDone) {
    if(get_time(type) - start_time > delay) {
        if(resetOnDone) {
            start();
        }
        return true;
    }
    return false;
}

unsigned long AsyncDelay::timeLeft() {
    return delay - (get_time(type) - start_time);
}