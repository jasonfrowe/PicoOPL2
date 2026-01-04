/**
 * song_player.h
 * 
 * Internal Song Player
 * Plays embedded MIDI song data for testing/demonstration
 */

#ifndef SONG_PLAYER_H
#define SONG_PLAYER_H

#include "pico/types.h"

/**
 * Start playing the internal song in a loop
 * This function runs forever - it contains the main loop
 * 
 * @param led_pin GPIO pin for LED blinking during playback
 */
void song_player_loop(uint led_pin);

#endif // SONG_PLAYER_H
