/**
 * voice_manager.h
 * 
 * OPL2 Voice Allocation and Management
 * Handles 9-voice polyphonic playback with LRU voice stealing
 */

#ifndef VOICE_MANAGER_H
#define VOICE_MANAGER_H

#include <stdint.h>
#include <stdbool.h>

// --- VOICE STRUCTURE ---
typedef struct {
    bool active;
    uint8_t midi_channel; // Which MIDI channel owns this voice?
    uint8_t midi_note;    // Which note is playing?
    uint32_t age;         // For "Note Stealing" (Simple LRU)
} OPLVoice;

// --- EXTERNAL VOICE ARRAY ---
// External access needed for direct manipulation in audio engine
extern OPLVoice voices[9];

// --- FUNCTION DECLARATIONS ---

/**
 * Initialize all voices to inactive state
 */
void init_voices(void);

/**
 * Find a physical OPL voice to play this MIDI note
 * Handles retrigger, drum allocation, and voice stealing
 * 
 * @param m_ch MIDI channel (0-15)
 * @param m_note MIDI note number (0-127)
 * @return Physical OPL voice index (0-8)
 */
int allocate_voice(uint8_t m_ch, uint8_t m_note);

/**
 * Find which physical voice is playing this note
 * 
 * @param m_ch MIDI channel (0-15)
 * @param m_note MIDI note number (0-127)
 * @return Physical OPL voice index (0-8), or -1 if not found
 */
int find_active_voice(uint8_t m_ch, uint8_t m_note);

/**
 * Apply MIDI velocity to a voice by adjusting carrier TL
 * Converts MIDI velocity (0-127) to OPL attenuation (0-63)
 * 
 * @param channel Physical OPL channel (0-8)
 * @param velocity MIDI velocity (0-127)
 */
void apply_velocity(uint8_t channel, uint8_t velocity);

#endif // VOICE_MANAGER_H
