# Inline board design — local physical buttons + remote ESP32

**Design-stage specification, NOT a verified schematic.** This follows the user request to read original MacDat/Bambu physical keys **without cutting or splicing wires**.

## Connection

For EACH original two-wire pushbutton:

- Plug the Bambu PCB's 2-pin switch header into an inline male/female SH1.0 cable/PCB adapter.
- Route both conductors straight through from Bambu PCB to original button. Their continuity must not depend on ESP32 supply, boot or firmware.
- Place one **normally open isolated PhotoMOS output contact** in parallel across those exact two switch wires, after characterizing the circuit.
- **Optionally** sample the differential switch-terminal voltage with a properly designed **high-input-impedance isolated sensing front end** to infer physical closure.
- The ESP32 drives the isolated PhotoMOS input LED, not the button terminals.
- For remote-vs-local classification, firmware must correlate sensing pulses with its own actively commanded output. A local-press event is only valid when the remote output is off.

```text
 Bambu switch header              inline PCB                     Bambu original key
  contact A  --------------------+------------------------------------ A
                                 |
                           PhotoMOS contact
                                 |  [ONLY after measurements]
  contact B  --------------------+------------------------------------ B
              |                  |
              +--- high-Z differential sensing (isolated, proposed) ---> ESP GPIO
```

**No Y cable is required**: the PCB is in series *mechanically* with a passive straight-through **electrical** connection. No Bambu wires are cut. The sensing path must NEVER apply a GPIO voltage, ESP ground or pull-up directly to unknown Bambu PCB lines.

## Why NOT a simple optocoupler across the key?

An optocoupler LED + resistor between the two key terminals draws current while the key is open and **may look like a half-pressed key** to the original MCU. Similarly a direct ESP32 GPIO can damage the board or create ground loops. Some key scanning circuits are pulsed or multiplexed, so 'closed=0 V' needs verification.

## Measurements needed before implementing sensing

1. With printer **off/unpowered**: verify two pins go only to an isolated normally-open pushbutton, and record idle/pressed continuity (do not use resistance mode while energized).
2. With printer **powered**: measure the open-switch voltage magnitude and polarity to the corresponding Bambu return, and captured waveform if possible (a safe differential measurement / battery handheld DMM).
3. During manual key actuation, record the voltage waveform and any debounce/long-press semantics.
4. Characterize total leakage allowed across an open switch; select sensing front end with negligible loading and fail-open behavior.
5. Verify original buttons remain fully functional with the ESP32 **unpowered/disconnected**, and that remote pulses alone cannot falsely generate a reported *local* key event.

Only then assign GPIO inputs, circuit values, filtering and a documented board layout. The current firmware has **no button-state GPIO** and intentionally does **not fabricate physical key events**.

## Failure behavior

- Software/ESP crash: PhotoMOS outputs **OFF** (gate pull-down); Bambu original buttons remain connected through.
- Loss of Wi-Fi: physical buttons continue; remote commands unavailable.
- Remote pulse while local key is held: not yet interlocked; this must be detected/blocked before enabling physically unattended operations.
- UNLOAD remote operation is **disabled by default** and cannot be interpreted as successful filament withdrawal.
