/**
 * midi_input.h
 * 
 * MIDI Input Handler
 * Receives MIDI data via UART and sends events to audio engine
 */

#ifndef MIDI_INPUT_H
#define MIDI_INPUT_H

#include <stdint.h>
#include <stdbool.h>

/**
 * Initialize MIDI input on GPIO-17 (UART0 RX)
 * Sets up 31250 baud, 8N1
 */
void midi_input_init(void);

/**
 * Update MIDI input - process incoming bytes
 * Call this regularly from main loop when in MIDI-IN mode
 */
void midi_input_update(void);

/**
 * Enable/disable MIDI input processing
 * 
 * @param enabled true to enable, false to disable
 */
void midi_input_set_enabled(bool enabled);

#endif // MIDI_INPUT_H
