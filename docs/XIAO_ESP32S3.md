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

- Power the XIAO from its own **5 V USB-C supply**. The Bambu Bus stays dedicated to the original TPU Feed Assist Module; **do not wire Bambu Bus 5 V, any unknown voltage, or Bambu switch-line supply into XIAO pins**.
- Keep Bambu button-side electrical contacts galvanically isolated from XIAO ground. PhotoMOS output contacts go **across** original button terminals only **after measurement**; the PhotoMOS input LED is driven from XIAO's GPIO + series resistor.
- Original physical buttons are **passively wired through the SH1.0 inline adapter** and continue to work with XIAO unpowered.
- Put suitable external **OFF-state biasing** on PhotoMOS LED driver circuitry so boot/reset/high-impedance ESP GPIOs cannot accidentally press a key. `INPUT_PULLUP` is for an **isolated sensor transistor only**, not original button terminals.
- No direct connection between XIAO GPIO or GND and unknown Bambu button conductors.
- Validate LED drive current, PhotoMOS ON resistance, off leakage, LED input threshold and Bambu switch voltage before wiring to any printer.

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
