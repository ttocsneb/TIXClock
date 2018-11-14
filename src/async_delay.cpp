#include "async_delay.h"

#include <Arduino.h>

AsyncDelay::AsyncDelay(AsyncDelayType delayType, unsigned long delay) {
    type = delayType;
    start_time = 0;
    this->delay = delay;
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
    enabled = true;
}

void AsyncDelay::start(unsigned long delay) {
    start();
    setDelay(delay);
}

void AsyncDelay::enable(bool enable) {
    enabled = enable;
}

bool AsyncDelay::isEnabled() {
    return enabled;
}

bool AsyncDelay::finished(bool resetOnDone) {
    if(!enabled) {
        return false;
    }

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