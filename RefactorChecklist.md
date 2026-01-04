# PicoOPL2 Synth Refactoring Checklist

## Project Goal
Transform the PicoOPL2 from a GPIO sound card into a **standalone MIDI synthesizer** with proper hardware interface and modular code structure.

**Status:** 🔄 In Progress

---

## Phase 1: Code Cleanup & Refactoring

### 1.1 Remove GPIO Interface Code ✅
**Goal:** Remove all RP6502 GPIO communication code (no longer needed)

- [x] Remove `PIN_REQ` and `PIN_ACK` definitions
- [x] Remove `read_data_bus()` function
- [x] Remove all `#if !TEST_MODE` GPIO setup code
- [x] Remove external mode packet assembly code
- [x] Simplify `TEST_MODE` - it's now always enabled
- [x] Clean up main() to remove GPIO-specific initialization

**Status:** ✅ COMPLETE

**Files modified:**
- PicoOPL2.c - Removed all GPIO interface code, simplified to standalone mode

---

### 1.2 Extract Voice Management to Separate Module ✅
**Goal:** Move voice allocator functions into their own files

Create `voice_manager.c` and `voice_manager.h`:

- [x] Move `OPLVoice` struct definition
- [x] Move `voices[9]` array
- [x] Move `note_counter` variable
- [x] Move `init_voices()` function
- [x] Move `allocate_voice()` function
- [x] Move `find_active_voice()` function
- [x] Move `apply_velocity()` function
- [x] Add proper header guards and includes

**Status:** ✅ COMPLETE

**Files created:**
- voice_manager.c - Voice allocation and management implementation
- voice_manager.h - Public interface with function declarations

**Files modified:**
- PicoOPL2.c (removed ~100 lines, added include)
- CMakeLists.txt (added voice_manager.c)

---

### 1.3 Extract MIDI Channel State Management ✅
**Goal:** Separate MIDI program tracking

Create `midi_state.c` and `midi_state.h`:

- [x] Move `midi_ch_program[16]` array
- [x] Create `midi_state_init()` function
- [x] Create `midi_set_program(channel, program)` function
- [x] Create `midi_get_program(channel)` function
- [x] Add proper header guards and includes

**Status:** ✅ COMPLETE

**Files created:**
- midi_state.c - MIDI channel program state management
- midi_state.h - Public interface for MIDI state

**Files modified:**
- PicoOPL2.c (removed array, replaced with function calls)
- CMakeLists.txt (added midi_state.c)

---

### 1.4 Extract Audio Engine to Separate Module ✅
**Goal:** Move Core 1 audio processing logic

Create `audio_engine.c` and `audio_engine.h`:

- [x] Move `event_queue` declaration
- [x] Move `core1_entry()` function
- [x] Create `audio_engine_init()` function
- [x] Create `audio_engine_start()` function (launches Core 1)
- [x] Create `audio_engine_add_event()` function
- [x] Add proper header guards and includes

**Status:** ✅ COMPLETE

**Files created:**
- audio_engine.c - Core 1 audio processing implementation
- audio_engine.h - Public interface for audio engine

**Files modified:**
- PicoOPL2.c (removed ~70 lines of audio engine code)
- CMakeLists.txt (added audio_engine.c)

---

### 1.5 Create Song Player Module
**Goal:** Separate test song playback logic from main

Create `song_player.c` and `song_player.h`:

- [ ] Move internal jukebox loop from main()
- [ ] Create `song_player_start()` function
- [ ] Create `song_player_loop()` function
- [ ] Keep song_data.h include isolated to this module
- [ ] Add proper header guards and includes

**Files to create:**
- song_player.c
- song_player.h

**Files to modify:**
- PicoOPL2.c (remove moved code, add include)
- CMakeLists.txt (add song_player.c)

---

### 1.6 Simplify Main File
**Goal:** Make PicoOPL2.c a clean entry point

- [ ] Keep only main() function
- [ ] Remove all TEST_MODE conditionals (always in test/internal mode for now)
- [ ] Call initialization functions from other modules
- [ ] Add clear comments explaining program flow
- [ ] Keep file under 100 lines

**Target structure:**
```c
#include "pico/stdlib.h"
#include "opl2_hardware.h"
#include "opl2.h"
#include "voice_manager.h"
#include "midi_state.h"
#include "audio_engine.h"
#include "song_player.h"

int main() {
    // 1. Initialize hardware
    // 2. Initialize OPL2
    // 3. Initialize voice manager
    // 4. Initialize MIDI state
    // 5. Start audio engine
    // 6. Start song player
    // 7. Idle loop
}
```

