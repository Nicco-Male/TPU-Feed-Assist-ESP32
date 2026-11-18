# Hardware verification checklist

- [ ] Confirm target board is Seeed **XIAO ESP32-S3 standard**, pin mapping per [XIAO_ESP32S3.md](XIAO_ESP32S3.md).
- [ ] Verify the nominal 24 V feeder power rail/polarity on this actual unit, with 2 AMS + Hub connected; do **not** touch data/RS485 conductors.
- [ ] Verify feeder and both AMS leave enough supply headroom for the XIAO added load.
- [ ] Bench-test protected DC/DC 24 -> 5 V supply (fuse, transient protection, current headroom) and XIAO diode/power-path solution.
- [ ] Verify no USB reverse backfeed when connecting a programming computer.
- [ ] Decide and document isolated vs non-isolated DC/DC; non-isolated DC/DC shares Bambu GND and does **not** provide full galvanic isolation.
- [ ] Verify original feeder remains functional if added 5 V supply branch fails.
- [ ] See [INTEGRATED_POWER.md](INTEGRATED_POWER.md) for final integrated power design; USB-C separate supply is only for safe prototype testing.
- [ ] Verify XIAO boot pins keep the three PhotoMOS outputs **off** (external resistors on driver inputs), including reset/USB disconnect.

**Do not power the bridge from the unknown switch conductors.**

- [ ] PhotoMOS contact resistance, leakage and voltage rating verified against Bambu button terminals (measured).
- [ ] 2-pin pinout and switch mode verified off-power; polarity + waveform recorded powered.
- [ ] Original local buttons work even with ESP32 unpowered.
- [ ] ESP32 GPIO default states keep PhotoMOS outputs OFF during cold boot and brownout.
- [ ] FEED and FORCE tested on printer idle, with manually confirmed pulse duration.
- [ ] Physical UNLOAD tested only after verifying documented safe filament/PTFE procedure.
- [ ] HTTP authentication and unique command ID checks verified on isolated bench.
- [ ] MQTT LWT, duplicate/stale commands and restart behavior tested.
- [ ] No remote press during actual printing until printer-state interlock exists.
- [ ] Electrical sensing design (if installed) does not draw detectable key-line current or cause false presses.
- [ ] Measured manual press/remote press classification is correct for all buttons.

Each tested sample should record module PCB revision, voltages, switch waveforms, test date, and results. No checkbox is currently a claim of passing.
