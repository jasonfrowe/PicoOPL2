/**
 * midi_input.c
 * 
 * MIDI Input Handler Implementation
 * Receives MIDI data via UART and sends events to audio engine
 */

#include "midi_input.h"
#include "audio_engine.h"
#include "queue.h"
#include "menu.h"
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/irq.h"
#include <stdio.h>

// MIDI UART Configuration
#define MIDI_UART uart0
#define MIDI_UART_TX_PIN 16  // Not used, but needed for UART init
#define MIDI_UART_RX_PIN 17  // Connected to 6N138 output
#define MIDI_BAUD_RATE 31250
#define CHANNEL_ALL 255  // Must match menu.c definition

// MIDI Parser State
static bool enabled = false;
static uint8_t running_status = 0;
static uint8_t data_byte_count = 0;
static uint8_t midi_data[3];

// Forward declaration
static void process_midi_message(void);

// UART interrupt handler for immediate MIDI byte processing
static void on_uart_rx(void) {
    while (uart_is_readable(MIDI_UART)) {
        uint8_t byte = uart_getc(MIDI_UART);
        
        if (!enabled) continue;
        
        // Check if this is a status byte (bit 7 set)
        if (byte & 0x80) {
            // Status byte
            if (byte >= 0xF0) {
                // System message - ignore for now
                running_status = 0;
                data_byte_count = 0;
                continue;
            }
            
            // Channel message
            running_status = byte;
            midi_data[0] = byte;
            data_byte_count = 0;
        } else {
            // Data byte
            if (running_status == 0) {
                // No running status, ignore
                continue;
            }
            
            // Store data byte
            midi_data[1 + data_byte_count] = byte;
            data_byte_count++;
            
            // Determine if we have a complete message
            uint8_t command = running_status & 0xF0;
            uint8_t expected_bytes = 2; // Most messages are 2 data bytes
            
            if (command == 0xC0 || command == 0xD0) {
                // Program Change and Channel Pressure only have 1 data byte
                expected_bytes = 1;
            }
            
            if (data_byte_count >= expected_bytes) {
                // Complete message received - process immediately
                process_midi_message();
                data_byte_count = 0;
            }
        }
    }
}

void midi_input_init(void) {
    // Initialize UART for MIDI
    uart_init(MIDI_UART, MIDI_BAUD_RATE);
    
    // Set UART format: 8 data bits, 1 stop bit, no parity
    uart_set_format(MIDI_UART, 8, 1, UART_PARITY_NONE);
    
    // Set GPIO pins
    gpio_set_function(MIDI_UART_RX_PIN, GPIO_FUNC_UART);
    
    // Enable FIFO
    uart_set_fifo_enabled(MIDI_UART, true);
    
    // Set up interrupt for RX - process MIDI bytes immediately when they arrive
    int uart_irq = MIDI_UART == uart0 ? UART0_IRQ : UART1_IRQ;
    irq_set_exclusive_handler(uart_irq, on_uart_rx);
    irq_set_enabled(uart_irq, true);
    
    // Enable UART RX interrupt
    uart_set_irq_enables(MIDI_UART, true, false);
    
    printf("MIDI Input initialized on GPIO-%d with interrupt\n", MIDI_UART_RX_PIN);
    
    enabled = false;
    running_status = 0;
    data_byte_count = 0;
}

void midi_input_set_enabled(bool en) {
    enabled = en;
    if (enabled) {
        printf("MIDI Input enabled\n");
        // Flush any old data
        while (uart_is_readable(MIDI_UART)) {
            uart_getc(MIDI_UART);
        }
        running_status = 0;
        data_byte_count = 0;
    } else {
        printf("MIDI Input disabled\n");
    }
}

static void process_midi_message(void) {
    uint8_t status = midi_data[0];
    uint8_t channel = status & 0x0F;
    uint8_t command = status & 0xF0;
    
    // Accept all MIDI channels - patches are managed via menu
    
    SongEvent event = {0};
    
    switch (command) {
        case 0x80: // Note Off
            event.type = 0;  // Note Off
            event.channel = channel;
            event.note = midi_data[1];
            event.velocity = 0;
            event.delay_ms = 0;
            audio_engine_add_event(&event);
            break;
            
        case 0x90: // Note On
            if (midi_data[2] == 0) {
                // Velocity 0 = Note Off
                event.type = 0;  // Note Off
                event.channel = channel;
                event.note = midi_data[1];
                event.velocity = 0;
                event.delay_ms = 0;
            } else {
                // Real Note On
                event.type = 1;  // Note On
                event.channel = channel;
                event.note = midi_data[1];
                event.velocity = midi_data[2];
                event.delay_ms = 0;
            }
            audio_engine_add_event(&event);
            break;
            
        case 0xC0: // Program Change
            event.type = 3;  // Program Change
            event.channel = channel;
            event.note = midi_data[1];  // Program number
            event.velocity = 0;
            event.delay_ms = 0;
            audio_engine_add_event(&event);
            break;
            
        default:
            // Ignore other messages for now
            break;
    }
}

void midi_input_update(void) {
    // MIDI is now interrupt-driven, so this function is mostly a no-op
    // Just used to ensure any stale data is cleared when re-enabling
    if (!enabled) {
        return;
    }
}
