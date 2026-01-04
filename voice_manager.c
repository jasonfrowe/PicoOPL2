/**
 * voice_manager.c
 * 
 * OPL2 Voice Allocation and Management Implementation
 */

#include "voice_manager.h"
#include "opl2.h"
#include "instruments.h"

// --- VOICE STATE ---
OPLVoice voices[9];       // The 9 Physical OPL Channels
uint32_t note_counter = 0; // Global clock for age tracking

// --- IMPLEMENTATION ---

void init_voices(void) {
    for(int i=0; i<9; i++) {
        voices[i].active = false;
        voices[i].midi_channel = 255;
        voices[i].midi_note = 0;
        voices[i].age = 0;
    }
}

int allocate_voice(uint8_t m_ch, uint8_t m_note) {
    // 1. Check for Retrigger (Same note, same channel)
    for(int i=0; i<9; i++) {
        if (voices[i].active && voices[i].midi_channel == m_ch && voices[i].midi_note == m_note) {
            voices[i].age = ++note_counter;
            return i;
        }
    }

    // 2. DRUM HANDLING (MIDI Ch 9 -> Physical Voice 8)
    if (m_ch == 9) {
        voices[8].active = true;
        voices[8].midi_channel = 9;
        voices[8].midi_note = m_note;
        return 8;
    }

    // 3. MELODIC HANDLING (Find free voice 0-7)
    for(int i=0; i<8; i++) {
        if (!voices[i].active) {
            voices[i].active = true;
            voices[i].midi_channel = m_ch;
            voices[i].midi_note = m_note;
            voices[i].age = ++note_counter;
            return i;
        }
    }

    // 4. STEAL OLDEST (Voices 0-7 only)
    int oldest_idx = 0;
    uint32_t min_age = 0xFFFFFFFF;
    for(int i=0; i<8; i++) {
        if (voices[i].age < min_age) {
            min_age = voices[i].age;
            oldest_idx = i;
        }
    }
    
    voices[oldest_idx].midi_channel = m_ch;
    voices[oldest_idx].midi_note = m_note;
    voices[oldest_idx].age = ++note_counter;
    return oldest_idx;
}

int find_active_voice(uint8_t m_ch, uint8_t m_note) {
    if (m_ch == 9) return 8; // Drums always Ch 8

    for(int i=0; i<8; i++) {
        if (voices[i].active && voices[i].midi_channel == m_ch && voices[i].midi_note == m_note) {
            return i;
        }
    }
    return -1; // Not found (maybe already stolen/stopped)
}

void apply_velocity(uint8_t channel, uint8_t velocity) {
    if (channel > 8) return;
    
    // Get the original patch carrier KSL/TL
    uint8_t base_ksl = shadow_carrier_ksl[channel];
    uint8_t base_tl  = base_ksl & 0x3F;      // Original TL from patch
    uint8_t ksl_bits = base_ksl & 0xC0;      // KSL bits
    
    // Convert MIDI velocity (0-127) to OPL attenuation (0-63)
    // Higher velocity = less attenuation (louder)
    // Lower velocity = more attenuation (quieter)
    uint8_t velocity_attenuation = (127 - velocity) >> 1;
    
    // Combine: base TL + velocity attenuation
    // This gives us dynamic range based on MIDI velocity
    uint8_t final_tl = base_tl + velocity_attenuation;
    
    // Clamp to valid OPL range
    if (final_tl > 63) final_tl = 63;
    
    // Write to carrier TL register
    uint8_t offsets[9] = {0, 1, 2, 8, 9, 10, 16, 17, 18};
    opl2_write(0x43 + offsets[channel], ksl_bits | final_tl);
}
