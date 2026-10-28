# ESP32 BLE Mouse (Heltec V2, V3/V3.2 and ESP32-S3-MINI-Nano)

This PlatformIO/Arduino firmware turns a **Heltec WiFi LoRa 32 V2**, **Heltec WiFi LoRa 32 V3 (including V3.2)**, or **ESP32-S3-MINI-Nano N16R8** into a standard Bluetooth Low Energy HID mouse. It needs no Windows software after flashing.

| Board | PlatformIO environment | Bluetooth name |
| --- | --- | --- |
| Heltec WiFi LoRa 32 V2 (default) | `heltec_wifi_lora_32_v2` | `Heltec BLE Mouse` |
| Heltec WiFi LoRa 32 V3 / V3.2 | `heltec_wifi_lora_32_v3` | `Heltec V3 BLE Mouse` |
| NiceMCU ESP32-S3-MINI-Nano (marked ESP32-S3-N16R8) | `esp32_s3_mini_nano` | `ESP32-S3 BLE Mouse` |

## Hardware and pins

The Heltec LoRa radio is deliberately not initialized. Each Heltec profile selects its own pins:

| Function | V2 GPIO | V3 / V3.2 GPIO |
| --- | ---: | ---: |
| PRG/user button (active low) | 0 | 0 |
| Status LED (optional) | 25 | 35 |
| OLED SDA | 4 | 17 |
| OLED SCL | 15 | 18 |
| OLED reset | 16 | 21 |
| OLED power / Vext (active low) | Not controlled | 36 |
| OLED I2C address | `0x3C` | `0x3C` |