---

## Phase 2: Hardware Interface Preparation

### 2.1 Update Pin Definitions (Assembly.md)
**Goal:** Match new hardware design from Assembly.md

Update `opl2_hardware.h`:

- [ ] Change data bus from GP16-22,26 → GP2-GP9
- [ ] Update bitmask from 0x047F0000 → 0x000003FC
- [ ] Keep A0=GP0, WR=GP10, CS=GP11, IC=GP17
- [ ] Document GP1 reserved for MIDI RX (future use)
- [ ] Document GP12/GP13 reserved for I2C LCD (future use)
- [ ] Document GP14/GP15/GP16 reserved for rotary encoder (future use)
- [ ] Update clock output pin if needed (currently GP21)

**Files to modify:**
- opl2_hardware.h
- opl2_hardware.c (update gpio_put_masked calls)

---

### 2.2 Add Power Management Placeholder
**Goal:** Document power system from Assembly.md

Create `power.h` with comments only (no implementation yet):

- [ ] Document TP5100 charging system
- [ ] Document BMS protection
- [ ] Document LM2596 buck converter (5V output)
- [ ] Document IRF4905 power path MOSFET
- [ ] Document battery monitoring (future ADC implementation)

**Files to create:**
- power.h (documentation only)

---

### 2.3 Add LCD Interface Placeholder
**Goal:** Prepare for LCD backpack (I2C)

Create `lcd.h` and `lcd.c` stubs:

- [ ] Define I2C pins (GP12=SDA, GP13=SCL)
- [ ] Add TODO comments for functions:
  - `lcd_init()`
  - `lcd_clear()`
  - `lcd_print()`
  - `lcd_set_cursor()`
- [ ] Document level shifter requirement (BOB-12009)

**Files to create:**
- lcd.h
- lcd.c (stub functions only)

---

### 2.4 Add Rotary Encoder Interface Placeholder
**Goal:** Prepare for UI controls

Create `encoder.h` and `encoder.c` stubs:

- [ ] Define encoder pins (GP14=A, GP15=B, GP16=SW)
- [ ] Add TODO comments for functions:
  - `encoder_init()`
  - `encoder_read()` - returns delta
  - `encoder_button_pressed()`
- [ ] Document interrupt-based implementation plan

**Files to create:**
- encoder.h
- encoder.c (stub functions only)

---

### 2.5 Add Audio Output Chain Documentation
**Goal:** Document NE5532P op-amp configuration

Create `audio_output.h` with comments only:

- [ ] Document virtual ground (2.5V reference)
- [ ] Document mixer configuration (OPL2 + Line-In)
- [ ] Document Class-A mod (4.7kΩ to ground)
- [ ] Document line-out with Muse cap and divider
- [ ] Document volume pot connection to PAM8406

**Files to create:**
- audio_output.h (documentation only)

---

## Phase 3: MIDI Preparation (Parts on Order)

### 3.1 Add UART MIDI Receive Module (Stub)
**Goal:** Prepare for 6N138 optocoupler input

Create `midi_input.h` and `midi_input.c`:

- [ ] Define MIDI pins (GP1 = UART0 RX)
- [ ] Add 6N138 wiring documentation in comments
- [ ] Create stub functions:
  - `midi_input_init()` - Configure UART0
  - `midi_input_available()` - Check for data
  - `midi_input_read()` - Read MIDI byte
- [ ] Document 31250 baud rate requirement
- [ ] Add TODO for MIDI parser

**Files to create:**
- midi_input.h
- midi_input.c (stub with documentation)

---

### 3.2 Add MIDI Parser Module (Stub)
**Goal:** Prepare for MIDI message parsing

Create `midi_parser.h` and `midi_parser.c`:

- [ ] Create MIDI message structs:
  - `MidiNoteOn`
  - `MidiNoteOff`
  - `MidiProgramChange`
  - `MidiControlChange`
- [ ] Create stub parser functions:
  - `midi_parse_byte()` - State machine
  - `midi_parse_to_event()` - Convert to SongEvent
- [ ] Document running status handling
- [ ] Add TODO for system exclusive messages

**Files to create:**
- midi_parser.h
- midi_parser.c (stub functions)

---

### 3.3 Create MIDI Configuration
**Goal:** Add MIDI mode selection

- [ ] Add `#define MIDI_MODE 0` to configuration
- [ ] When 0: Use internal song player
- [ ] When 1: Use UART MIDI input (future)
- [ ] Update main() to check mode and route accordingly

**Files to modify:**
- PicoOPL2.c (add mode selection)

