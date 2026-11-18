# Integrating a TPU feeder in Bambuddy's SpoolBuddy Devices

**Goal:** show the XIAO ESP32-S3 TPU Feed Assist bridge under **Settings → SpoolBuddy Devices**, alongside RFID/scale stations, but as a separate **TPU feeder** capability. The hardware remote-controller inspiration is by [MacDat](https://makerworld.com/en/models/2704030); ESP32 firmware and software integration are independent extensions. See [NOTICE](../NOTICE.md).

## Upstream code inspection (18 November 2026)

Confirmed in the upstream [Bambuddy source](https://github.com/maziggy/bambuddy):

- `backend/app/api/routes/spoolbuddy.py`: `POST /spoolbuddy/devices/register`, `POST /spoolbuddy/devices/{device_id}/heartbeat`, and `GET /spoolbuddy/devices`; base app path is `/api/v1`.
- `backend/app/models/spoolbuddy_device.py`: the current table stores `device_id`, `hostname`, `ip_address`, `firmware_version`, `has_nfc`, `has_scale`, online state through `last_seen`, etc., **but no device kind/capabilities field**.
- `spoolbuddy/daemon/api_client.py`: SpoolBuddy sends `X-API-Key` with registration and heartbeats.
- `frontend/src/components/SpoolBuddySettings.tsx`: the current SpoolBuddy device cards include actions for **update, restart browser, restart daemon, reboot, shutdown**. Those are targeted at Raspberry Pi/Linux and cannot be reused unchanged for ESP32.

**Important:** the existence of a registration endpoint does **not** mean the ESP32 feeder is already supported. Registering with `has_nfc=false, has_scale=false` might make the XIAO appear online but would misleadingly show NFC/scale warnings and unsupported Linux/SSH actions. We will NOT emulate a Pi daemon or publish false hardware data.

## Upstream change proposal

1. Add additive, backwards-compatible `device_type` field to registration, database, heartbeat, and JSON response:
   - Existing Pi stations default to `spoolbuddy_station`.
   - ESP32 feeder uses `tpu_feed_assist`.
   - Future ESP32 readers could use `spoolreader`, after confirming their actual protocol.
2. Support a `capabilities` list or JSON object: `feed_button`, `force_button`, `unload_button`, `local_button_monitor`, `ota`, `firmware_version`. Never infer real motor feedback merely from button pulses.
3. Render feeder cards under **SpoolBuddy Devices** with Wi-Fi/IP/firmware/last seen and manual FEED/FORCE controls; UNLOAD must remain locked until independent toolhead/filament and physical arming interlocks are validated.
4. Add explicit `POST /api/v1/spoolbuddy/devices/{id}/feeder/press` (proposal, not existing upstream endpoint); queue to firmware through safe authenticated HTTP or a separate MQTT command adapter. Include `request_id` and TTL, and no retained MQTT commands. Report `pulse_complete_unverified` only.
5. Use existing `POST /devices/register` and `/heartbeat` extended by `device_type` for discovery and device online/offline after upstream agrees; leave legacy SpoolBuddy backward compatible.

## XIAO firmware configuration

A public GitHub repo must NEVER contain real Wi-Fi passwords, Bambuddy API keys or MQTT credentials.

- First boot: XIAO AP setup page on `192.168.4.1`, configured with a network scan and SSID/password.
- Onboarding: form for **Bambuddy URL and API key** (stored on device in NVS/Preferences) plus optional MQTT broker settings.
- These credentials do not imply native integration until the upstream API supports the feeder type.
- After onboarding: join the same routable LAN/VLAN as Bambuddy and the X1C and expose authenticated management.
- Physical buttons remain passive-through, independent of network connectivity.
- OTA only with credentials and operator-verified printer idle.

## Why not expose as an AMS slot?

SpoolBuddy is Bambuddy's external hardware management area; the X1C AMS protocol has its **own** filament selection and toolhead state. The TPU Feed Assist button bridge is not an AMS, does not perform filament cutting, and cannot automatically swap TPU with existing AMS material. Separate motor/filament sensor feedback and mechanical path switching are still required.

## Suggested upstream discussion

Would the maintainers accept adding an optional `device_type=tpu_feed_assist` and feature-specific device actions to the **existing SpoolBuddy Devices** UI/API, preserving the original Pi station workflow? This gives the XIAO one registration/heartbeat path and provides a reusable extension point for future ESP32 peripheral types. A contributor PR can follow after physical bench tests and explicit validation of the XIAO GPIO hardware.

Related local [integration issue](https://github.com/Nicco-Male/TPU-Feed-Assist-ESP32/issues/2).
