/**
 * midi_state.c
 * 
 * MIDI Channel State Management Implementation
 */

#include "midi_state.h"

// Track the current Instrument assigned to each MIDI Channel
static uint8_t midi_ch_program[16] = {0};

void midi_state_init(void) {
    for(int i = 0; i < 16; i++) {
        midi_ch_program[i] = 0;
    }
}

void midi_set_program(uint8_t channel, uint8_t program) {
    if (channel < 16) {
        midi_ch_program[channel] = program;
    }
}

uint8_t midi_get_program(uint8_t channel) {
    if (channel < 16) {
        return midi_ch_program[channel];
    }
    return 0; // Default to program 0 if invalid channel
}
