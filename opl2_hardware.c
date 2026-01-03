#include "opl2_hardware.h"
#include "hardware/clocks.h"
#include "hardware/pwm.h"
#include "opl2.h"

void opl2_hw_init() {
    // Initialize Data Pins 0-7
    gpio_init_mask(OPL2_DATA_MASK);
    gpio_set_dir_out_masked(OPL2_DATA_MASK);

    // Initialize Control Pins
    uint32_t control_mask = (1 << OPL2_A0) | (1 << OPL2_WR) | (1 << OPL2_CS) | (1 << OPL2_IC);
    gpio_init_mask(control_mask);
    gpio_set_dir_out_masked(control_mask);

    // Set default idle state: CS and WR High, IC High
    gpio_put(OPL2_CS, 1);
    gpio_put(OPL2_WR, 1);
    gpio_put(OPL2_IC, 1);

    // Hard Reset Sequence
    gpio_put(OPL2_IC, 0);
    sleep_ms(10);
    gpio_put(OPL2_IC, 1);
    sleep_ms(10);
}

void start_opl2_clock() {
    // We want 3.579545 MHz. 
    // The Pico system clock is usually 125MHz.
    // We can use the PWM slice to create a precise clock divider.
    uint gpio = 21; // Using GP21 (Pin 27)
    gpio_set_function(gpio, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(gpio);

    // Calculate divider for ~3.579 MHz
    // 125,000,000 / 3,579,545 = 34.92
    // We'll set the wrap to 34 and use the clock divider for precision
    pwm_set_clkdiv(slice_num, 1.0f); 
    pwm_set_wrap(slice_num, 34); // 125MHz / 35 = 3.571 MHz (Close enough for OPL2!)
    pwm_set_chan_level(slice_num, pwm_gpio_to_channel(gpio), 17); // 50% duty cycle
    pwm_set_enabled(slice_num, true);
}

// Setup function to initialize hardware
void hardware_setup() {
    stdio_init_all();
    
    // 1. Start the 3.57MHz Clock for the YM3812
    start_opl2_clock(); 
    
    // 2. Initialize the GPIO pins (Data 0-7, A0, WR, CS, IC)
    opl2_hw_init(); 
    
    // 3. Optional: Give the chip a moment to stabilize
    sleep_ms(100);

    // 4. Clear all registers (optional but recommended)
    for(int i = 0; i < 255; i++) opl2_write(i, 0x00);

    // 5. ENABLE WAVEFORM SELECT (The "Secret Sauce")
    // Register 0x01, Bit 5 must be 1 to allow non-sine waveforms.
    // Without this, OPL2 ignores waveform settings in your MIDI patches.
    opl2_write(0x01, 0x20); 

    // 6. INITIALIZE RHYTHM MODE (Optional)
    // Register 0xBD controls the percussion. Zeroing it (done above) 
    // puts it in "6-melody, 5-rhythm" or "9-melody" mode.
    opl2_write(0xBD, 0x00); 

    // printf("YM3812 Hardware Initialized and Waveforms Unlocked.\n");
}