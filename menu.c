/**
 * menu.c
 * 
 * LCD Menu System Implementation
 */

#include "menu.h"
#include "lcd.h"
#include "encoder.h"
#include "instruments.h"
#include "song_player.h"
#include "midi_input.h"
#include "midi_state.h"
#include "pico/stdlib.h"
#include <stdio.h>
#include <string.h>

// Menu state
static menu_mode_t current_mode = MODE_MIDI_IN;
static uint8_t selected_channel = 0;  // 0-15 for MIDI channels
static uint8_t selected_program = 0;  // 0-255 for patches
static uint8_t volume = 127;
static int8_t octave = 0;
static bool voice_states[9] = {false};
static char song_name[21] = "Doom E1M1";
static uint32_t last_update = 0;
static bool menu_dirty = true;
static bool patch_edit_mode = false;
static bool channel_edit_mode = false;

#define CHANNEL_ALL 255  // Special value for "all channels"

// Cursor position (which line is selected)
static uint8_t cursor_line = 1;  // Start on line 2 (0-indexed: line 1)

// GM Patch names (abbreviated for 20x4 display)
static const char* gm_patch_names[] = {
    "AcousticGrandPiano", "BrightAcousticPiano", "ElectricGrandPiano", "Honky-tonkPiano",
    "ElectricPiano1", "ElectricPiano2", "Harpsichord", "Clavi",
    "Celesta", "Glockenspiel", "MusicBox", "Vibraphone",
    "Marimba", "Xylophone", "TubularBells", "Dulcimer",
    "DrawbarOrgan", "PercussiveOrgan", "RockOrgan", "ChurchOrgan",
    "ReedOrgan", "Accordion", "Harmonica", "TangoAccordion",
    "AcousticGuitarNylon", "AcousticGuitarSteel", "ElectricGuitarJazz", "ElectricGuitarClean",
    "ElectricGuitarMuted", "OverdrivenGuitar", "DistortionGuitar", "GuitarHarmonics",
    "AcousticBass", "ElectricBassFinger", "ElectricBassPick", "FretlessBass",
    "SlapBass1", "SlapBass2", "SynthBass1", "SynthBass2",
    "Violin", "Viola", "Cello", "Contrabass",
    "TremoloStrings", "PizzicatoStrings", "OrchestralHarp", "Timpani",
    "StringEnsemble1", "StringEnsemble2", "SynthStrings1", "SynthStrings2",
    "ChoirAahs", "VoiceOohs", "SynthVoice", "OrchestraHit",
    "Trumpet", "Trombone", "Tuba", "MutedTrumpet",
    "FrenchHorn", "BrassSection", "SynthBrass1", "SynthBrass2",
    "SopranoSax", "AltoSax", "TenorSax", "BaritoneSax",
    "Oboe", "EnglishHorn", "Bassoon", "Clarinet",
    "Piccolo", "Flute", "Recorder", "PanFlute",
    "BlownBottle", "Shakuhachi", "Whistle", "Ocarina",
    "Lead1Square", "Lead2Sawtooth", "Lead3Calliope", "Lead4Chiff",
    "Lead5Charang", "Lead6Voice", "Lead7Fifths", "Lead8Bass+Lead",
    "Pad1NewAge", "Pad2Warm", "Pad3Polysynth", "Pad4Choir",
    "Pad5Bowed", "Pad6Metallic", "Pad7Halo", "Pad8Sweep",
    "FX1Rain", "FX2Soundtrack", "FX3Crystal", "FX4Atmosphere",
    "FX5Brightness", "FX6Goblins", "FX7Echoes", "FX8Sci-fi",
    "Sitar", "Banjo", "Shamisen", "Koto",
    "Kalimba", "Bagpipe", "Fiddle", "Shanai",
    "TinkleBell", "Agogo", "SteelDrums", "Woodblock",
    "TaikoDrum", "MelodicTom", "SynthDrum", "ReverseCymbal",
    "GuitarFretNoise", "BreathNoise", "Seashore", "BirdTweet",
    "TelephoneRing", "Helicopter", "Applause", "Gunshot"
};

