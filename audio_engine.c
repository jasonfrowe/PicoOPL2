/**
 * audio_engine.c
 * 
 * Multi-Core Audio Engine Implementation
 * Processes MIDI events and controls OPL2 synthesis
 */

#include "audio_engine.h"
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/util/queue.h"
#include "opl2.h"
#include "instruments.h"
#include "voice_manager.h"
#include "midi_state.h"

// Event queue for communication between cores
static queue_t event_queue;

// --- CORE 1: THE AUDIO ENGINE ---
static void core1_entry(void) {
    SongEvent event;
    init_voices();
    
    while (true) {
        queue_remove_blocking(&event_queue, &event);
            
        if (event.delay_ms > 0) sleep_ms(event.delay_ms);

        switch (event.type) {
            case 0: // Note Off
            {
                int voice = find_active_voice(event.channel, event.note);
                if (voice != -1) {
                    opl2_note_off(voice);
                    voices[voice].active = false;
                }
                break;
            }

            case 1: // Note On
            {
                // 1. Allocate Voice
                int voice = allocate_voice(event.channel, event.note);
                
                // 2. Load Instrument
                if (event.channel == 9) { // MIDI DRUMS
                    load_drum_patch(voice, event.note);
                } 
                else {
                    // MELODIC
                    uint8_t prog = midi_get_program(event.channel);
                    load_gm_instrument(voice, prog);
                }

                // 3. Play - Use actual MIDI velocity now that patches have proper headroom
                apply_velocity(voice, event.velocity);
                opl2_note_on(voice, event.note);
                break;
            }

            case 3: // Program Change
                midi_set_program(event.channel, event.note);
                break;
                
            case 2: // Reset
                for(int i=0; i<9; i++) opl2_note_off(i);
                init_voices();
                break;
        }
    }
}

// --- PUBLIC INTERFACE ---

void audio_engine_init(uint16_t queue_size) {
    queue_init(&event_queue, sizeof(SongEvent), queue_size);
}

void audio_engine_start(void) {
    multicore_launch_core1(core1_entry);
}

void audio_engine_add_event(const SongEvent *event) {
    queue_add_blocking(&event_queue, event);
}
