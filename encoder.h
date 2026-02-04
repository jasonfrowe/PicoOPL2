/**
 * encoder.h
 * 
 * Rotary Encoder (PEC11R) Interface
 * Handles rotation detection and button press/long-press
 */

#ifndef ENCODER_H
#define ENCODER_H

#include <stdint.h>
#include <stdbool.h>

// Encoder pins
#define ENCODER_PIN_A  14
#define ENCODER_PIN_B  15
#define ENCODER_SW     16

// Button events
typedef enum {
    ENCODER_BTN_NONE,
    ENCODER_BTN_PRESS,      // Short press
    ENCODER_BTN_LONG_PRESS  // Long press (>500ms)
} encoder_button_event_t;

/**
 * Initialize the rotary encoder
 * Sets up GPIO pins and interrupts
 */
void encoder_init(void);

/**
 * Get rotation delta since last call
 * Positive = clockwise, negative = counter-clockwise
 * 
 * @return Change in encoder position
 */
int encoder_get_delta(void);

/**
 * Get button event
 * Detects short press and long press
 * 
 * @return Button event type
 */
encoder_button_event_t encoder_get_button(void);

#endif // ENCODER_H
