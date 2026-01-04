/**
 * song_player.c
 * 
 * Internal Song Player Implementation
 * Feeds song data from song_data.h to the audio engine
 */

#include "song_player.h"
#include "pico/stdlib.h"
#include "audio_engine.h"
#include "instruments.h"
#include "queue.h"
#include "song_data.h"
#include <stdio.h>

void song_player_loop(uint led_pin) {
    // --- INTERNAL SONG PLAYER ---
    while(true) {
        printf("Core 0: Feeding Queue...\n");
        gpio_put(led_pin, 1);
        
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
        gpio_put(led_pin, 0);
        sleep_ms(2000);
        
        // Reload defaults for next loop
        load_drum_patch(8, 36);
    }
}