The V3 profile uses PlatformIO's built-in `heltec_wifi_lora_32_V3` board definition for the ESP32-S3 with 8 MB flash and no PSRAM. Its OLED and LED pins match the [Arduino board variant](https://github.com/espressif/arduino-esp32/blob/2.0.17/variants/heltec_wifi_lora_32_V3/pins_arduino.h). The firmware drives Vext control GPIO36 LOW before resetting and initializing the OLED, following [Heltec's power-control convention](https://github.com/HelTecAutomation/Heltec_ESP32/blob/master/src/heltec.h). V3.2 uses this same profile; see [Heltec's hardware revision notes](https://docs.heltec.org/en/node/esp32/wifi_lora_32/hardware_update_log.html#v3-2).

The ESP32-S3-MINI-Nano profile targets the **N16R8 variant (16 MB flash, 8 MB octal PSRAM)** described in the [board listing](https://wavgat.com/product/esp32-s3-n16r8-mini-development-board-type-c-interface-wifi-bluetooth-iot-microcontroller-module-16mb-flash-8mb-psram/). Confirm this matches your board; this profile is not for the 4 MB S3 SuperMini or Arduino Nano ESP32. It uses the BOOT button on GPIO0 to toggle movement. OLED and discrete status LED support are disabled for this profile. The onboard RGB indicator is configured as a WS2812-compatible LED on GPIO48, the Arduino S3 profile's default; adjust `RGB_STATUS_LED_PIN` if your board revision uses another pin. Connection, button, and movement feedback is also available through the serial monitor.

## Build and flash

Install [PlatformIO](https://platformio.org/), open this folder, then build and upload:

```text
pio run
pio run --target upload
pio device monitor --baud 115200
```

These commands select the default Heltec environment, using the `heltec_wifi_lora_32_V2` board definition, Arduino framework, `ESP32 BLE Mouse`, and U8g2. USB upload uses the board's normal USB serial port.

For the Heltec WiFi LoRa 32 V3 or V3.2, select its environment explicitly:

```text
pio run -e heltec_wifi_lora_32_v3
pio run -e heltec_wifi_lora_32_v3 --target upload
pio device monitor -e heltec_wifi_lora_32_v3 --baud 115200
```

Use the board's USB-C connector and CP2102 USB serial port for upload and serial diagnostics. This profile uses UART serial through the CP2102; native USB CDC is not enabled. If automatic upload fails, hold PRG, press and release RST, release PRG, then retry uploading. Select the port with `--upload-port COMx` if needed.

For the ESP32-S3-MINI-Nano, select its environment explicitly:

```text
pio run -e esp32_s3_mini_nano
pio run -e esp32_s3_mini_nano --target upload
pio device monitor -e esp32_s3_mini_nano --baud 115200
```

The project includes a local `esp32-s3-mini-nano` board definition with 16 MB flash partitions, octal PSRAM, and native USB CDC serial enabled. USB carries upload and serial diagnostics; mouse reports use Bluetooth LE. If automatic upload fails, hold BOOT, press and release RST, release BOOT, then retry uploading. Select the new serial port with `--upload-port COMx` if needed. Press RST after flashing if the board stays in the bootloader.

Build all three profiles with `pio run -e heltec_wifi_lora_32_v2 -e heltec_wifi_lora_32_v3 -e esp32_s3_mini_nano`.

The pre-build script [scripts/patch_ble_mouse.py](scripts/patch_ble_mouse.py) automatically adapts the two string arguments in ESP32 BLE Mouse 0.3.1 to work with Arduino-ESP32 2.x and 3.x. It reapplies the fix after dependency reinstalls; no manual changes inside `.pio/libdeps` are needed.

## Windows pairing

After boot, open **Settings → Bluetooth & devices → Add device → Bluetooth** and choose **Heltec BLE Mouse**, **Heltec V3 BLE Mouse**, or **ESP32-S3 BLE Mouse**, according to your board. Windows sees it as an ordinary BLE mouse. The HID library advertises, pairs/bonds through the standard ESP32 BLE stack, and resumes advertising after a disconnect. If Windows has stale pairing data, remove the matching mouse entry in Windows Bluetooth settings, reboot/reset the board, and pair again. To clear bonds on the ESP32 itself, erase its flash (`pio run -e <environment> --target erase`) and upload the firmware again; that also erases all other saved firmware data.

## Operation

Movement always starts **OFF** after a reset. Press and release PRG (Heltec) or BOOT (MINI-Nano) once to enable it; press and release again to disable it. The button uses a 50 ms debounce and toggles only once per press. When enabled and connected, the board waits 9 seconds, sends `+60` X, waits 250 ms, sends `-60` X, then waits 9 seconds before the next cycle. No long blocking delay is used.

Heltec OLED meanings:

| Bluetooth | Mouse | Meaning |
| --- | --- | --- |
| WAITING... | OFF | Not connected; movement disabled |
| WAITING... | ARMED | Movement enabled; waiting for a host |
| CONNECTED | ACTIVE | Movement enabled and reports may be sent |

The OLED briefly shows button feedback and `ACTIVE >` / `NOW` after a report. The optional status LED (GPIO25 on V2, GPIO35 on V3/V3.2) is off for OFF, slow-blinks for ARMED, and is on for ACTIVE, with brief activity flashes after movement reports.

On the MINI-Nano, the RGB LED slowly pulses green on a three-second breathing cycle while movement is enabled and Bluetooth is connected. It glows solid dim red after boot, while disabled, or while waiting for a connection. It does not flash with individual movement reports.

## Configuration and troubleshooting

Edit [include/config.h](include/config.h) to change `MOUSE_MOVE_PIXELS`, `MOUSE_MOVE_INTERVAL_MS`, the return delay, display/button timings, or each board's peripheral settings. The board definition's `ARDUINO_heltec_wifi_lora_32_V3` flag selects the V3/V3.2 defaults; `BOARD_ESP32_S3_MINI_NANO` selects the MINI-Nano defaults. For the RGB indicator, adjust `RGB_STATUS_LED_PIN`, `RGB_STATUS_LED_BRIGHTNESS` (0-255, default 5), and `RGB_STATUS_LED_PULSE_MS` (full pulse period in milliseconds, default 3000; must be greater than zero), or set `ENABLE_RGB_STATUS_LED` to 0 to disable it. The firmware uses Arduino's built-in addressable LED driver. To add an external OLED to the MINI-Nano, enable `ENABLE_OLED`, define its SDA/SCL/reset/address pins, and add U8g2 to that environment's `lib_deps`. Enable `ENABLE_STATUS_LED` only after defining a verified discrete `LED_PIN`.

If BLE does not pair, remove its old Windows entry, power-cycle the board, and retry near the computer. Use the 115200 baud serial monitor for connection diagnostics. If the OLED is blank, confirm that you flashed the environment for your board: V2 uses SDA=4, SCL=15, RESET=16; V3/V3.2 uses SDA=17, SCL=18, RESET=21 and needs GPIO36 LOW to power the OLED. The firmware selects these pins and resets the display automatically.

**GPIO0 warning:** PRG (Heltec) and BOOT (MINI-Nano) use a boot-strapping pin. Do not hold the button while resetting or powering the board unless you intentionally want the bootloader. After a normal boot, it is read only as the runtime toggle button.

## Release notes

### v1.0.0 - Initial release

- Initial firmware for the original Heltec WiFi LoRa 32 V2 ESP32 board
- Turns the board into a Bluetooth LE HID mouse named `Heltec BLE Mouse`
- No Windows driver or companion app required after flashing
- PRG button toggles mouse motion on and off with debounce and status feedback
- OLED display reports connection and motion state in a simple, visible format
- Optional status LED shows whether the board is off, armed, or active
- Uses a configurable movement pattern that sends periodic X-axis mouse motion while connected
- Keeps the LoRa radio uninitialized and uses the V2-specific OLED pin mapping
- Includes setup, pairing, and troubleshooting guidance for Windows users

This is the initial release baseline for the project and is intended for the Heltec WiFi LoRa 32 V2 hardware only.
