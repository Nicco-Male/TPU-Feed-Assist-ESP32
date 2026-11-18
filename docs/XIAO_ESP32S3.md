# Seeed Studio XIAO ESP32-S3 — board choice and pin allocation

**Target:** [Seeed Studio XIAO ESP32-S3 (standard)](https://wiki.seeedstudio.com/xiao_esp32s3_getting_started/), **not** ESP32-S3 DevKitC. PlatformIO: `board = seeed_xiao_esp32s3`.

This is an **experimental design**. MacDat created the [original external button hardware](https://makerworld.com/en/models/2704030); our XIAO interface and sensor electronics are **independent proposed additions**. See [NOTICE.md](../NOTICE.md).

## Proposed XIAO wiring

| XIAO pin | ESP32-S3 GPIO | Direction | Purpose | Interface |
| --- | --- | --- | --- | --- |
| D3 | GPIO4 | OUT | FEED | PhotoMOS driver LED via a correctly selected series resistor + default-OFF driver |
| D4 | GPIO5 | OUT | UNLOAD | PhotoMOS driver LED; **firmware disabled** unless physical arming input |
| D5 | GPIO6 | OUT | FORCE | PhotoMOS driver LED |
| D0 | GPIO1 | IN | Original FEED button observation | **Isolated active-low sensor output**, untested, disabled |
| D1 | GPIO2 | IN | Original UNLOAD button observation | **Isolated active-low sensor output**, untested, disabled |
| D9 | GPIO8 | IN | Original FORCE button observation | **Isolated active-low sensor output**, untested, disabled |
| D8 | GPIO7 | IN | UNLOAD hardware arming | Separate switch to XIAO GND (active LOW), NOT connected to Bambu |
| D10 | GPIO9 | — | Spare | Future optional sensor |
| D2 | GPIO3 | — | **Reserved** | ESP32-S3 strap pin; do not use for this accessory |
| D6 / D7 | GPIO43 / GPIO44 | — | **Reserved** | UART0 TX/RX |

All D numbers are **XIAO silkscreen labels**. Firmware constants are **ESP32 native GPIO numbers**, not D numbers. This prevents the mistaken use of GPIO10, which is **not one of the standard XIAO's D0–D10 header signals**.

See Seeed's [official pin table](https://wiki.seeedstudio.com/xiao_esp32s3_getting_started/) and [PlatformIO board registry](https://docs.platformio.org/en/latest/boards/espressif32/seeed_xiao_esp32s3.html).

## Power and isolation

**Current intended final design:** draw power from the **nominal 24 V Bambu Bus rail** already feeding the TPU Feed Assist, use a protected DC/DC 24 V -> 5 V, then supply the XIAO's **5V pin through a diode or proper power-path circuit**. No new external USB supply is needed when fully validated. **This connection is NOT electrically tested** and the exact 24 V rail/polarity and available current must be measured on the actual unit, with the X1C, two AMS and AMS Hub installed. **Never put 24 V or any unknown switch voltage directly into a XIAO pin.**

Full harness/protection/regulator/isolation and acceptance criteria: [INTEGRATED_POWER.md](INTEGRATED_POWER.md).

- Original buttons remain **passively wired through** the SH1.0 inline adapters even with XIAO unpowered.
- The PhotoMOS outputs remain floating contacts across original button pairs; do not connect any original button conductor directly to an ESP GPIO.
- **Non-isolated buck means XIAO GND and Bambu supply GND are shared**; the system is NOT galvanically isolated as a whole. A correctly specified isolated DC/DC can preserve system-level galvanic separation.
- During initial electronics testing use USB-C with an off-printer bench setup. Before connecting programming USB alongside external 5 V, verify reverse-current/backfeed protection per [Seeed power documentation](https://wiki.seeedstudio.com/xiao_esp32s3_getting_started/).
- Put appropriate external default-OFF bias on the PhotoMOS LED drive circuitry so boot/reset/high-impedance GPIOs do not simulate a press.

## Build / upload

```sh
cp include/config.example.h include/config.h
# Edit credentials in include/config.h (never commit it)
pio run -e xiao_esp32s3
pio run -e xiao_esp32s3 -t upload
pio device monitor -b 115200
```

This project targets the standard XIAO ESP32-S3. The **Sense variant** shares main header pin numbers but its optional microSD uses GPIO7/8/9 and conflicts with our selected pins if the card interface is active. The **Plus** exposes additional pins, but is not needed.

## Tests before connecting to a printer

1. Verify three PhotoMOS contacts are **open** with XIAO unpowered, during reset and at firmware startup, and only pulse when directly commanded.
2. Verify each XIAO output corresponds to the expected button using a **bench-only dummy load**, not the Bambu PCB.
3. Verify isolated sensor outputs pull only their assigned XIAO input LOW; no signal leakage onto the Bambu lines.
4. Measure the actual Bambu button voltage, polarity and timing using suitable instruments.
5. Confirm physical button bypass works with XIAO completely disconnected.

The firmware's local-button monitoring remains **OFF** by default. `ENABLE_UNLOAD_BUTTON=false` remains the default even if the arming switch is installed. See [HARDWARE_TEST_PLAN.md](HARDWARE_TEST_PLAN.md).
