/**
 * song_player.h
 * 
 * Internal Song Player
 * Plays embedded MIDI song data for testing/demonstration
 */

#ifndef SONG_PLAYER_H
#define SONG_PLAYER_H

#include "pico/types.h"
#include <stdbool.h>

/**
 * Initialize song player
 */
void song_player_init(void);

/**
 * Update song player state (non-blocking)
 * Call this regularly from main loop
 * 
 * @param led_pin GPIO pin for LED blinking during playback
 */
void song_player_update(uint led_pin);

/**
 * Check if song is currently playing
 * 
 * @return true if playing, false if paused/stopped
 */
bool song_player_is_playing(void);

/**
 * Start/resume playback
 */
void song_player_play(void);

/**
 * Pause playback
 */
void song_player_pause(void);

/**
 * Skip to next song (restart for now)
 */
void song_player_skip(void);

#endif // SONG_PLAYER_H
