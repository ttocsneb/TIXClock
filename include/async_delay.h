#ifndef __ASYNC_DELAY__
#define __ASYNC_DELAY__

enum AsyncDelayType {
    ASYNC_MILLIS=0,
    ASYNC_MICROS=1
};

class AsyncDelay {
private:
    unsigned long start_time;
    unsigned long delay;
    bool enabled;

    AsyncDelayType type;
public:
    AsyncDelay(AsyncDelayType delayType = ASYNC_MILLIS, unsigned long delay = 0, bool enabled = true);

    void setDelay(unsigned long delay);

    void start();
    void start(unsigned long delay);

    void enable(bool enable);
    bool isEnabled();

    bool finished(bool resetOnDone = true);

    unsigned long timeLeft();
};

#endif