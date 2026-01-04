# PicoOPL2 Port Checklist

## Project Overview
Porting functionality from `pico_jukebox.c` to `PicoOPL2.c` for Raspberry Pi Pico + Yamaha OPL2 chip project.

**Status:** ✅ **PROJECT COMPLETE - DOOM MUSIC PLAYING PERFECTLY!**

---

## Summary

All core functionality has been successfully ported and tested:
- ✅ Multi-core audio engine operational
- ✅ 9-voice polyphonic playback with LRU voice stealing  
- ✅ MIDI velocity dynamics working correctly
- ✅ All 128 GM instruments loaded and functional
- ✅ Hardware issue identified and resolved (YM3014B pin 7-8 requires 1µF cap to ground)
- ✅ Clean audio output with no distortion

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

## Testing & Validation - ALL COMPLETE! ✅

### 13. Initial Testing
- ✅ Compiled with TEST_MODE=1
- ✅ Music plays perfectly - no distortion
- ✅ Voice allocation working (9 voices, drum on Ch 8)
- ✅ Velocity dynamics working correctly
- ✅ Program changes working

### 14. External Mode Testing
- ⏳ Test with TEST_MODE=0 (ready for future testing)
- ⏳ Verify GPIO handshake protocol with external controller
- ⏳ Test with RP6502 or external MIDI source

---

## Hardware Configuration Notes

### YM3014B DAC Wiring (Critical!)
**Correct pin connections:**
- Pin 1 (Vdd) → +5V
- Pin 2 (ToBUFF) → 10µF AC coupling cap → LM358 input
- Pin 3 (LOAD) → YM3812 pin 20 (SH1)
- Pin 4 (SD) → YM3812 pin 21 (DO)
- Pin 5 (Clock) → YM3812 pin 23 (SH2)
- Pin 6 (GND) → Ground
- **Pin 7 (Rb) & Pin 8 (MP) → Bridge together → 1µF ceramic cap to ground** ⚠️

**Critical:** The 1µF capacitor between pins 7-8 and ground is REQUIRED to prevent distortion at high volumes.

---

## Code Cleanup - COMPLETE ✅

### 15. Remove Old Code
- ✅ Removed SIMPLE_TEST_MODE debugging code
- ⏳ Eventually deprecate old opl.c (currently unused)
- ✅ All references use opl2_write(), opl2_note_on(), etc.

### 16. Documentation
- ✅ Pin assignments documented in opl2_hardware.h
- ✅ Packet protocol documented in code comments
- ✅ Voice allocation strategy documented
- ✅ Hardware configuration documented above

---

## Files Summary

**Active Production Files:**
- ✅ PicoOPL2.c - Main program with voice allocator and dual-core engine
- ✅ opl2.c - Low-level OPL2 interface functions
- ✅ opl2.h - OPL2 function declarations
- ✅ opl2_hardware.c - Hardware initialization and pin setup
- ✅ opl2_hardware.h - Pin definitions and hardware interface
- ✅ instruments.c - GM instrument bank (128 patches)
- ✅ instruments.h - Instrument loading interface
- ✅ queue.h - SongEvent structure and queue definitions
- ✅ song_data.h - MIDI song data (Doom E1M1)

**Legacy Files (can be removed):**
- 📚 pico_jukebox.c - Original reference (port complete)
- 📚 opl.c - Old functions (replaced by opl2.c)
- 📚 opl.h - Old interface (replaced by opl2.h)

---

## PROJECT COMPLETE! 🎉

**Successfully ported and verified:**
- ✅ Multi-core audio engine with queue-based event processing
- ✅ 9-voice polyphonic playback with LRU voice stealing
- ✅ Full GM instrument support (128 patches + drums)
- ✅ MIDI velocity dynamics
- ✅ Program change support
- ✅ Clean audio output (no distortion)
- ✅ Hardware issue identified and documented

**Ready for:**
- External mode testing with GPIO interface
- Custom song data
- Integration with external MIDI controllers

**Next Immediate Steps:**
1. Port basic OPL functions from opl.c to opl2.c
2. Update instruments.c to use opl2_write()
3. Add data structures and voice management
4. Implement Core 1 audio engine
5. Add TEST_MODE with song playback