---

## Phase 4: Build System Updates

### 4.1 Update CMakeLists.txt
**Goal:** Add all new source files

- [ ] Add voice_manager.c
- [ ] Add midi_state.c
- [ ] Add audio_engine.c
- [ ] Add song_player.c
- [ ] Add lcd.c (stub)
- [ ] Add encoder.c (stub)
- [ ] Add midi_input.c (stub)
- [ ] Add midi_parser.c (stub)
- [ ] Add any required libraries (pico_multicore already included)

---

### 4.2 Update Build Configuration
**Goal:** Ensure clean compilation

- [ ] Verify all includes are correct
- [ ] Check for circular dependencies
- [ ] Add header guards to all new headers
- [ ] Test compilation after each phase
- [ ] Document any new library requirements

---

## Phase 5: Testing & Validation

### 5.1 Test Refactored Code
**Goal:** Verify nothing broke

- [ ] Compile successfully
- [ ] Flash to hardware
- [ ] Verify Doom song still plays correctly
- [ ] Verify no distortion
- [ ] Verify velocity dynamics work
- [ ] Check memory usage (should be similar)

---

### 5.2 Hardware Pin Test (When Updated)
**Goal:** Verify new pin assignments work

- [ ] Test OPL2 communication with new data bus (GP2-GP9)
- [ ] Verify clock output on GP21
- [ ] Test that reserved pins don't interfere
- [ ] Document any issues found

---

### 5.3 Stub Module Tests
**Goal:** Verify stubs don't cause issues

- [ ] Call lcd_init() - should not crash
- [ ] Call encoder_init() - should not crash
- [ ] Call midi_input_init() - should not crash
- [ ] Verify stubs return safe defaults

---

## Phase 6: Documentation

### 6.1 Create README.md
**Goal:** Document the complete project

- [ ] Hardware overview with schematic references
- [ ] Bill of materials (from Assembly.md)
- [ ] Software architecture diagram
- [ ] Pin assignment table
- [ ] Build instructions
- [ ] Flashing instructions
- [ ] Future features roadmap

---

### 6.2 Code Documentation
**Goal:** Add comprehensive comments

- [ ] Add file headers to all modules
- [ ] Document all public functions
- [ ] Add architecture notes to README
- [ ] Document YM3014B 1µF cap requirement!
- [ ] Create troubleshooting guide

---

## Phase 7: Future Features (Post-MIDI Parts)

### 7.1 MIDI Implementation
- [ ] Implement UART RX interrupt handler
- [ ] Implement MIDI parser state machine
- [ ] Test with MIDI keyboard
- [ ] Add MIDI channel filtering
- [ ] Add velocity curve adjustment

### 7.2 UI Implementation
- [ ] Implement LCD driver
- [ ] Implement rotary encoder driver
- [ ] Create menu system
- [ ] Add program selection
- [ ] Add parameter editing

### 7.3 Advanced Features
- [ ] Add preset storage (flash)
- [ ] Add MIDI learn mode
- [ ] Add voice monitoring display
- [ ] Add audio level metering
- [ ] Add battery level display

---

## Notes

### Current Working Features ✅
- OPL2 communication and synthesis
- 9-voice polyphonic playback
- Voice allocation with LRU stealing
- MIDI velocity dynamics
- Program change support
- Internal song playback (Doom E1M1)
- Clean audio output (1µF cap fix applied)

### Hardware Status
- ✅ YM3812 OPL2 chip working
- ✅ YM3014B DAC working (with 1µF cap fix)
- ✅ PWM clock generation working
- ⏳ Power system (being assembled per Assembly.md)
- ⏳ MIDI input (parts on order)
- ⏳ LCD display (ready to connect)
- ⏳ Rotary encoder (ready to connect)
- ⏳ Audio output chain (NE5532P, PAM8406)

### Design Decisions
- Removed GPIO interface (RP6502 is separate project)
- Now building standalone synthesizer
- TEST_MODE becomes MIDI_MODE (0=internal songs, 1=UART MIDI)
- Modular code structure for easier feature additions
- Hardware pin assignments match Assembly.md
- Prepare stubs for all future hardware interfaces

---

## Priority Order

**Phase 1 (Immediate):** Code cleanup and refactoring
**Phase 2 (Next):** Update pin definitions for new hardware
**Phase 3 (Waiting on parts):** MIDI preparation (stubs)
**Phase 4:** Build system
**Phase 5:** Testing
**Phase 6:** Documentation
**Phase 7:** Future implementation

**Start with:** Phase 1.1 - Remove GPIO interface code