static void render_display(void) {
    char line[21];
    
    // Line 1: MODE
    lcd_set_cursor(0, 0);
    if (cursor_line == 0) {
        snprintf(line, sizeof(line), ">MODE: %-13s", current_mode == MODE_SONG ? "SONG" : "MIDI-IN");
    } else {
        snprintf(line, sizeof(line), " MODE: %-13s", current_mode == MODE_SONG ? "SONG" : "MIDI-IN");
    }
    lcd_print(line);
    
    // Line 2: Channel info or Song name
    lcd_set_cursor(0, 1);
    if (current_mode == MODE_MIDI_IN) {
        // Show MIDI channel (1-9 or ALL)
        char prefix = (cursor_line == 1) ? '>' : ' ';
        if (selected_channel == CHANNEL_ALL) {
            snprintf(line, sizeof(line), "%cCh:ALL          ", prefix);
        } else {
            snprintf(line, sizeof(line), "%cCh:%02d           ", prefix, selected_channel + 1);
        }
    } else {
        // Show song name
        if (cursor_line == 1) {
            snprintf(line, sizeof(line), ">%-19s", song_name);
        } else {
            snprintf(line, sizeof(line), " %-19s", song_name);
        }
    }
    lcd_print(line);
    
    // Line 3: Volume/Octave or visualizer
    lcd_set_cursor(0, 2);
    if (current_mode == MODE_MIDI_IN) {
        // Show patch name (selectable)
        char patch_name[15];
        strncpy(patch_name, patch_names[selected_program], 14);
        patch_name[14] = '\0';
        char prefix = (cursor_line == 2) ? '>' : ' ';
        snprintf(line, sizeof(line), "%cP%03d:%-14s", prefix, selected_program, patch_name);
    } else {
        // Show playing status
        bool is_playing = song_player_is_playing();
        if (cursor_line == 2) {
            snprintf(line, sizeof(line), ">%-19s", is_playing ? "Playing..." : "Paused");
        } else {
            snprintf(line, sizeof(line), " %-19s", is_playing ? "Playing..." : "Paused");
        }
    }
    lcd_print(line);
    
    // Line 4: Voice activity or playback controls
    lcd_set_cursor(0, 3);
    if (current_mode == MODE_MIDI_IN) {
        // Show active voices as dots
        if (cursor_line == 3) {
            strcpy(line, ">ACT:");
        } else {
            strcpy(line, " ACT:");
        }
        for (int i = 0; i < 9; i++) {
            strcat(line, voice_states[i] ? "\xFF" : ".");  // Full block for active voice
        }
    } else {
        // Show voice activity in SONG mode too
        if (cursor_line == 3) {
            strcpy(line, ">ACT:");
        } else {
            strcpy(line, " ACT:");
        }
        for (int i = 0; i < 9; i++) {
            strcat(line, voice_states[i] ? "\xFF" : ".");  // Full block for active voice
        }
    }
    lcd_print(line);
}

void menu_init(void) {
    encoder_init();
    menu_dirty = true;
}

