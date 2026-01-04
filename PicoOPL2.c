#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/util/queue.h"
#include "opl2_hardware.h"
#include "opl2.h"
#include "instruments.h"
#include "voice_manager.h"
#include "queue.h"
#include "song_data.h"

// Track the current Instrument assigned to each MIDI Channel
uint8_t midi_ch_program[16] = {0};

queue_t event_queue;

// --- CORE 1: THE AUDIO ENGINE ---
void core1_entry() {
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
                    uint8_t prog = midi_ch_program[event.channel];
                    load_gm_instrument(voice, prog);
                }

                // 3. Play - Use actual MIDI velocity now that patches have proper headroom
                apply_velocity(voice, event.velocity);
                opl2_note_on(voice, event.note);
                break;
            }

            case 3: // Program Change
                if (event.channel < 16) {
                    midi_ch_program[event.channel] = event.note;
                }
                break;
                
            case 2: // Reset
                for(int i=0; i<9; i++) opl2_note_off(i);
                init_voices();
                break;
        }
    }
}

// --- MAIN ---
int main() {
    stdio_init_all();
    hardware_setup();
    
    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    // Init OPL2
    printf("Booting PicoOPL2 Synthesizer...\n");
    sleep_ms(2000);
    
    opl2_clear();
    opl2_write(0x01, 0x20); // Enable Waveform Select
    opl2_write(0xBD, 0x00); // Ensure Melodic Mode

    // Load default instruments
    for(int i=0; i<9; i++) load_gm_instrument(i, 0);
    load_drum_patch(8, 36);

    // Start Engine
    queue_init(&event_queue, sizeof(SongEvent), 512);
    multicore_launch_core1(core1_entry);

    // --- INTERNAL SONG PLAYER ---
    while(true) {
        printf("Core 0: Feeding Queue...\n");
        gpio_put(LED_PIN, 1);
        
        int i = 0;
        while(true) {
            SongEvent e = midi_song[i++];
            
            if (e.type == 2) {
                // End of song marker - Send Reset
                SongEvent reset = { .type=2, .delay_ms=0 };
                queue_add_blocking(&event_queue, &reset);
                break;
            }
            
            queue_add_blocking(&event_queue, &e);
        }
        
        printf("Song Done. Restarting in 2s...\n");
        gpio_put(LED_PIN, 0);
        sleep_ms(2000);
        
        // Reload defaults for next loop
        load_drum_patch(8, 36);
    }
}