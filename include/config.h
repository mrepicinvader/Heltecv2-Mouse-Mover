#pragma once

// User-adjustable settings
#if defined(BOARD_ESP32_S3_MINI_NANO)
#define DEVICE_NAME "ESP32-S3 BLE Mouse"
#define DEVICE_MANUFACTURER "ESP32"
#define BUTTON_LABEL "BOOT"
#define BUTTON_PIN 0

// Addressable RGB LED; GPIO48 is the default ESP32-S3 RGB pin.
#define ENABLE_OLED 0
#define ENABLE_STATUS_LED 0
#define ENABLE_RGB_STATUS_LED 1
#define RGB_STATUS_LED_PIN 48
#define RGB_STATUS_LED_BRIGHTNESS 5  // Maximum channel brightness, 0-255; keep it dim.
#define RGB_STATUS_LED_PULSE_MS 3000UL  // Full green breathing cycle; must be > 0.
#elif defined(ARDUINO_heltec_wifi_lora_32_V3)
// Heltec WiFi LoRa 32 V3, including hardware revision V3.2.
#define DEVICE_NAME "Heltec V3 BLE Mouse"
#define DEVICE_MANUFACTURER "Heltec"
#define BUTTON_LABEL "PRG"

#define BUTTON_PIN 0
#define LED_PIN 35

#define ENABLE_OLED 1
#define ENABLE_STATUS_LED 1
#define ENABLE_RGB_STATUS_LED 0
#define OLED_SDA 17
#define OLED_SCL 18
#define OLED_RST 21
#define OLED_ADDRESS 0x3C
#define OLED_POWER_PIN 36  // Vext control, active low.
#else
#define DEVICE_NAME "Heltec BLE Mouse"
#define DEVICE_MANUFACTURER "Heltec"
#define BUTTON_LABEL "PRG"

#define BUTTON_PIN 0
#define LED_PIN 25

#define ENABLE_OLED 1
#define ENABLE_STATUS_LED 1
#define ENABLE_RGB_STATUS_LED 0
#define OLED_SDA 4
#define OLED_SCL 15
#define OLED_RST 16
#define OLED_ADDRESS 0x3C
#endif

#define MOUSE_MOVE_PIXELS 60
#define MOUSE_MOVE_INTERVAL_MS 9000UL
#define MOUSE_RETURN_DELAY_MS 250UL

#define BUTTON_DEBOUNCE_MS 50UL
#define MOVE_INDICATOR_MS 1000UL
#define BUTTON_FEEDBACK_MS 800UL
#define BOOT_SCREEN_MS 1200UL
#define INSTRUCTION_SCREEN_MS 1800UL
