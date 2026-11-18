# Hardware verification checklist

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
