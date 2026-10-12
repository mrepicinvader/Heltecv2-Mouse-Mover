#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <BleMouse.h>
#include "config.h"

// The BLE library supplies the standard HID mouse descriptor, advertising,
// pairing/bonding support, and reconnect handling for the ESP32 Arduino stack.
BleMouse bleMouse(DEVICE_NAME, "Heltec", 100);
U8G2_SSD1306_128X64_NONAME_F_HW_I2C display(U8G2_R0, U8X8_PIN_NONE);

bool bleConnected = false;
bool mouseEnabled = false;
bool displayDirty = true;
bool positiveMovePending = true;
bool buttonStableState = HIGH;
bool buttonLastReading = HIGH;

uint32_t buttonLastChangeTime = 0;
uint32_t lastMovementTime = 0;
uint32_t feedbackUntil = 0;
uint32_t activityUntil = 0;
uint32_t movementCount = 0;
uint32_t bootTime = 0;

enum class Feedback { None, Enabled, Disabled };
Feedback feedback = Feedback::None;

void markDisplayDirty() { displayDirty = true; }

void initializeDisplay() {
  pinMode(OLED_RST, OUTPUT);
  digitalWrite(OLED_RST, LOW);
  delay(20);  // Hardware reset pulse required by the Heltec V2 OLED.
  digitalWrite(OLED_RST, HIGH);

  Wire.begin(OLED_SDA, OLED_SCL);
  display.setI2CAddress(OLED_ADDRESS << 1);  // U8g2 expects the 8-bit address.
  display.begin();
  display.setFont(u8g2_font_6x12_tf);
  Serial.println("[OLED] initialized");
}

void initializeBluetooth() {
  bleMouse.begin();
  Serial.println("[BLE] initialized");
  Serial.println("[BLE] advertising");
}

void setMouseEnabled(bool enabled) {
  if (mouseEnabled == enabled) return;

  mouseEnabled = enabled;
  positiveMovePending = true;
  lastMovementTime = millis();
  feedback = enabled ? Feedback::Enabled : Feedback::Disabled;
  feedbackUntil = millis() + BUTTON_FEEDBACK_MS;
  Serial.println(enabled ? "[BUTTON] mouse enabled" : "[BUTTON] mouse disabled");
  markDisplayDirty();
}

void updateButton() {
  const bool reading = digitalRead(BUTTON_PIN);
  const uint32_t now = millis();

  if (reading != buttonLastReading) {
    buttonLastReading = reading;
    buttonLastChangeTime = now;
  }

  if (reading != buttonStableState &&
      (uint32_t)(now - buttonLastChangeTime) >= BUTTON_DEBOUNCE_MS) {
    buttonStableState = reading;
    // Toggle only on the debounced press edge; holding the button cannot repeat.
    if (buttonStableState == LOW) setMouseEnabled(!mouseEnabled);
  }
}

void updateBluetooth() {
  const bool connectedNow = bleMouse.isConnected();
  if (connectedNow == bleConnected) return;

  bleConnected = connectedNow;
  positiveMovePending = true;
  lastMovementTime = millis();
  Serial.println(bleConnected ? "[BLE] connected" : "[BLE] disconnected");
  if (!bleConnected) Serial.println("[BLE] advertising");
  markDisplayDirty();
}

void sendMovement(int8_t x) {
  if (!bleConnected || !mouseEnabled) return;
  bleMouse.move(x, 0, 0, 0);
  ++movementCount;
  activityUntil = millis() + MOVE_INDICATOR_MS;
  Serial.println("[HID] movement sent");
  markDisplayDirty();
}

void updateMouseMovement() {
  if (!mouseEnabled || !bleConnected) return;

  const uint32_t now = millis();
  const uint32_t wait = positiveMovePending ? MOUSE_MOVE_INTERVAL_MS : MOUSE_RETURN_DELAY_MS;
  if ((uint32_t)(now - lastMovementTime) < wait) return;

  sendMovement(positiveMovePending ? MOUSE_MOVE_PIXELS : -MOUSE_MOVE_PIXELS);
  positiveMovePending = !positiveMovePending;
  lastMovementTime = now;
}

void drawText(uint8_t x, uint8_t y, const char *text) { display.drawStr(x, y, text); }

void updateDisplay() {
  const uint32_t now = millis();
  if (feedback != Feedback::None && (int32_t)(now - feedbackUntil) >= 0) {
    feedback = Feedback::None;
    markDisplayDirty();
  }
  if (activityUntil != 0 && (int32_t)(now - activityUntil) >= 0) {
    activityUntil = 0;
    markDisplayDirty();
  }
  if (!displayDirty) return;
  displayDirty = false;

  display.clearBuffer();
  if ((uint32_t)(now - bootTime) < BOOT_SCREEN_MS) {
    drawText(0, 22, "Heltec BLE Mouse");
    drawText(0, 42, "Starting...");
  } else if ((uint32_t)(now - bootTime) < BOOT_SCREEN_MS + INSTRUCTION_SCREEN_MS) {
    drawText(0, 22, "BLE advertising...");
    drawText(0, 42, "Press PRG");
    drawText(0, 57, "to enable");
  } else if (feedback != Feedback::None) {
    drawText(0, 25, "Mouse movement");
    drawText(0, 48, feedback == Feedback::Enabled ? "ENABLED" : "DISABLED");
  } else {
    drawText(0, 12, "HELTEC BLE MOUSE");
    drawText(0, 30, "Bluetooth:");
    drawText(70, 30, bleConnected ? "CONNECTED" : "WAITING...");
    drawText(0, 48, "Mouse:");
    const char *state = !mouseEnabled ? "OFF" : (bleConnected ? "ACTIVE" : "ARMED");
    drawText(48, 48, state);
    if (activityUntil != 0) drawText(96, 48, ">");
    char countText[22];
    snprintf(countText, sizeof(countText), "Moves: %lu", (unsigned long)movementCount);
    drawText(0, 63, countText);
    if (activityUntil != 0) drawText(78, 63, "NOW");
  }
  display.sendBuffer();
}

void updateStatusLed() {
#if ENABLE_STATUS_LED
  static bool lastLed = false;
  bool ledOn = false;
  const uint32_t now = millis();
  if (activityUntil != 0) ledOn = ((now / 100UL) % 2UL) == 0;
  else if (mouseEnabled && bleConnected) ledOn = true;
  else if (mouseEnabled) ledOn = ((now / 500UL) % 2UL) == 0;
  if (ledOn != lastLed) {
    digitalWrite(LED_PIN, ledOn ? HIGH : LOW);
    lastLed = ledOn;
  }
#endif
}

void setup() {
  Serial.begin(115200);
  Serial.println("[BOOT] Heltec BLE Mouse");
  pinMode(BUTTON_PIN, INPUT_PULLUP);
#if ENABLE_STATUS_LED
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
#endif
  bootTime = millis();
  initializeDisplay();
  initializeBluetooth();
}

void loop() {
  updateButton();
  updateBluetooth();
  updateMouseMovement();
  updateDisplay();
  updateStatusLed();
}
