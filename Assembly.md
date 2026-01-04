# 🎹 Pico OPL2 Synth: Master Wiring & Parts Guide

## 1. Integrated Power & Charging System
This system allows the synth to run off a **12V DC Jack**, charge the **2S Battery (8.4V)**, and automatically switch to battery power without a reboot.

### A. Parts List (Power Section)
| Component | Qty | Description | Notes |
| :--- | :--- | :--- | :--- |
| **Rocker Switch** | 1 | SPST (On-Off) | Master power for the Buck Converter. |
| **TP5100 Module** | 1 | 2S Li-ion Charger | Solder "SET" bridge for 8.4V mode. |
| **BMS Board** | 1 | 2S 18650 Protection | Essential for battery safety. |
| **LM2596 Buck** | 1 | DC-DC Step Down | Set output to **5.0V**. |
| **IRF4905** | 1 | P-Channel MOSFET | Battery "Gatekeeper" (Power Path). |
| **1N5822 Diode** | 1 | 3A Schottky | Prevents back-feeding the DC Jack. |
| **100kΩ Resistor** | 1 | Pull-down | For the MOSFET Gate. |

### B. Power Path & Switch Wiring
1.  **DC Jack (12V Input):**
    *   **Positive (+):** Connect to **TP5100 VIN+** AND **Anode** of 1N5822 Diode.
2.  **Power Selection Logic:**
    *   **1N5822 Cathode:** Connect to **Terminal A** of Rocker Switch.
    *   **IRF4905 Drain (Pin 3):** Connect to **Terminal A** of Rocker Switch.
3.  **The Master Switch:**
    *   **Terminal B** of Rocker Switch: Connect to **LM2596 Input+**.
4.  **Battery Path Control:**
    *   **IRF4905 Source (Pin 2):** Connect to **BMS P+** (Battery Positive).
    *   **IRF4905 Gate (Pin 1):** Connect to **DC Jack Positive (+)** (upstream of diode).
    *   **100kΩ Resistor:** Connect from **IRF4905 Gate** to **Ground**.

---

## 2. Main Logic Master Pinout (Pico)
This layout resolves all previous conflicts. **Note:** The Data Bus is shifted to **GP2–GP9** to keep **GP1** free for MIDI.

| Pico Pin | Function | Peripheral | Notes |
| :--- | :--- | :--- | :--- |
| **GP0** | **OPL2 A0** | YM3812 Pin 4 | Address/Data Select |
| **GP1** | **MIDI RX** | 6N138 Pin 6 | UART0 RX |
| **GP2–GP9** | **Data Bus** | YM3812 D0–D7 | **Shifted Bus (Bitmask: 0x3FC)** |
| **GP10** | **OPL2 WR** | YM3812 Pin 5 | Write Enable (Active Low) |
| **GP11** | **OPL2 CS** | YM3812 Pin 7 | Chip Select (Active Low) |
| **GP12** | **I2C0 SDA** | LCD Backpack | Use Level Shifter (BOB-12009) |
| **GP13** | **I2C0 SCL** | LCD Backpack | Use Level Shifter (BOB-12009) |
| **GP14** | **Encoder A** | Rotary Encoder | Phase A |
| **GP15** | **Encoder B** | Rotary Encoder | Phase B |
| **GP16** | **Encoder SW** | Rotary Encoder | Push Button Select |
| **GP17** | **OPL2 IC** | YM3812 Pin 3 | Initial Clear (Reset) |
| **GP21** | **OPL2 Clock** | YM3812 Pin 24 | 3.57MHz PWM Out |
| **GP30** | **RUN** | Reset Button | Connect to Button $\rightarrow$ GND |

---

## 3. OPL2 Hardware Interface (YM3812 & YM3014B)
Based on corrected Application Manual specs.

### A. YM3812 (OPL2)
*   **Pin 1:** **Vcc (+5V)**
*   **Pin 3:** **Reset (IC)** $\leftarrow$ Pico GP17
*   **Pin 4:** **A0** $\leftarrow$ Pico GP0
*   **Pin 5:** **WR** $\leftarrow$ Pico GP10
*   **Pin 6:** **RD** $\rightarrow$ **Tie to 5V** (Disable Read)
*   **Pin 7:** **CS** $\leftarrow$ Pico GP11
*   **Pin 10–18:** **Data Bus D0–D7** $\leftarrow$ Shifter $\leftarrow$ **Pico GP2–GP9**
*   **Pin 12:** **Vss (GND)**
*   **Pin 20:** **SH (S&H)** $\rightarrow$ YM3014B Pin 3
*   **Pin 21:** **MO (Data)** $\rightarrow$ YM3014B Pin 4
*   **Pin 22:** **SY (Sync)** $\rightarrow$ YM3014B Pin 5
*   **Pin 24:** **phiM (Clock)** $\leftarrow$ Pico GP21

### B. YM3014B (DAC)
*   **Pin 1:** **Vdd (+5V)**
*   **Pin 2:** **Buff (Audio)** $\rightarrow$ 1kΩ Resistor $\rightarrow$ NE5532P Pin 3
*   **Pin 7 & 8 (RB/MP):** **Jumper together** + **10µF Ceramic Cap to Ground**.

---

## 4. High-Fidelity Audio Chain (NE5532P)
Using two dual op-amps for buffering, mixing, and line-out.

1.  **Virtual Ground (VREF):** 10k/10k Resistor divider from 5V to GND. Output (2.5V) goes to NE5532P Non-inverting inputs.
2.  **The Mixer:** OPL2 audio and Line-In audio enter **NE5532P Pin 2** via **10kΩ resistors**.
3.  **The Class-A Mod:** Connect a **4.7kΩ resistor** from **NE5532P Pin 1 (Output)** to **Ground**.
4.  **The Line-Out:** Output from Pin 1 $\rightarrow$ **(+) 10µF Muse Cap (-)** $\rightarrow$ **10k/1k Voltage Divider**.
5.  **The Amp:** Tap signal from before the divider to feed the **Volume Pot** $\rightarrow$ **PAM8406**.

---

## 5. MIDI Input (Optocoupler Isolation)
*   **MIDI Jack Pin 4:** $\rightarrow$ 220Ω Resistor $\rightarrow$ **6N138 Pin 2**.
*   **MIDI Jack Pin 5:** $\rightarrow$ **6N138 Pin 3**.
*   **6N138 Pin 8:** **5V (Vcc)**.
*   **6N138 Pin 6:** **Pico GP1** AND **4.7kΩ Resistor to 3.3V**.
*   **6N138 Pin 5:** **Ground**.

---

### 💻 Software Implementation Tip:
Because the Data Bus now starts at **GP2**, you must shift your data in your C code:
```c
// Shift 8-bit OPL2 data to Pico Pins GP2-GP9
void write_bus(uint8_t val) {
    gpio_put_masked(0x3FC, (uint32_t)val << 2);
}
```