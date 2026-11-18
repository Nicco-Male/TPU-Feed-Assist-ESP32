# Proposed Bambuddy accessory integration

Upstream: https://github.com/maziggy/bambuddy

**Original hardware inspiration:** [MacDat's TPU Feed Assist Module Remote Controller](https://makerworld.com/en/models/2704030). The SH1.0 external-button concept is MacDat's original work. The ESP32 protocol and integration proposal are independent extensions, not MacDat's firmware or an endorsement. See [NOTICE.md](../NOTICE.md).

## Intended device

- Printer: Bambu Lab X1 Carbon
- Existing Bambu AMS Hub with two AMS units, eight slots
- Separate external TPU spool via TPU Feed Assist Module
- ESP32-S3 bridge driven through an isolated PhotoMOS interface

## HTTP protocol (prototype)

- `GET /api/v1/status`: GPIO command/pulse status, connectivity, NOT motor or filament state.
- `POST /api/v1/press`: `{"button":"feed","request_id":"unique-id"}` (also `force`; `unload` gated)
- Header: `X-API-Key`, or HTTP Basic within trusted LAN/VPN.

Status MUST NOT claim physical success after a pulse: `pulse_complete_unverified` is just the output opening.

## MQTT

- `tpu-feed-assist/<device-id>/availability`: online/offline, retained/LWT
- `.../status`: retained device status
- `.../event`: unretained command results
- `.../cmd/press`: unretained JSON command with `button`, `request_id`, `ts` Unix UTC seconds

## Proposed Bambuddy UI

Add optional **external TPU feeder accessory** per printer (not a virtual AMS slot). Show device availability and FEED/FORCE buttons; UNLOAD disabled unless armed. Track request IDs to distinguish confirmed command delivery from filament movement. Add a later **physical-button event** integration only after electrical sensing and firmware events exist. Local key events should not be confused with remote commands.

## Explicit non-goals

- Do not trigger AMS/extruder/toolhead commands based solely on a button event.
- Do not claim automatic AMS <-> TPU multimaterial printing.
- Do not use bridge events as proof of filament presence / retraction / pressure.
- Do not enable fully autonomous unload without toolhead state feedback, filament sensors, mechanical switching and verified safety interlocks.

## Upstream questions

1. Would Bambuddy accept external accessory adapters (HTTP or MQTT)?
2. How should access control and per-printer accessory state be represented?
3. Is there a plugin extension point preferable to a core patch?

We welcome review after bench testing. A feature request to Bambuddy should link here, with observed hardware measurements and a published firmware release.
