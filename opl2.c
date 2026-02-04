#include <stdio.h>
#include "pico/stdlib.h"
#include "opl2.h"
#include "opl2_hardware.h"

// ==========================================================
// OPL2 FREQUENCY MATH
// ==========================================================
// Standard OPL2 runs at 3.58 MHz. We use these F-Numbers directly
// since we're driving a real Yamaha OPL2 chip.

const uint16_t fnum_table[12] = {
    344, 363, 385, 408, 432, 458, 485, 514, 544, 577, 611, 647
};

// Shadow registers for all 9 channels
// We need this to remember the Block/F-Number when we send a NoteOff
uint8_t shadow_b0[9] = {0};

void opl2_write(uint8_t reg, uint8_t data) {
    // 1. SELECT REGISTER
    gpio_put(OPL2_A0, 0);
    gpio_put_masked(OPL2_DATA_MASK, reg);
    gpio_put(OPL2_CS, 0); 
    gpio_put(OPL2_WR, 0);
    
    sleep_us(1); // Increased from NOPs to 1 microsecond for reliability
    
    gpio_put(OPL2_WR, 1); 
    gpio_put(OPL2_CS, 1);
    sleep_us(OPL2_WAIT_ADDRESS);

    // 2. WRITE DATA
    gpio_put(OPL2_A0, 1);
    gpio_put_masked(OPL2_DATA_MASK, data);
    gpio_put(OPL2_CS, 0); 
    gpio_put(OPL2_WR, 0);
    
    sleep_us(1); // Increased for reliability
    
    gpio_put(OPL2_WR, 1); 
    gpio_put(OPL2_CS, 1);
    sleep_us(OPL2_WAIT_DATA);
}

uint16_t midi_to_opl2_freq(uint8_t midi_note) {
    // Clamp to lowest valid note (C0) to prevent negative math
    if (midi_note < 12) midi_note = 12;
    
    int block = (midi_note - 12) / 12;
    int note_idx = (midi_note - 12) % 12;
    
    // OPL2 only supports Blocks 0-7
    if (block > 7) block = 7;

    uint16_t f_num = fnum_table[note_idx];

    // Pack: KeyOn (0x20) | Block | F-Num High
    uint8_t high_byte = 0x20 | (block << 2) | ((f_num >> 8) & 0x03);
    uint8_t low_byte = f_num & 0xFF;

    return (high_byte << 8) | low_byte;
}

void opl2_note_on(uint8_t channel, uint8_t midi_note) {
    if (channel > 8) return; // Safety

    // 1. Calculate params using the helper
    uint16_t freq_data = midi_to_opl2_freq(midi_note);
    uint8_t high_byte = (freq_data >> 8) & 0xFF; // Includes 0x20 (KeyOn)
    uint8_t low_byte  = freq_data & 0xFF;

    // 2. Write to OPL2
    opl2_write(0xA0 + channel, low_byte);
    opl2_write(0xB0 + channel, high_byte);

    // 3. Update Shadow (Exclude KeyOn bit for safe storage)
    shadow_b0[channel] = high_byte & ~0x20;
}

void opl2_note_off(uint8_t channel) {
    if (channel > 8) return;

    // Retrieve the pitch for this channel, but keep KeyOn (0x20) CLEARED
    uint8_t safe_release_byte = shadow_b0[channel];
    opl2_write(0xB0 + channel, safe_release_byte);
}

void opl2_clear() {
    for (int i = 0; i < 256; i++) {
        opl2_write(i, 0x00);
    }
    // Clear shadow memory too
    for (int i = 0; i < 9; i++) shadow_b0[i] = 0;
}

void opl2_silence_all() {
    // Turn off all 9 voices by clearing the KeyOn bit (0x20)
    // but preserve the pitch information in shadow registers
    for (int i = 0; i < 9; i++) {
        // Write the shadow value which has KeyOn cleared
        opl2_write(0xB0 + i, shadow_b0[i] & ~0x20);
        // Don't clear shadow_b0 - keep the pitch data
    }
}