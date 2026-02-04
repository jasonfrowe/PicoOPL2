/**
 * encoder.c
 * 
 * Rotary Encoder Implementation
 * Uses interrupt-based quadrature decoding
 */

#include "encoder.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"

// Encoder state
static volatile int32_t encoder_position = 0;
static volatile uint32_t button_press_time = 0;
static volatile bool button_pressed = false;
static volatile bool button_handled = false;
static volatile bool ignore_rotation = false;

// Quadrature lookup table for reliable decoding
static const int8_t encoder_table[16] = {
    0,  // 0000
    -1, // 0001
    1,  // 0010
    0,  // 0011
    1,  // 0100
    0,  // 0101
    0,  // 0110
    -1, // 0111
    -1, // 1000
    0,  // 1001
    0,  // 1010
    1,  // 1011
    0,  // 1100
    1,  // 1101
    -1, // 1110
    0   // 1111
};

static uint8_t encoder_state = 0;

// GPIO interrupt callback
static void encoder_gpio_callback(uint gpio, uint32_t events) {
    if (gpio == ENCODER_PIN_A || gpio == ENCODER_PIN_B) {
        if (ignore_rotation) return;
        // Read current encoder state
        encoder_state = (encoder_state << 2) & 0x0F;
        if (gpio_get(ENCODER_PIN_A)) encoder_state |= 0x02;
        if (gpio_get(ENCODER_PIN_B)) encoder_state |= 0x01;
        
        // Update position based on lookup table
        encoder_position += encoder_table[encoder_state];
    }
    else if (gpio == ENCODER_SW) {
        if (events & GPIO_IRQ_EDGE_FALL) {
            // Button pressed
            button_press_time = to_ms_since_boot(get_absolute_time());
            button_pressed = true;
            button_handled = false;
            // Ignore any rotation while the push button is held down
            ignore_rotation = true;
            // Disable A/B interrupts to prevent spurious counts during button press
            gpio_set_irq_enabled(ENCODER_PIN_A, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, false);
            gpio_set_irq_enabled(ENCODER_PIN_B, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, false);
            encoder_position = 0;
        }
        else if (events & GPIO_IRQ_EDGE_RISE) {
            // Button released
            button_pressed = false;
            ignore_rotation = false;
            // Reset encoder state to current pin levels and re-enable interrupts
            encoder_state = 0;
            if (gpio_get(ENCODER_PIN_A)) encoder_state |= 0x02;
            if (gpio_get(ENCODER_PIN_B)) encoder_state |= 0x01;
            gpio_set_irq_enabled(ENCODER_PIN_A, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
            gpio_set_irq_enabled(ENCODER_PIN_B, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
        }
    }
}

void encoder_init(void) {
    // Initialize encoder pins A and B
    gpio_init(ENCODER_PIN_A);
    gpio_init(ENCODER_PIN_B);
    gpio_set_dir(ENCODER_PIN_A, GPIO_IN);
    gpio_set_dir(ENCODER_PIN_B, GPIO_IN);
    gpio_pull_up(ENCODER_PIN_A);
    gpio_pull_up(ENCODER_PIN_B);
    
    // Initialize button pin
    gpio_init(ENCODER_SW);
    gpio_set_dir(ENCODER_SW, GPIO_IN);
    gpio_pull_up(ENCODER_SW);
    
    // Read initial state
    if (gpio_get(ENCODER_PIN_A)) encoder_state |= 0x02;
    if (gpio_get(ENCODER_PIN_B)) encoder_state |= 0x01;
    
    // Enable interrupts
    gpio_set_irq_enabled_with_callback(ENCODER_PIN_A, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &encoder_gpio_callback);
    gpio_set_irq_enabled(ENCODER_PIN_B, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
    gpio_set_irq_enabled(ENCODER_SW, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
}

int encoder_get_delta(void) {
    // Don't return rotation while button is pressed
    if (button_pressed) {
        encoder_position = 0;
        return 0;
    }
    
    // Read and reset position (divide by 4 for detents)
    int delta = encoder_position / 4;
    encoder_position = encoder_position % 4;
    return delta;
}

encoder_button_event_t encoder_get_button(void) {
    if (!button_pressed && button_handled) {
        button_handled = false;
        button_press_time = 0;  // Reset after event is consumed
        return ENCODER_BTN_NONE;
    }
    
    if (button_pressed && !button_handled) {
        uint32_t press_duration = to_ms_since_boot(get_absolute_time()) - button_press_time;
        
        // Long press detection (>500ms while still pressed)
        if (press_duration > 500) {
            button_handled = true;
            return ENCODER_BTN_LONG_PRESS;
        }
    }
    
    // Short press detection (released before 500ms)
    if (!button_pressed && !button_handled && button_press_time > 0) {
        uint32_t press_duration = to_ms_since_boot(get_absolute_time()) - button_press_time;
        if (press_duration < 500) {
            button_handled = true;
            return ENCODER_BTN_PRESS;
        }
        // If too much time has passed, reset the press time
        button_press_time = 0;
    }
    
    return ENCODER_BTN_NONE;
}
