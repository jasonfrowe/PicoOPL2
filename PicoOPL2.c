#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/util/queue.h"
#include "opl2_hardware.h"
#include "opl2.h"
#include "instruments.h"
#include "voice_manager.h"
#include "midi_state.h"
#include "audio_engine.h"
#include "queue.h"
#include "song_data.h"

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
    audio_engine_init(512);
    audio_engine_start();

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
                audio_engine_add_event(&reset);
                break;
            }
            
            audio_engine_add_event(&e);
        }
        
        printf("Song Done. Restarting in 2s...\n");
        gpio_put(LED_PIN, 0);
        sleep_ms(2000);
        
        // Reload defaults for next loop
        load_drum_patch(8, 36);
    }
}