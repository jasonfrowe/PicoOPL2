### 1. Updated PARTS.md
Add this to your existing list. I've selected a standard size that is easy to panel-mount.

| Component | Qty | Description | Notes |
| :--- | :--- | :--- | :--- |
| **Rocker Switch** | 1 | SPST (On-Off) Snap-in | [Mouser: 691-R13-112A-02-BB] or any standard 2-pin rocker. |
| **P-Channel MOSFET** | 1 | IRLML6402 or IRF4905 | For the Power Path (Battery disconnect). |
| **Schottky Diode** | 1 | 1N5822 (3A) | To prevent back-feeding the DC Jack from the battery. |
| **100kΩ Resistor** | 1 | Pull-down Resistor | For the MOSFET Gate. |

---

### 2. Updated WIRING.md (Power Section)
The switch is placed **after** the power selection logic but **before** the Buck Converter. This ensures the batteries can charge even when the synth is "OFF," but the Buck Converter won't drain your batteries when the switch is flipped.

#### A. Integrated Power Path & Switch Logic
1.  **DC Jack (12V Input):**
    *   **Positive (+):** Connects to **TP5100 VIN+** AND the **Anode** of the 1N5822 Diode.
2.  **Power Selection (Automatic Switch):**
    *   **1N5822 Cathode:** Connects to **Terminal A** of your Rocker Switch.
    *   **IRF4905 MOSFET Drain (Pin 3):** ALSO connects to **Terminal A** of your Rocker Switch.
3.  **The Master Switch:**
    *   **Terminal B** of your Rocker Switch: Connects to the **LM2596 Input+**.
4.  **Battery Path Control:**
    *   **IRF4905 Source (Pin 2):** Connects to **BMS P+** (Battery Positive).
    *   **IRF4905 Gate (Pin 1):** Connects to the **DC Jack Positive (+)** (upstream of the diode).
    *   **100kΩ Resistor:** Connects from **IRF4905 Gate (Pin 1)** to **Ground**.

> **The Result:** 
> *   **If Plugged In:** 12V hits the MOSFET Gate, turning the battery path **OFF**. The synth runs on 12V.
> *   **If Unplugged:** The 100k resistor pulls the Gate to Ground, turning the battery path **ON**.
> *   **The Switch:** In both cases, the Rocker Switch acts as a "Gatekeeper" for the Buck Converter. Flipping it to OFF stops all current to the Pico/OPL2/Amp, but the TP5100 charger stays connected to the batteries for charging.

---

### 3. Understanding the 9V vs 12V Choice
You mentioned having a **9V DC Jack**. While 9V *can* work, **12V is highly recommended** for this specific 2S (8.4V) battery setup:
*   **Charging Headroom:** The TP5100 charger needs the input voltage to be at least 1-2V higher than the battery voltage (8.4V) to charge efficiently. 9V is "on the edge" and might not fully charge the batteries.
*   **Voltage Drops:** The 1N5822 diode and the wires cause small voltage drops. Starting with 12V ensures that even after those drops, your Buck Converter has plenty of "pressure" to create a clean 5V output for the audio amp.

---

### 4. Audio Path Upgrade: NE5532P
You asked if the **NE5532P** replaces all LM358s. **Yes.**
*   **Buffer/Filter Stage:** Use one NE5532P (Dual) for the OPL2 output buffer.
*   **Mixer Stage:** Use the second NE5532P (Dual) for the Line-In mixing and final gain stage.
*   **Headroom:** Since the NE5532P is not "Rail-to-Rail," it won't work well if your battery drops too low. However, because you are using a **Buck Converter** to keep the rail at a steady **5V**, the NE5532P will perform beautifully as long as your audio signal stays centered at the **2.5V Virtual Ground**.

### 5. Final Hardware "Gotcha" - Quiescent Current
Even with the switch OFF, the **BMS** and the **TP5100** (when unplugged) will draw a tiny, microscopic amount of current from the battery (microamps). 
*   **Storage:** If you plan to leave the synth in a closet for 6 months, it's best to physically disconnect the batteries or add a "Kill Switch" directly on the battery line. 
*   **Daily Use:** For everyday use, the current setup is perfect—just like a modern laptop or a boutique synth.

**Next Step:** You have the full bill of materials and the wiring logic. Once the parts arrive, start by building the **Power Path** on a piece of perfboard and verify you get a clean 5V from the Buck Converter before plugging in the expensive OPL2 chip!


---

# 🎹 Pico OPL2 Synth: Master Wiring Guide

## 1. Power Path & Charging System
This system allows the synth to run off a **12V DC Jack**, charge the **2S Battery**, and automatically switch to battery power when unplugged without a reboot.

### A. DC Input & Charging
*   **DC Jack (Center +):** Connect to **TP5100 VIN+** AND the **Anode** of a 1N5822 Schottky Diode.
*   **DC Jack (GND):** Connect to **Common Ground Plane**.
*   **TP5100 "SET" Pads:** Solder these together (Short) to enable **2S (8.4V) Mode**.
*   **TP5100 BAT+:** Connect to **BMS P+** (Battery Positive).
*   **TP5100 GND:** Connect to **Common Ground Plane**.

### B. Power Path (Automatic Switching)
This circuit uses a P-Channel MOSFET as a "gatekeeper" for the battery.
*   **IRF4905 MOSFET Source (Pin 2/Tab):** Connect to **BMS P+** (Battery Positive).
*   **IRF4905 MOSFET Drain (Pin 3):** Connect to the **LM2596 Input+**.
*   **IRF4905 MOSFET Gate (Pin 1):** Connect to the **12V DC Jack (Center +)**.
*   **Gate Resistor:** Connect a **100kΩ resistor** from **Gate (Pin 1)** to **Ground**.
*   **Blocking Diode:** The 1N5822 Schottky Diode (from step A) cathode goes to **LM2596 Input+**.

