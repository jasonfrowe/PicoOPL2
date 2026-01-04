# 🎹 Pico OPL2 Synth: Comprehensive Parts List

## 1. Core Logic & FM Synthesis
| Component | Qty | Description | Notes |
| :--- | :--- | :--- | :--- |
| **Raspberry Pi Pico** | 1 | RP2040 MCU | Core 0: MIDI/UI, Core 1: OPL2 Timing. |
| **Yamaha YM3812** | 1 | OPL2 FM Synthesis Chip | Pin 1 = VCC, Pin 12 = GND, Pin 20 = NC/GND. |
| **Yamaha YM3014B** | 1 | Serial Floating DAC | Pairs with YM3812. |
| **SN74LVC245AN** | 1 | 8-bit Level Shifter | [Mouser: 595-SN74LVC245AN] |
| **3.579545 MHz Osc** | 1 | ECS-2100X Half-Size | Optional (Pico can generate via PWM). |
| **IC Sockets** | 5 | 8, 14, 20, 24-pin | Use **Turned Pin** for OPL2/DAC. |

## 2. Audio Path (High Fidelity)
| Component | Qty | Description | Notes |
| :--- | :--- | :--- | :--- |
| **NE5532P** | 2 | Dual Low-Noise Op-Amp | Replaces all LM358s for better SNR. |
| **PAM8406 Module** | 1 | 5W Stereo Amp | Can switch between Class D and Class AB. |
| **2.5" Speakers** | 2 | 4Ω or 8Ω Full Range | 3W - 5W rating recommended. |
| **10kΩ Dual Pot** | 1 | Logarithmic Taper | Main Volume (Ganged for Stereo). |
| **3.5mm Stereo Jack** | 1 | Switched (5-Pin) | Disconnects speakers when plugged in. |
| **10µF Audio Cap** | 10 | Nichicon Muse ES (Bipolar) | [647-UES1H100MPM1TD] - Signal coupling. |
| **10µF Ceramic Cap** | 5 | KEMET Radial Ceramic | [80-C330C106K5R5TA] - DAC Ref (Pin 7/8). |

## 3. Power Path & Battery System (12V Integrated)
| Component | Qty | Description | Notes |
| :--- | :--- | :--- | :--- |
| **18650 Li-ion Cells** | 2 | 3.7V Rechargeable | Connect in series (2S) for 7.4V. |
| **2S BMS Board** | 1 | 2S 8.4V Protection | Essential for battery safety. |
| **TP5100 Module** | 1 | 2S Li-ion Charger | Accepts 9V-15V input. Solder "SET" for 2S. |
| **LM2596 Buck Module** | 1 | DC-DC Step Down | Set to exactly 5.0V. |
| **12V DC Jack** | 1 | Switchcraft 712A | Panel mount, 2.1mm center positive. |
| **IRF4905 MOSFET** | 1 | P-Channel MOSFET | For Power Path (Battery disconnect on plug). |
| **1N5822 Diode** | 2 | 3A Schottky Diode | For reverse polarity and load sharing. |
| **10µH Inductor** | 2 | Bourns RLB Power Choke | [652-RLB0914-100KL] - For Pi-Filter. |
| **470µF Low-ESR Cap**| 4 | Panasonic FR Series | [667-EEU-FR1C471] - Power filtering. |

## 4. MIDI Interface (Isolated)
| Component | Qty | Description | Notes |
| :--- | :--- | :--- | :--- |
| **6N138** | 1 | High Speed Optocoupler | [649-6N138] - Lite-On. |
| **5-Pin DIN Jack** | 1 | Switchcraft 57GB5FX | [502-57GB5FX] - Standard MIDI Panel Mount. |
| **1N4148 Diode** | 5 | Signal Diode | Protection diode for MIDI loop. |
| **220Ω Resistor** | 5 | 1/4W Metal Film | MIDI loop current limiting. |

## 5. User Interface
| Component | Qty | Description | Notes |
| :--- | :--- | :--- | :--- |
| **20x4 LCD Screen** | 1 | Character LCD (I2C) | Blue or Green backlight recommended. |
| **Rotary Encoder** | 1 | Bourns PEC11R w/ Switch | [652-PEC11R-4215F-S24] - Patch browsing. |
| **Rocker Switch** | 1 | SPST Power Switch | Main system on/off. |
| **Aluminum Knobs** | 2 | 6mm D-Shaft / Knurled | One for Volume, one for Encoder. |

## 6. Prototyping & Enclosure
| Component | Qty | Description | Notes |
| :--- | :--- | :--- | :--- |
| **Pico Perfboard** | 2 | BusBoard BR1 | [854-BR1] - Exact breadboard layout. |
| **Machined Headers** | 5 | 40-pin Male/Female | [649-10129381-940LF] - For modular Pico/OPL2. |
| **Brass Standoffs** | 1 | M3 Kit (6mm/10mm) | Mounting boards inside enclosure. |
| **Heat Shrink Tubing**| 1 | Assorted pack | For insulating 18650/Switch wiring. |

---

### 🛒 Ordering Notes:
1.  **Mouser Canada:** All part numbers above (e.g., [652-...]) are confirmed shippable to Canada and RoHS compliant.
2.  **The Pi-Filter:** You need **two** 470µF caps and **one** 10µH inductor *per* power rail you want to clean (recommend one for the whole system after the buck converter).
3.  **The Class-A Mod:** Ensure you have **4.7kΩ resistors** to pull the NE5532P outputs to Ground to eliminate crossover distortion.

**Next step:** Once these arrive, the first build objective is the **Power Path**. Getting the 12V DC Jack, the TP5100 Charger, and the LM2596 working ensures you have "clean" 5V power before you attach the sensitive YM3812 silicon.
