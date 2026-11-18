# TPU Feed Assist ESP32 — prototype

## Credits — MacDat's original work

**Hardware concept inspired by [MacDat's “TPU Feed Assist Module Remote Controller” on MakerWorld](https://makerworld.com/en/models/2704030).** MacDat designed and published the original **external three-button remote-control solution** for the Bambu Lab TPU Feed Assist Module, including the SH1.0 button wiring/extension approach and an external remote enclosure. **Please visit the original project and credit MacDat when reproducing or adapting that hardware.**

This repository builds on **that hardware concept** by proposing an ESP32-S3 bridge, isolated electronic button actuation and remote software control. **The ESP32 firmware, MQTT/HTTP API and proposed inline sensing board here are our independent extensions, not features of MacDat's project.** MacDat is not presented as a contributor, maintainer or endorser of this repository.

**MacDat's 3D models, photos, instructions and any other source assets are NOT included or relicensed under MIT here.** Download original assets from MakerWorld and respect the license shown there; our MIT `LICENSE` covers only the original code and documentation in this repository. See [NOTICE.md](NOTICE.md) for attribution details.

**Unofficial ESP32-S3 accessory bridge for Bambu Lab TPU Feed Assist Module.**

**Target board:** [Seeed Studio XIAO ESP32-S3 (standard)](https://wiki.seeedstudio.com/xiao_esp32s3_getting_started/), powered separately via USB-C. PlatformIO target: `seeed_xiao_esp32s3`. [Pin allocation and wiring](docs/XIAO_ESP32S3.md). The optional Sense camera/SD and Plus display add-ons are not part of this design.

The goal is remote control and **local-button event monitoring** using a MacDat-inspired SH1.0 inline adapter. Original buttons should remain electrically usable with the ESP32 unpowered. Firmware implements **remote button pulses**, and provides **optional debounced GPIO input monitoring** of physical keys. Input monitoring is disabled by default: its isolated sensing electronics still require hardware measurements and validation.

> **Experimental / no tested hardware**: this is prototype code. Do not attach the bridge to a printer until the button pinout, voltage, polarity, scan timing and PhotoMOS compatibility have been verified with a DMM or oscilloscope.

## Functions

| Feature | Status |
| --- | --- |
| ESP32-S3 + Wi-Fi | Prototype source |
| Isolated PhotoMOS FEED and FORCE press | Prototype source, not bench-validated |
| UNLOAD | Disabled by default; requires compile-time enable + physical arming input |
| Web UI / REST | Prototype source |
| MQTT events / status | Prototype source |
| Physical button sensing | Firmware input events implemented behind an OFF-by-default flag; sensing hardware NOT validated |
| Automatic AMS <-> external TPU material swaps | **Not supported** |
| Bambuddy integration | Proposed only |

## Hardware architecture

```text
Bambu PCB -- SH1.0 --> [ INLINE BOARD ] -- SH1.0 --> Original button
                             |
                             +-- PhotoMOS N/O contact, in parallel
                             +-- High impedance differential sense*
                                      |
                               isolated output --> XIAO ESP32-S3 --> MQTT / HTTP
```

*The passive button passthrough is independent of ESP32 power. The **sense front-end must be engineered only after measurements**. A pull-up or bare ESP32 GPIO across the unknown Bambu signal is not acceptable: it can falsely activate the key or damage electronics. An optocoupler wired directly across contacts also needs leakage/current calculations.

With an inline adapter, both physical-key wires remain passively connected even if ESP32 is unpowered. An alternative is to **disconnect the buttons from Bambu and route them directly to ESP32 inputs**, letting ESP32 regenerate presses via PhotoMOS; this is electrically simpler to read but breaks local controls whenever the ESP32 fails. We prefer the passive-through adapter for reliability.

For future physical-button sensing, an isolated active-low front end (not yet designed) can drive XIAO D0/GPIO1, D1/GPIO2, D9/GPIO8. Set `ENABLE_LOCAL_BUTTON_MONITORING=true` only after validating the sensor, never connect the original Bambu key wires to ESP pins. MQTT `event` will emit `local_press_observed_unverified` and `local_release_observed_unverified` for debounced changes; changes concurrent with remote pulses are suppressed. They are input observations, not proof the feeder responded.

The inline adapter concept is an **ESP32 extension of the original remote-button hardware approach by MacDat**. For the original mechanical design and wiring, consult [MacDat's MakerWorld project](https://makerworld.com/en/models/2704030). No MacDat printable files or artwork are redistributed here.

## Build

1. Install [PlatformIO](https://platformio.org/).
2. Copy `include/config.example.h` to `include/config.h` and set Wi-Fi, HTTP, MQTT secrets (never commit the copy).
3. `pio run -e xiao_esp32s3` and `pio run -e xiao_esp32s3 -t upload`.
4. Test PhotoMOS switching *off-printer* before making connections.

`GET /api/v1/status` returns **bridge status only**, not actual motor, toolhead, or filament state.
`POST /api/v1/press` accepts `{"button":"feed","request_id":"unique-id"}` and requires HTTP Basic or X-API-Key authentication. The MQTT equivalent is `tpu-feed-assist/<DEVICE_ID>/cmd/press`.

See [docs/HARDWARE_TEST_PLAN.md](docs/HARDWARE_TEST_PLAN.md) and [docs/BAMBUDDY_INTEGRATION.md](docs/BAMBUDDY_INTEGRATION.md).

## Scope and safety

Bambu's official TPU Feed Assist workflow does not support AMS-like automatic load/unload. This bridge does not create an additional AMS slot or implement automatic multi-material X1C/AMS + external TPU switching. The external spool path and toolhead require separate mechanical engineering, filament sensors and printer-state coordination.

**Never operate unattended material changes based solely on a successful PhotoMOS pulse.**

Our original firmware and documentation are MIT-licensed. MacDat's project remains MacDat's work under its own MakerWorld license; Bambu Lab, MacDat and Bambuddy are independent third parties and have not endorsed this project.