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
