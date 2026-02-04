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
#include "song_player.h"
#include "midi_input.h"
#include "lcd.h"
#include "encoder.h"
#include "menu.h"
#include "queue.h"

// --- MAIN ---
int main() {
    stdio_init_all();
    hardware_setup();
    
    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    // Init LCD
    printf("Initializing LCD...\n");
    if (lcd_init()) {
        lcd_backlight(true);
        lcd_clear();
        lcd_print_at(0, 0, "   PicoOPL2 Synth");
        lcd_print_at(0, 1, "    Initializing...");
    } else {
        printf("LCD: Failed to initialize\n");
    }
    
    // Init menu system
    menu_init();
    menu_set_song_name("Doom E1M1");
    
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
    
    // Initialize song player
    song_player_init();
    
    // Initialize MIDI input
    midi_input_init();
    midi_input_set_enabled(true);  // Start in MIDI-IN mode
    
    // Main loop - update menu, song player, and MIDI input
    while (true) {
        // Update voice activity for menu display
        bool voice_states[9];
        get_voice_states(voice_states);
        menu_update_voices(voice_states);
        
        menu_update();
        
        // Update appropriate input source based on mode
        if (menu_get_mode() == MODE_SONG) {
            song_player_update(LED_PIN);
        } else {
            midi_input_update();
        }
        
        sleep_ms(10);  // 100Hz update rate
    }
}