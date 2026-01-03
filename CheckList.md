# PicoOPL2 Port Checklist

## Project Overview
Porting functionality from `pico_jukebox.c` to `PicoOPL2.c` for Raspberry Pi Pico + Yamaha OPL2 chip project.

**Status Legend:**
- ✅ Complete
- 🔄 In Progress
- ⏳ To Do
- 📝 Notes/Issues

---

## Core Infrastructure

### 1. Hardware Setup
- ✅ `hardware_setup()` - Already done (replaces `setup_pins()`)
- ✅ `opl2_write()` - Already implemented in opl2.c
- ✅ `opl2_clear()` - Ported from `opl_clear()` in opl.c

### 2. OPL Helper Functions (Port from opl.c → opl2.c)
- ✅ `opl2_note_on()` - Ported from `OPL_NoteOn()` 
- ✅ `opl2_note_off()` - Ported from `OPL_NoteOff()`
- ✅ `midi_to_opl2_freq()` - Ported from `midi_to_opl_freq()`
- ✅ `shadow_b0[9]` array - Shadow registers for channels
- ✅ `fnum_table[12]` - Frequency number lookup table

### 3. Data Structures & Types
- ✅ Include `queue.h` in PicoOPL2.c
- ✅ Define `SongEvent` structure (already in queue.h)
- ✅ Create event queue: `queue_t event_queue`
- ✅ Create `OPLVoice` structure (9 voices)
- ✅ Create `voices[9]` array for voice allocation
- ✅ Create `note_counter` for age tracking
- ✅ Create `midi_ch_program[16]` for MIDI channel instruments

---

## Voice Management System

### 4. Voice Allocator Functions
- ✅ `init_voices()` - Initialize voice array
- ✅ `allocate_voice()` - Find/allocate physical voice for MIDI note
  - Handles retrigger (same note, same channel)
  - MIDI Channel 9 → Physical Voice 8 (drums)
  - Find free voices 0-7 for melodic
  - Voice stealing (oldest LRU)
- ✅ `find_active_voice()` - Locate which voice is playing a note

### 5. Audio Helper Functions  
- ✅ `apply_velocity()` - Convert MIDI velocity to OPL volume
  - Uses `shadow_carrier_ksl[9]` from instruments.c
  - Attenuates carrier TL register

---

## Instrument System

### 6. Instrument Loading (Already in instruments.c)
- ✅ `load_gm_instrument()` - Load General MIDI patch
- ✅ `load_drum_patch()` - Load drum sound
- ✅ `shadow_carrier_ksl[9]` - Global shadow array for velocity
- ✅ Update instruments.c to use `opl2_write()` instead of `opl_write()`

---

## Multi-Core Audio Engine

### 7. Core 1 - Audio Processing Thread
- ✅ `core1_entry()` - Main audio engine loop
  - Process events from queue
  - Handle delays with `sleep_ms()`
  - Event types:
    - Type 0: Note Off
    - Type 1: Note On (with velocity)
    - Type 2: Reset
    - Type 3: Program Change

### 8. Core 0 - Main Loop Setup
- ✅ Initialize stdio
- ✅ Setup data bus pins (GPIO 16-22, 26 for 8-bit data)
- ✅ Setup interface pins (PIN_REQ=27, PIN_ACK=28)
- ✅ Initialize queue (`queue_init()`)
- ✅ Load default instruments
- ✅ Launch Core 1 with `multicore_launch_core1()`

---

## Operation Modes

### 9. TEST_MODE (Internal Playback)
- ✅ Add `#define TEST_MODE 1` 
- ✅ Include "song_data.h" when TEST_MODE enabled
- ✅ Feed `midi_song[]` array into queue
- ✅ Loop playback with reset between iterations
- ✅ Handle Type 2 (end of song marker)

