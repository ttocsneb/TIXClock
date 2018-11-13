#ifndef __MATRIX_DRIVER_H__
#define __MATRIX_DRIVER_H__

#include <stdint.h>

#define FREQUENCY 1000

namespace matrix_driver {
    void begin(const uint8_t row_pins[4], const uint8_t col_pins[3], uint8_t enable_pin);

    /**
     * Directly set the values of a specific row.
     * 
     * @warning  randomizeLocations() will override this.
     * If you want to have randomized leds and start out with specific
     * leds, call setSection() first, then setRow(), but this will set
     * an entire location.
     * 
     * @param row (0-8) row to set
     * 
     * @param leds led values for that row.
     */
    void setRow(uint8_t row, const bool leds[3]);

    /**
     * Set the number of lit leds on a section.
     * 
     * @note the locations are not randomized until randomizeLocations()
     * is called.  If randomizeLocations() is not called, the lit leds will
     * count from the top right.
     * 
     * @param section (0-3) Each section has a different number of leds:
     * 0: 3
     * 1: 9
     * 2: 6
     * 3: 9
     * 
     * @param value The number of leds to light.
     */
    void setSection(uint8_t section, uint8_t value);

    /**
     * Randomize the locations of the leds.
     */
    void randomizeLocations();

    /**
     * Actually display the leds.
     */
    void update();
}

#endif