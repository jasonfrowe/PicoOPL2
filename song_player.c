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
#include "opl2.h"
#include <stdio.h>

// Player state
static bool playing = false;
static uint32_t song_index = 0;
static uint32_t song_restart_time = 0;
static bool waiting_to_restart = false;
static uint32_t next_event_time_ms = 0;

void song_player_init(void) {
    playing = false;
    song_index = 0;
    waiting_to_restart = false;
    next_event_time_ms = 0;
}

void song_player_update(uint led_pin) {
    if (!playing) {
        gpio_put(led_pin, 0);
        return;
    }
    
    // Handle restart delay
    if (waiting_to_restart) {
        if (to_ms_since_boot(get_absolute_time()) >= song_restart_time) {
            printf("Song restarting...\n");
            song_index = 0;
            waiting_to_restart = false;
            load_drum_patch(8, 36);  // Reload defaults
            next_event_time_ms = to_ms_since_boot(get_absolute_time());
        }
        return;
    }
    
    // Feed events based on their scheduled times (prevents slow-motion playback)
    gpio_put(led_pin, 1);
    uint32_t now_ms = to_ms_since_boot(get_absolute_time());
    size_t song_len = sizeof(midi_song) / sizeof(midi_song[0]);

    while (song_index < song_len && now_ms >= next_event_time_ms) {
        SongEvent e = midi_song[song_index++];

        if (e.type == 2) {
            // End of song marker - schedule restart
            printf("Song done. Restarting in 2s...\n");
            SongEvent reset = { .type=2, .delay_ms=0 };
            audio_engine_add_event(&reset);
            waiting_to_restart = true;
            song_restart_time = now_ms + 2000;
            gpio_put(led_pin, 0);
            return;
        }

        audio_engine_add_event(&e);
        next_event_time_ms += e.delay_ms;

        // Update now_ms to allow multiple zero-delay events in this call
        now_ms = to_ms_since_boot(get_absolute_time());
    }
}

bool song_player_is_playing(void) {
    return playing;
}

void song_player_play(void) {
    if (!playing) {
        printf("Song player: Play\n");
        playing = true;
        next_event_time_ms = to_ms_since_boot(get_absolute_time());
    }
}

void song_player_pause(void) {
    if (playing) {
        printf("Song player: Pause\n");
        playing = false;
        audio_engine_flush();  // Clear queued events
        sleep_ms(10);  // Give audio engine time to finish current event
        
        // Send a reset event to clear voice states and silence all notes
        SongEvent reset = { .type = 2, .delay_ms = 0 };
        audio_engine_add_event(&reset);
        sleep_ms(10);  // Give time for reset to process
    }
}

void song_player_skip(void) {
    printf("Song player: Skip (restart)\n");
    song_index = 0;
    waiting_to_restart = false;
    next_event_time_ms = to_ms_since_boot(get_absolute_time());
    load_drum_patch(8, 36);
}
