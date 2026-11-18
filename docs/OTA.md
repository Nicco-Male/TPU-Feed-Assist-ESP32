# OTA firmware updates — Seeed XIAO ESP32-S3

Starting with firmware `0.2.0-prototype`, the XIAO can receive future updates over **Wi-Fi**, so the integrated Bambu Bus 24 V -> 5 V design does not require reconnecting a USB-C programming cable after initial commissioning.

**This is not a commercial / safety-certified device.** The firmware cannot currently detect whether the X1C is printing. Do not initiate OTA while the printer, feeder, or filament path is active. An update reboots the MCU and temporarily stops remote monitoring and command output. The original pass-through physical buttons still work without ESP32 power.

## Initial setup (one-time USB flash)

1. Copy `include/config.example.h` to `include/config.h` and enter credentials; `config.h` is gitignored.
2. On first boot use the `TPU-Feed-XXXX` setup hotspot at `http://192.168.4.1`. Enter a **new, unique 16+ character** OTA password and choose `OTA enable = 1`. Both values are stored in NVS, not compiled into a public firmware. See [PROVISIONING.md](PROVISIONING.md).
3. With the controller connected to **USB-C on the workbench** and **no Bambu feeder attached**, flash from PlatformIO:
   ```sh
   pio run -e xiao_esp32s3 -t upload
   pio device monitor -b 115200
   ```
4. Check serial output for `Arduino OTA enabled on trusted LAN` and obtain the XIAO IP address from DHCP leases.

The OTA listener is disabled unless the portal's OTA flag is enabled and a 16+ character password has been saved.

## Subsequent Wi-Fi updates

- Ensure **printer and feeder are fully idle**. This is a mandatory human check until Bambuddy state gating is developed.
- Build the new image:
  ```sh
  pio run -e xiao_esp32s3
  ```
- Upload `.pio/build/xiao_esp32s3/firmware.bin` using the standard Arduino ESP32 `espota.py` utility or Arduino IDE network upload, on **TCP port 3232**, using your OTA password.

Example when `espota.py` is available on PATH or at the appropriate Arduino ESP32 tools directory:

```sh
python espota.py -i XIAO_IP -p 3232 -a 'YOUR_LOCAL_OTA_PASSWORD' -f .pio/build/xiao_esp32s3/firmware.bin
```

This command can expose the password to local shell history or process inspection. Prefer a local credential-handling workflow and never store it in a tracked script. The OTA service is **not TLS**; use it on a trusted LAN only, never expose or port-forward it to the internet. Remote management should occur via an authenticated VPN.

## Confirmation

- The bridge reports `"version": "0.3.0-prototype"` and `"ota_enabled": true` via authenticated `GET /api/v1/status`.
- `ota_in_progress` indicates that an update has started. All simulated button outputs are forced off when OTA begins.
- The MQTT `availability` topic will briefly go offline during reboot and return online.

## Known limits

- Printer-state safety interlocking is **not implemented**. OTA being enabled is not permission to update during printing.
- `ota_in_progress` only affects ESP32-side commands; it is not filament-motion feedback.
- Existing native Bambu buttons remain connected via the intended passive SH1.0 feed-through hardware; that hardware has not yet been validated.
- The integrated Bambu Bus power circuit remains a design proposal; bench-test the XIAO with USB power before wiring it to the feeder.
