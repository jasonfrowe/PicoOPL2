/**
 * menu.h
 * 
 * LCD Menu System for PicoOPL2
 * Handles UI display and user interaction
 */

#ifndef MENU_H
#define MENU_H

#include <stdint.h>
#include <stdbool.h>

// Operating modes
typedef enum {
    MODE_SONG,      // Internal song playback
    MODE_MIDI_IN    // External MIDI input (future)
} menu_mode_t;

/**
 * Initialize the menu system
 */
void menu_init(void);

/**
 * Update menu display and handle encoder input
 * Call this regularly from main loop
 */
void menu_update(void);

/**
 * Set current operating mode
 * 
 * @param mode New mode to set
 */
void menu_set_mode(menu_mode_t mode);

/**
 * Get current operating mode
 * 
 * @return Current mode
 */
menu_mode_t menu_get_mode(void);

/**
 * Update voice activity indicators
 * 
 * @param voice_active Array of 9 bools indicating active voices
 */
void menu_update_voices(const bool voice_active[9]);

/**
 * Set song name for display
 * 
 * @param name Song name (max 20 chars)
 */
void menu_set_song_name(const char *name);

#endif // MENU_H
