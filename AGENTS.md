# Repository guidance

This is PlatformIO/Arduino firmware for a BLE mouse mover supporting Heltec WiFi LoRa 32 V2, V3/V3.2, and ESP32-S3-MINI-Nano N16R8 boards.

## Project layout

- `src/main.cpp`: button debounce, BLE connection tracking, movement scheduling, OLED, and LED updates.
- `include/config.h`: board-specific pins, peripheral flags, and adjustable timings.
- `platformio.ini` and `boards/`: build environments and the custom MINI-Nano board definition.
- `scripts/patch_ble_mouse.py`: build-time compatibility patch for the BLE mouse dependency.

## Implementation conventions

- Preserve existing user changes and keep edits focused on the requested behavior.
- Keep runtime updates nonblocking with `millis()` timing so button handling and BLE remain responsive.
- Movement starts disabled and reports require both `mouseEnabled` and `bleConnected`.
- Guard peripheral code with its feature flag; preserve all supported board profiles.
- The MINI-Nano RGB LED pulses green when enabled and connected, and stays solid red otherwise. Keep LED brightness and pulse timing configurable.
- Keep the LoRa radio uninitialized.
- Update `README.md` when changing user-visible behavior or configuration.
- Modify dependency fixes in the pre-build script, not generated `.pio/libdeps` files.

## Validation

Build all supported profiles after firmware changes:

```text
pio run -e heltec_wifi_lora_32_v2 -e heltec_wifi_lora_32_v3 -e esp32_s3_mini_nano
```

Review `git diff --check`. Report build results and distinguish compilation from hardware verification. On hardware, check startup, button toggling, Bluetooth connection/disconnection, and LED behavior while movement reports continue.
