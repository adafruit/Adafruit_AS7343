/*!
 * @file 06_led_driver_test.ino
 * @brief Hardware test for AS7343 LED driver functionality
 *
 * Tests the integrated LED driver:
 * - Enable/disable LED
 * - Current settings (4-258mA, even values only)
 * - Current readback verification
 *
 * Hardware: AS7343 on I2C, NeoPixel ring on pin 6 (kept OFF)
 */

#include <Adafruit_AS7343.h>
#include <Adafruit_NeoPixel.h>

#define NEOPIXEL_PIN 6
#define NEOPIXEL_COUNT 16

Adafruit_AS7343 as7343;
Adafruit_NeoPixel pixels(NEOPIXEL_COUNT, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

// Test currents in mA
const uint16_t testCurrents[] = {4, 50, 100, 258};
const uint8_t numCurrents = sizeof(testCurrents) / sizeof(testCurrents[0]);

void setup() {
  Serial.begin(115200);
  while (!Serial)
    delay(10);

  Serial.println(F("AS7343 LED Driver Test"));
  Serial.println(F("======================"));
  Serial.println();

  // Initialize NeoPixels and keep OFF
  pixels.begin();
  pixels.clear();
  pixels.show();

  // Initialize sensor
  if (!as7343.begin()) {
    Serial.println(F("ERROR: AS7343 not found!"));
    Serial.println(F("RESULT: FAIL"));
    while (1)
      delay(100);
  }

  bool enableTestOK = true;
  bool currentTestOK = true;

  // Test LED Enable/Disable
  Serial.println(F("LED Enable/Disable: Testing..."));

  // Test enableLED(true)
  if (as7343.enableLED(true)) {
    Serial.println(F("  enableLED(true): OK"));
  } else {
    Serial.println(F("  enableLED(true): FAIL"));
    enableTestOK = false;
  }
  delay(50);

  // Test enableLED(false)
  if (as7343.enableLED(false)) {
    Serial.println(F("  enableLED(false): OK"));
  } else {
    Serial.println(F("  enableLED(false): FAIL"));
    enableTestOK = false;
  }
  delay(50);

  Serial.println();

  // Test LED Current Settings
  Serial.println(F("LED Current Settings:"));
  Serial.println(F("Current(mA)  Readback(mA)  Status"));
  Serial.println(F("-----------  ------------  ------"));

  for (uint8_t i = 0; i < numCurrents; i++) {
    uint16_t setCurrent = testCurrents[i];

    // Set current
    as7343.setLEDCurrent(setCurrent);
    delay(10);

    // Read back
    uint16_t readback = as7343.getLEDCurrent();

    // Check if readback is within tolerance (2mA for even-only constraint)
    int16_t diff = (int16_t)readback - (int16_t)setCurrent;
    if (diff < 0)
      diff = -diff;
    bool passThis = (diff <= 2);

    if (!passThis)
      currentTestOK = false;

    // Print with alignment
    Serial.print(setCurrent);
    if (setCurrent < 10)
      Serial.print(F("            "));
    else if (setCurrent < 100)
      Serial.print(F("           "));
    else
      Serial.print(F("          "));

    Serial.print(readback);
    if (readback < 10)
      Serial.print(F("             "));
    else if (readback < 100)
      Serial.print(F("            "));
    else
      Serial.print(F("           "));

    Serial.println(passThis ? F("PASS") : F("FAIL"));
  }

  Serial.println();
  Serial.print(F("Current readback: "));
  Serial.println(currentTestOK ? F("PASS") : F("FAIL"));

  // Final result
  bool allPass = enableTestOK && currentTestOK;
  Serial.print(F("RESULT: "));
  Serial.println(allPass ? F("PASS") : F("FAIL"));

  // Enable LED for pulsing demo
  Serial.println(F("\nStarting LED pulse demo (4-258mA, 2s up / 2s down)..."));
  as7343.setLEDCurrent(4);
  as7343.enableLED(true);
}

void loop() {
  // Ramp up: 4 to 258 mA in 2 seconds (128 steps of 2mA)
  for (uint16_t ma = 4; ma <= 258; ma += 2) {
    as7343.setLEDCurrent(ma);
    delay(15);
  }

  // Ramp down: 258 to 4 mA in 2 seconds
  for (uint16_t ma = 258; ma >= 4; ma -= 2) {
    as7343.setLEDCurrent(ma);
    delay(15);
  }
}
