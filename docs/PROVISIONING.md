# First boot: XIAO ESP32-S3 Wi-Fi/Bambuddy setup

**Firmware:** `v0.3.0-prototype`. The hardware concept for extending the three original buttons comes from [MacDat's original remote controller](https://makerworld.com/en/models/2704030). Our independent ESP32 controller is not an official Bambu or SpoolBuddy device.

## Setup without embedded passwords

1. Flash the public firmware via USB-C **on a workbench, not connected to the Bambu feeder**.
2. On first boot, look for Wi-Fi network `TPU-Feed-XXXX`, where XXXX is a suffix unique to the ESP32's chip ID. Connect using the temporary WPA2 passphrase **`TPUFeedSetup!`**.
3. Open **http://192.168.4.1** or follow your phone's captive portal popup. The page is provided by [tzapu/WiFiManager](https://github.com/tzapu/WiFiManager) (credit to its authors), not by Bambuddy.
4. Select the Wi-Fi SSID **reachable from the X1C and Bambuddy server** and enter the Wi-Fi password. Fill in:
   - **Bambuddy URL** (for example `http://<LAN-host>:8000`) and **Bambuddy API key**. Saved as placeholders for a **future typed SpoolBuddy-device integration**; entering them does NOT yet register a feeder in Bambuddy.
   - Optional local **MQTT host, port, username and password**. This prototype's preconfigured MQTT transport is **plaintext LAN MQTT** by default; do not point it to an Internet MQTT host or a broker requiring TLS until a tested CA-provisioning workflow is implemented.
   - **Web admin password** (12+ chars) and **HTTP API token** (16+ chars). Both are needed for a usable protected local management API. Secrets will not be auto-displayed after saving.
   - Optional **OTA password** (16+ chars) and `OTA enable = 1` to enable Wi-Fi updates after USB flashing.
5. Save Wi-Fi and server settings. The ESP32 connects to the requested network and stops the setup hotspot. Its IP can be found in your router's DHCP clients; access the authenticated `http://<xiao-ip>/` page to control FEED/FORCE only after hardware validation.

WiFiManager stores the station SSID/password in the ESP32 Wi-Fi subsystem; other fields are stored in `Preferences` NVS namespace `tpufeeder`. No real Wi-Fi, Bambuddy, MQTT, HTTP or OTA secrets belong in `include/config.example.h` or public Git history. The code may be compiled without secrets.

## Recovery and security limitations

- If Wi-Fi is unreachable after restart, WiFiManager reopens the setup hotspot.
- The **bootstrap hotspot password is shared and published**, not a per-device private secret. Anyone nearby who knows it could connect **while the setup hotspot is active**, see non-secret prefilled settings and attempt configuration changes. Run initial provisioning only in a trusted local environment. A future release should replace this with a unique device-bound bootstrap code and/or a physical setup button.
- Wi-Fi setup is HTTP inside the WPA2 access point. The regular management HTTP API is also plaintext (HTTP Basic or `X-API-Key`); **do not port-forward it**. Use a VPN for remote access.
- Existing secret fields are **not prefilled** in the portal. Leaving a secret blank preserves the current stored value.
- If no 12+ char admin password and no 16+ char HTTP token has been saved, web actions remain **unauthorized**. A first-time installer should set both.
- NVS is persistent flash storage and is **not encrypted** by this prototype. Anyone with sufficient physical access to the ESP32 may be able to extract it. Protect physical access and the Bambuddy API token; use a least-privilege key.
- Existing Bambuddy “SpoolBuddy Devices” currently assumes Raspberry Pi NFC/scale hardware; registration as a TPU feeder requires upstream `device_type` / capability-aware changes. See [SPOOLBUDDY_INTEGRATION.md](SPOOLBUDDY_INTEGRATION.md).
- The current module has **no automatic AMS/TPU material swapping, no verified filament feedback, no safe automated unload**.

## Firmware status

This is a firmware **prototype**, not a tested device. Validate access point, field persistence, protected HTTP endpoints, MQTT (if configured), OTA and GPIO behavior on the actual Seeed XIAO ESP32-S3 before connecting any Bambu hardware.
