# Integrated power — XIAO ESP32-S3 from TPU Feed Assist Bambu Bus

**Status: design proposal — NOT electrically validated.** The original Bambu Lab TPU Feed Assist Module uses the Bambu Bus for power. The standard AMS/X1 bus is described as a **nominal 24 V DC** supply. Do not assume the voltage or connector polarity until measured on the **actual TPU Feed Assist assembly**. This design draws **only power**, never touches Bambu Bus data/RS-485 signals.

The original physical remote-button architecture and SH1.0 cable concept were published by **[MacDat on MakerWorld](https://makerworld.com/en/models/2704030)**. This 24 V supply integration is our proposed ESP32 extension, not MacDat's design. See [NOTICE.md](../NOTICE.md).

## Proposed installation

```text
X1C / AMS Bus 6-pin adapter -> original TPU Feed Assist 4-pin feed
                                            |
                    passive 4-pin inline pass-through harness*
                                            |
                                     Bambu feeder PCB
                                            |
                  verified 24 V / GND tap before motor driver
                                            |
                              fuse + transient protection
                                            |
                           DC/DC buck, 24 V DC -> 5 V DC
                                            |
                                Schottky/ideal diode
                                            |
                                 XIAO 5V pin + GND

XIAO D3/D4/D5 -> PhotoMOS isolated switch contacts -> FEED/UNLOAD/FORCE keys
Original physical keys remain connected passively through SH1.0 adapters.
```

*Prefer an accessible in-line **harness adapter** or dedicated board with exactly matched rated connectors, rather than cutting, soldering or guessing pads on the Bambu PCB. Locate the 24 V rail using the exact feeder assembly, official schematic if available, and a multimeter. Do not infer 4-pin/6-pin orientation from internet pinout drawings.

## Physical connection: 4-pin BUS male/female interposer

**Recommended for this project:** do **not** cut the original Bambu Bus cable, and do not draw power from the Hall/tension sensor wiring. Use a short **male-to-female 4-pin pass-through interposer** between the original TPU Feed Assist Bambu Bus cable and the feeder's 4-pin power input, with a small tap for **only** bus +24 V and GND.

```text
Original 4-pin Bambu Bus cable
             |
    [4-pin matching socket]
       1:1 pass-through PCB <--- exactly same pin-to-pin mapping (verify)
    [4-pin matching plug]
             |
    Original 4-pin input on TPU Feed Assist
             |
     [feeder PCB untouched]

From interposer's MEASURED +24V / GND pads ONLY:
  +24V -> branch fuse/protection -> DC/DC 24->5V -> diode/power path -> XIAO 5V
   GND -> DC/DC input return -> XIAO GND if using NON-isolated buck
  Bambu BUS differential data pair A/B -> **straight-through only**;
  no connection to XIAO or DC/DC.
```

Connector family references for standard X1 Bambu 4-pin Bus: [Bambu Power Tap 446](https://www.printables.com/model/468472-x1c-power-tap-446), [Power Tap DIY](https://www.printables.com/model/473070-bambu-power-tap-diy) and [Molex Micro-Fit 3.0 discussion](https://forum.bambulab.com/t/tap-into-ams-power-for-small-fans/115907). Treat these as **compatible design examples**, not certified pinout / mechanical compatibility with every TPU Feed Assist PCB revision. Check mating gender, keyed orientation, latch, continuity and actual supply polarity before ordering or energizing. Never assign +24/GND based on wire color alone.

This interposer can be placed **inside an expanded printed feeder housing**, potentially with XIAO and DC/DC on the same small carrier. The external power cable stays unchanged. It must not strain the feeder's input socket.

### Could we use the Hall tension-sensor harness instead?

The official guide calls it the **Hall board**, attached to the main feeder PCB by a separate cable ([Bambu guide mirror, Step 6](https://p3d.mx/products/tpu-feed-assist-module-for-bambu-lab)). Public assembly instructions do **not provide a verified pinout, voltage, or spare current capacity** for that connector. It might contain a 3.3/5 V logic rail, or it may be a scanned/digital/analogue sensing circuit that must not be loaded.

Even if a DMM measures 3.3/5 V at that connector, that does **not prove it can supply the Wi-Fi current bursts of a XIAO ESP32-S3**. Its sensor supply might be filtered/low-current and extra loading can corrupt tension feedback or reset the feeder. Thus **sensor harness = potential signal observation after study, NOT chosen power source**.

A different option is taking +24 V/GND at measured input-side solder pads on the feeder PCB, but that is less reversible and requires modifying the original Bambu PCB. The inline bus interposer is the preferred no-cut/no-solder-originals option.

## Design targets (not a tested BOM)

- **Input**: Bambu Bus nominal 24 V DC; verify polarity, power-up transients and available headroom with X1C + two AMS units + AMS Hub + feed module.
- **Protection**: fused *branch* sized based on actual inrush/current, correct polarity protection, input filtering and an appropriately chosen TVS where needed. Do not replace Bambu's bus protection.
- **Converter**: buck regulator with input voltage range covering 24 V + measured transients (for example, 9–36 V or wider after reviewing transient ratings) and **5.0 V regulated output**, with **>= 0.5 A peak output** (1 A headroom preferred) for XIAO Wi-Fi bursts. The total *extra load* is only authorized after proving the bus has capacity.
- **XIAO input**: 5 V through a diode or properly designed power-path / ideal-diode circuit to the **5V** pad, GND to GND. See [Seeed's XIAO ESP32-S3 guide](https://wiki.seeedstudio.com/xiao_esp32s3_getting_started/) concerning externally driving the 5V pin. Do not connect 24 V to any ESP32 or XIAO pin, and do not feed the **3V3** output.
- **USB debug**: protect against reverse backfeed into the DC/DC supply when a programming USB cable is plugged in. The protection and pin arrangement must be verified against the actual XIAO hardware revision. Keep the printer switched off/disconnected during initial USB testing.
- **Fail-safe**: opening, disconnecting or failing the added power branch must NOT interrupt power to the original feeder, original buttons or Bambu Bus communication.
- **Noise**: keep the 24->5 V regulator physically separated from antenna and motor wiring; verify 5 V rail stability during motor activity and Wi-Fi transmission.

## Electrical isolation choices

### A. Non-isolated buck (smaller/cheaper)

The 5 V return is **electrically common** with Bambu's 24 V supply ground. The PhotoMOS switch **contacts still isolate the signal terminals** from their LED input, but the *whole ESP32 and Bambu system are not galvanically isolated*. Do not claim otherwise. The proposed sensing circuit must be checked for ground loops and interaction with button scans.

### B. Isolated 24 V -> 5 V DC/DC (preferred when full isolation matters)

A certified/appropriately rated isolated DC/DC module keeps XIAO ground **floating** relative to Bambu Bus ground, preserving galvanic separation across the whole controller (provided there are no other shared ground connections). Select adequate output current and account for module size, load and heat. PhotoMOS sensing and driving remain independent.

The **exact** regulator, fuse and isolation scheme will be selected **after measurements**, not guessed from connector colors.

## Validation gates

1. Record nominal 24 V and GND pin locations on the actual feeder power connector, with suitable electrical test equipment, **without shorting adjacent pins**.
2. Measure the feeder's motor current and 24 V supply drop during operation; test X1C + both AMS units for power margin. **24 V 3 A is a connector/Hub rating, not necessarily unused extra capacity**.
3. Bench-test the buck/isolated DC/DC with a dummy load, including turn-on, Wi-Fi peak load and output ripple.
4. Confirm XIAO is powered from the adapter, correctly boots and has stable Wi-Fi during motor activity; original module and buttons still function if added branch is off.
5. Confirm USB programming does not backfeed 5 V into either the adapter or printer's power rail.
6. Only then install into an appropriately ventilated, strain-relieved housing; verify insulation, cable retention, warmth and EMI behavior.

No printer integration is approved merely because these instructions exist.

## References

- [Official Bambu Lab AMS Hub specs](https://bambulab-eu.myshopify.com/it/products/ams-hub) (24 V nominal input, 3 A rating)
- [Seeed XIAO ESP32-S3 — power pins](https://wiki.seeedstudio.com/xiao_esp32s3_getting_started/)
- [MacDat — original TPU Feed Assist external button remote](https://makerworld.com/en/models/2704030)
