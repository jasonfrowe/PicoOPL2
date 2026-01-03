#ifndef OPL_H
#define OPL_H

#include <stdint.h>

// Core OPL2 Functions
extern void opl2_write(uint8_t reg, uint8_t data);
extern void opl2_note_on(uint8_t channel, uint8_t midi_note);
extern void opl2_note_off(uint8_t channel);
extern uint16_t midi_to_opl2_freq(uint8_t midi_note);
extern void opl2_clear();

// Shadow variables
extern uint8_t shadow_b0[9];

#endif // OPL_H