void menu_update(void) {
    uint32_t now = to_ms_since_boot(get_absolute_time());
    
    // Handle encoder rotation - moves cursor up/down
    int delta = encoder_get_delta();
    if (delta != 0) {
        if (current_mode == MODE_MIDI_IN && cursor_line == 1 && channel_edit_mode) {
            // Adjust channel selection while in edit mode (1-9 + ALL)
            // Cycle through: 0, 1, 2, 3, 4, 5, 6, 7, 8, ALL
            if (delta > 0) {
                if (selected_channel == 8) {
                    selected_channel = CHANNEL_ALL;
                } else if (selected_channel == CHANNEL_ALL) {
                    selected_channel = 0;
                } else {
                    selected_channel++;
                }
            } else {
                if (selected_channel == 0) {
                    selected_channel = CHANNEL_ALL;
                } else if (selected_channel == CHANNEL_ALL) {
                    selected_channel = 8;
                } else {
                    selected_channel--;
                }
            }
            menu_dirty = true;
        } else if (current_mode == MODE_MIDI_IN && cursor_line == 2 && patch_edit_mode) {
            // Adjust patch selection while in edit mode
            int new_program = (int)selected_program + delta;
            while (new_program < 0) new_program += 256;
            while (new_program > 255) new_program -= 256;
            selected_program = (uint8_t)new_program;
            
            // Update channel(s) with new program
            if (selected_channel == CHANNEL_ALL) {
                // Set all 9 channels to this program
                for (int i = 0; i < 9; i++) {
                    midi_set_program(i, selected_program);
                }
            } else {
                // Set single channel
                midi_set_program(selected_channel, selected_program);
            }
            menu_dirty = true;
        } else {
            // Move cursor
            int new_cursor = cursor_line + delta;
            if (new_cursor < 0) new_cursor = 3;
            if (new_cursor > 3) new_cursor = 0;
            cursor_line = new_cursor;
            patch_edit_mode = false;
            channel_edit_mode = false;
            menu_dirty = true;
        }
    }
    
    // Handle encoder button - performs action on selected line
    encoder_button_event_t btn = encoder_get_button();
    if (btn == ENCODER_BTN_PRESS) {
        // Short press: activate/adjust current line
        if (cursor_line == 0) {
            // MODE line - toggle mode
            menu_mode_t new_mode = (current_mode == MODE_SONG) ? MODE_MIDI_IN : MODE_SONG;
            
            if (new_mode == MODE_MIDI_IN) {
                // Switching to MIDI-IN: stop song player, enable MIDI input
                song_player_pause();
                midi_input_set_enabled(true);
                patch_edit_mode = false;
                channel_edit_mode = false;
            } else {
                // Switching to SONG: disable MIDI input, start song player
                midi_input_set_enabled(false);
                song_player_play();
                patch_edit_mode = false;
                channel_edit_mode = false;
            }
            
            current_mode = new_mode;
            menu_dirty = true;
        }
        else if (cursor_line == 1) {
            if (current_mode == MODE_MIDI_IN) {
                // Toggle channel edit mode
                channel_edit_mode = !channel_edit_mode;
                menu_dirty = true;
            }
            // In SONG mode, could skip to next song (future)
        }
        else if (cursor_line == 2) {
            if (current_mode == MODE_MIDI_IN) {
                // Toggle patch edit mode
                patch_edit_mode = !patch_edit_mode;
                menu_dirty = true;
            } else {
                // SONG mode: toggle play/pause
                if (song_player_is_playing()) {
                    song_player_pause();
                } else {
                    song_player_play();
                }
                menu_dirty = true;
            }
        }
        else if (cursor_line == 3) {
            // Execute playback control (future)
            menu_dirty = true;
        }
    }
    else if (btn == ENCODER_BTN_LONG_PRESS) {
        // Long press: back out / decrease value
        if (cursor_line == 2 && current_mode == MODE_MIDI_IN) {
            // Decrease volume
            volume = (volume - 10 < 0) ? 0 : volume - 10;
            menu_dirty = true;
        }
    }
    
    // Update display at 10Hz or when dirty
    if (menu_dirty || (now - last_update) > 100) {
        render_display();
        last_update = now;
        menu_dirty = false;
    }
}

void menu_set_mode(menu_mode_t mode) {
    if (current_mode != mode) {
        current_mode = mode;
        menu_dirty = true;
    }
}

menu_mode_t menu_get_mode(void) {
    return current_mode;
}

void menu_update_voices(const bool voice_active[9]) {
    bool changed = false;
    for (int i = 0; i < 9; i++) {
        if (voice_states[i] != voice_active[i]) {
            voice_states[i] = voice_active[i];
            changed = true;
        }
    }
    if (changed) {
        menu_dirty = true;
    }
}

void menu_set_song_name(const char *name) {
    strncpy(song_name, name, 20);
    song_name[20] = '\0';
    menu_dirty = true;
}
