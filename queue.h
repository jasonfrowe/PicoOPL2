#ifndef QUEUE_H
#define QUEUE_H

#include <stdint.h>
#include "pico/util/queue.h"

// --- The Data Packet ---
// Used by: 
// 1. Python Script (generates arrays of this)
// 2. Queue (passes these between cores)
// 3. Audio Engine (processes MIDI events)

typedef struct {
    uint8_t type;      // 1=NoteOn, 0=NoteOff, 3=PatchChange
    uint16_t delay_ms; // 16-bit Delay
    uint8_t channel;   // 0-8
    uint8_t note;      // MIDI Note (0-127) or Program Number
    uint8_t velocity;  // 0-127 (Volume Dynamics)
} SongEvent;

#endif // QUEUE_H