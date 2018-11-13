#ifndef __SETTINGS_H__
#define __SETTINGS_H__

#include <stdint.h>

namespace settings {

    namespace brightness {
        uint8_t get();
        void set(uint8_t value);
    };

    namespace update_time {
        uint32_t get();
        void set(uint32_t value);
    }

    void begin();
};

#endif