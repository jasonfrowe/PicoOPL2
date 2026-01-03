#include "pico/stdlib.h"

// Pin Definitions based on our wiring
#define OPL2_DATA_MASK 0x000000FF // GPIO 0-7
#define OPL2_A0        8
#define OPL2_WR        9
#define OPL2_CS        10
#define OPL2_IC        11

// Timing constants (slightly conservative for clones)
#define OPL2_WAIT_ADDRESS 4  // microseconds
#define OPL2_WAIT_DATA    23 // microseconds

extern void opl2_hw_init();
extern void opl2_write(uint8_t reg, uint8_t data);
extern void start_opl2_clock();
extern void hardware_setup();