> **How it works:** When 12V is plugged in, the MOSFET Gate goes HIGH, turning the battery path OFF. The 12V flows through the diode to the buck converter. When unplugged, the 100k resistor pulls the Gate LOW, turning the battery path ON instantly.

---

## 2. Main Logic & Control (Pico)

### A. OPL2 Data Bus (via 74LVC245)
*   **Pico GP0–GP7:** Connect to **74LVC245 Pins A1–A8**.
*   **74LVC245 Pins B1–B8:** Connect to **YM3812 Pins 10, 11, 13, 14, 15, 16, 17, 18** (D0–D7).
*   **74LVC245 Pin 1 (DIR):** Tie to **3.3V**.
*   **74LVC245 Pin 19 (OE):** Tie to **GND**.
*   **74LVC245 Pin 20 (VCC):** Tie to **3.3V**.

### B. OPL2 Control Lines
*   **GP8:** **YM3812 Pin 4 (A0)**
*   **GP9:** **YM3812 Pin 5 (WR)**
*   **GP10:** **YM3812 Pin 7 (CS)**
*   **GP11:** **YM3812 Pin 3 (IC/Reset)**
*   **GP21:** **YM3812 Pin 24 (phiM - Clock In)**

### C. MIDI & UI
*   **GP1 (UART0 RX):** Connect to **6N138 Pin 6** (with 4.7kΩ pull-up to 3.3V).
*   **I2C0 SDA/SCL (GP12/GP13):** Connect to **20x4 LCD Backpack**.
*   **Rotary Encoder A/B:** Connect to **GP14 / GP15**.
*   **Rotary Encoder Button:** Connect to **GP16**.

---

## 3. High-Fidelity Audio (NE5532P)
The **NE5532P** replaces the LM358 for superior audio. We use it as a **Summing Mixer** to allow a "Line-In" for other gear.

### A. Virtual Ground (VREF)
*   Create a 2.5V reference using two **10kΩ resistors** in series from 5V to GND.
*   Connect the center tap to **NE5532P Pin 3 and Pin 5** (Non-inverting inputs).
*   Add a **10µF cap** from this tap to Ground for stability.

### B. Mixer & Line-In (IC 1)
*   **OPL2 Audio (YM3014B Pin 2):** Connect via **10kΩ resistor** to **NE5532P Pin 2**.
*   **Line-In (Left/Right):** Connect via **10kΩ resistor** to **NE5532P Pin 2**.
*   **Feedback:** Connect a **10kΩ resistor** between **NE5532P Pin 1 and Pin 2**.
*   **Class-A Bias:** Connect a **4.7kΩ resistor** from **Pin 1 to Ground**.

### C. Output to Amp
*   **NE5532P Pin 1:** Connect to **(+) 10µF Muse Cap (-)** $\rightarrow$ **Volume Pot Pin 3**.
*   **Volume Pot Pin 2 (Wiper):** Connect to **PAM8406 Input**.
*   **Volume Pot Pin 1:** Connect to **Audio Ground**.

---

## 4. MIDI Input (Optocoupler)
This circuit isolates your Pico from the electrical noise of the keyboard.

*   **MIDI Jack Pin 4:** Connect to **220Ω Resistor** $\rightarrow$ **6N138 Pin 2**.
*   **MIDI Jack Pin 5:** Connect to **6N138 Pin 3**.
*   **1N4148 Diode:** Connect between **6N138 Pin 2 (Anode)** and **Pin 3 (Cathode)**.
*   **6N138 Pin 8 (VCC):** Connect to **5V** (gives the chip more speed).
*   **6N138 Pin 5 (GND):** Connect to **Pico Ground**.
*   **6N138 Pin 6 (VOUT):** Connect to **Pico GP1** AND **4.7kΩ Resistor to 3.3V**.
*   **6N138 Pin 7:** Leave floating.

---

## 5. Summary Pin Layout (YM3812 Master)
*   **Pin 1:** **5V (VCC)**
*   **Pin 3:** **Reset (IC)** from Pico GP11
*   **Pin 4:** **A0** from Pico GP8
*   **Pin 5:** **Write (WR)** from Pico GP9
*   **Pin 6:** **Read (RD)** - Tie to **5V** (Disable)
*   **Pin 7:** **Chip Select (CS)** from Pico GP10
*   **Pin 10–18:** **Data Bus D0-D7** (Skipping Pin 12 GND)
*   **Pin 12:** **Ground**
*   **Pin 20:** **NC (No Connection)**
*   **Pin 21:** **MO (Data out)** to YM3014B Pin 4
*   **Pin 22:** **SH (S&H out)** to YM3014B Pin 3
*   **Pin 23:** **phiSY (Clock out)** to YM3014B Pin 5
*   **Pin 24:** **phiM (Master Clock In)** from Pico GP21

---

### Final Hardware Pro-Tips:
1.  **Star Ground:** Connect the GND of the PAM8406, the OPL2, and the Buck Converter to a **single central point** on your board.
2.  **The Pi-Filter:** Place the **10µH Inductor** and **470µF Capacitors** immediately after the LM2596 output. All 5V logic and audio should pull power from *after* this filter.
3.  **Twisted Pairs:** Twist the wires going from your Volume Pot to the PAM8406 and from the PAM8406 to the speakers to reduce EMI interference.
