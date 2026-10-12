# Heltec BLE Mouse (WiFi LoRa 32 V2)

This PlatformIO/Arduino firmware turns an **original-ESP32 Heltec WiFi LoRa 32 V2** into a standard Bluetooth Low Energy HID mouse named `Heltec BLE Mouse`. It needs no Windows software after flashing. It is not for the Heltec V3 (ESP32-S3).

## Hardware and pins

The LoRa radio is deliberately not initialized.

| Function | GPIO |
| --- | ---: |
| PRG/user button (active low) | 0 |
| Status LED (optional) | 25 |
| OLED SDA | 4 |
| OLED SCL | 15 |
| OLED reset | 16 |
| OLED I2C address | `0x3C` |

## Build and flash

Install [PlatformIO](https://platformio.org/), open this folder, then build and upload:

```text
pio run
pio run --target upload
pio device monitor --baud 115200
```

The environment uses the `heltec_wifi_lora_32_V2` board definition, Arduino framework, `ESP32 BLE Mouse`, and U8g2. USB upload uses the board's normal USB serial port.

## Windows pairing

After boot, open **Settings → Bluetooth & devices → Add device → Bluetooth** and choose **Heltec BLE Mouse**. Windows sees it as an ordinary BLE mouse. The HID library advertises, pairs/bonds through the standard ESP32 BLE stack, and resumes advertising after a disconnect. If Windows has stale pairing data, remove *Heltec BLE Mouse* in Windows Bluetooth settings, reboot/reset the board, and pair again. To clear bonds on the ESP32 itself, erase its flash (for example `pio run --target erase`) and upload the firmware again; that also erases all other saved firmware data.

## Operation

Movement always starts **OFF** after a reset. Press and release PRG once to enable it; press and release again to disable it. The button uses a 50 ms debounce and toggles only once per press. When enabled and connected, the board sends `+5` X, waits 250 ms, sends `-5` X, then waits 30 seconds before the next cycle. No long blocking delay is used.

OLED meanings:

| Bluetooth | Mouse | Meaning |
| --- | --- | --- |
| WAITING... | OFF | Not connected; movement disabled |
| WAITING... | ARMED | Movement enabled; waiting for a host |
| CONNECTED | ACTIVE | Movement enabled and reports may be sent |

The OLED briefly shows button feedback and `ACTIVE >` / `NOW` after a report. The optional GPIO25 LED is off for OFF, slow-blinks for ARMED, and is on for ACTIVE.

## Configuration and troubleshooting

Edit [include/config.h](include/config.h) to change `MOUSE_MOVE_PIXELS`, `MOUSE_MOVE_INTERVAL_MS`, the return delay, display/button timings, or `ENABLE_STATUS_LED`.

If BLE does not pair, remove its old Windows entry, power-cycle the board, and retry near the computer. Use the 115200 baud serial monitor for connection diagnostics. If the OLED is blank, confirm this is V2 hardware and the fixed pins SDA=4, SCL=15, RESET=16; the firmware explicitly resets it and calls `Wire.begin(4, 15)`.

**GPIO0 warning:** PRG is an ESP32 boot-strapping pin. Do not hold it while resetting or powering the board unless you intentionally want the bootloader. After a normal boot, it is read only as the runtime toggle button.