### 10. EXTERNAL MODE (GPIO Communication)
- ✅ `read_data_bus()` - Read 8-bit parallel data from GPIOs
- ✅ Implement REQ/ACK handshake protocol
- ✅ 6-byte packet assembly:
  - Byte 0: type
  - Bytes 1-2: delay_ms (16-bit)
  - Byte 3: channel
  - Byte 4: note
  - Byte 5: velocity
- ✅ Push assembled packets to queue

---

## Configuration & Dependencies

### 11. Header Includes
- ✅ Add `#include "pico/multicore.h"`
- ✅ Add `#include "pico/util/queue.h"`
- ✅ Add `#include "opl2.h"`
- ✅ Add `#include "instruments.h"`
- ✅ Add `#include "queue.h"`
- ✅ Add conditional `#include "song_data.h"`

### 12. Pin Definitions
- ✅ Define `PIN_REQ 27`
- ✅ Define `PIN_ACK 28`
- ✅ Define data bus pins array: `{16,17,18,19,20,21,22,26}`

---

## Testing & Validation

### 13. Initial Testing
- ✅ Compile with TEST_MODE=1
- 🎵 Verify song playback from song_data.h - **READY TO TEST!**
- ⏳ Test voice allocation (9 voices, drum on Ch 8)
- ⏳ Test velocity dynamics
- ⏳ Test program changes

### 14. External Mode Testing
- ⏳ Test with TEST_MODE=0
- ⏳ Verify GPIO handshake protocol
- ⏳ Test with RP6502 or external controller
- ⏳ Verify packet assembly and parsing

---

## Code Cleanup

### 15. Remove Old Code
- ⏳ Eventually remove/deprecate old opl.c functions
- ⏳ Update all references from `opl_write` to `opl2_write`
- ⏳ Consolidate all active code into opl2.c/opl2.h

### 16. Documentation
- ⏳ Document pin assignments
- ⏳ Document packet protocol
- ⏳ Document voice allocation strategy
- ⏳ Comment the frequency tuning (4.0 MHz FPGA vs 3.58 MHz standard)

---

## Missing Files to Copy

### 17. Files Status
- ✅ `song_data.h` - Already copied
- ✅ `queue.h` - Already present
- ✅ `instruments.c` - Already present
- ✅ `instruments.h` - Already present

---

## Notes & Decisions

### Hardware Configuration
- OPL2 uses direct hardware interface (not FPGA)
- Pin mappings already set in opl2_hardware.h
- Wait times: ADDRESS=3.3µs, DATA=23µs

### Frequency Tuning
- Original opl.c has frequency compensation for 4.0 MHz FPGA vs 3.58 MHz OPL2
- Need to verify if this compensation is needed for real OPL2 chip
- Current fnum_table in opl.c: `{308, 325, 345, 365, 387, 410, 434, 460, 487, 516, 547, 579}`

### Voice Allocation Strategy
- 9 physical OPL voices (channels 0-8)
- Voice 8 reserved for drums (MIDI channel 9)
- Voices 0-7 for melodic instruments
- LRU (Least Recently Used) voice stealing when all voices busy

### Queue Configuration
- Queue size: 512 events
- Blocking add/remove for flow control
- Core 0 produces events, Core 1 consumes

---

## Current File Status

**Working Files:**
- ✅ PicoOPL2.c - Basic LED blink test with opl2_write
- ✅ opl2.c - Low-level write function
- ✅ opl2_hardware.c - Hardware initialization  
- ✅ opl2_hardware.h - Pin definitions
- ✅ instruments.c - GM instrument bank
- ✅ instruments.h - Instrument interface

**Reference Files (old system):**
- 📚 pico_jukebox.c - Source for porting
- 📚 opl.c - Legacy functions to port
- 📚 opl.h - Legacy interface

**Next Immediate Steps:**
1. Port basic OPL functions from opl.c to opl2.c
2. Update instruments.c to use opl2_write()
3. Add data structures and voice management
4. Implement Core 1 audio engine
5. Add TEST_MODE with song playback
