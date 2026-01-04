/**
 * midi_state.h
 * 
 * MIDI Channel State Management
 * Tracks program (instrument) assignments for all 16 MIDI channels
 */

#ifndef MIDI_STATE_H
#define MIDI_STATE_H

#include <stdint.h>

/**
 * Initialize all MIDI channels to program 0 (default)
 */
void midi_state_init(void);

/**
 * Set the program (instrument) for a MIDI channel
 * 
 * @param channel MIDI channel (0-15)
 * @param program GM program number (0-127)
 */
void midi_set_program(uint8_t channel, uint8_t program);

/**
 * Get the current program (instrument) for a MIDI channel
 * 
 * @param channel MIDI channel (0-15)
 * @return GM program number (0-127)
 */
uint8_t midi_get_program(uint8_t channel);

#endif // MIDI_STATE_H
