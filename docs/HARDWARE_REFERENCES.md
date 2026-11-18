# Bambu TPU Feed Assist — reference photos and harness notes

**Original hardware photos by Bambu Lab, linked here rather than copied or relicensed.** Credit for the optional external-button hardware reference belongs to [MacDat](https://makerworld.com/en/models/2704030); see [NOTICE](../NOTICE.md).

## Verified assembly images

- **Main control PCB mounted on extruder** (Bambu Lab, assembly Step 7): [open photo](https://cdn.shopify.com/s/files/1/0759/4531/2569/files/360_20260320110305.png?v=1773975806)
- **Three physically separate button PCBs** (Bambu Lab, Step 3): [open photo](https://cdn.shopify.com/s/files/1/0759/4531/2569/files/360_20260320105243.png?v=1773975183)
- **Assembly and wiring guide** including the Hall board and button harness (Steps 4, 6, 10): [Bambu guide](https://wiki.bambulab.com/en/general/manual/tpu-feed-assist-module), [accessible guide reproduction](https://p3d.mx/products/tpu-feed-assist-module-for-bambu-lab).
- **Original external controller**: [MacDat on MakerWorld](https://makerworld.com/en/models/2704030).

## Actual assembly topology

Bambu installation steps say:

1. The three button boards connect to the supplied **1-to-4 cable** (color-matched).
2. The **black branch of the same 1-to-4 harness connects to the Hall board**.
3. The shared harness terminates on **one mainboard connector**.

Therefore the sensor and buttons are NOT necessarily independent separate harnesses. A full harness pass-through adapter for this connector might simplify the hardware, but the **pin count, pin order, electrical scheme, available supply rails and connector family have NOT been established from these photos**.

**Do not equate the internal Hall/button harness connector with the external 4-pin Bambu Bus power connector.** These are distinct and must be traced independently.

## Power decision

- **Preferred**: protected 24 V (nominal) -> 5 V DC/DC tapped from measured Bambu Bus supply via matching pass-through connector; see [INTEGRATED_POWER](INTEGRATED_POWER.md).
- **Alternative (unverified)**: 3.3/5 V from Hall harness, only IF the correct voltage and regulator spare capacity are measured **under Wi-Fi peak load** and without corrupting Hall feedback. There is no public confirmation of the regulator headroom, so this is **not selected**.
- No PCB modifications or wiring assumptions until connector/pinout measurements are available. Reference photographs do not prove the pinout or electrical capabilities.

**Do not use the readily found black dual-board photo from a P2S forum thread as definitive TPU Feed Assist PCB evidence:** that thread discusses the *P2S printer motherboard* connectors, not a clearly identified close-up of the Feed Assist board.
