/*!
 * @file 15_autozero_test.ino
 * @brief Hardware test for AS7343 auto-zero configuration
 *
 * Tests auto-zero frequency API (compensates for temperature drift):
 * - setAutoZeroFrequency(uint8_t) - set frequency 0-255
 * - getAutoZeroFrequency() - read current frequency
 *
 * Values:
 * - 0 = never (not recommended)
 * - 1 = every cycle
 * - 255 = only before first measurement (default)
 *
 * Hardware: AS7343 on I2C
 */

#include <Adafruit_AS7343.h>

Adafruit_AS7343 as7343;

// Test values for auto-zero frequency
const uint8_t testValues[] = {0, 1, 127, 255};
const uint8_t numTests = sizeof(testValues) / sizeof(testValues[0]);

void setup() {
  Serial.begin(115200);
  while (!Serial)
    delay(10);

  Serial.println(F("AS7343 Auto-Zero Test"));
  Serial.println(F("====================="));
  Serial.println();

  // Initialize sensor
  if (!as7343.begin()) {
    Serial.println(F("ERROR: AS7343 not found!"));
    Serial.println(F("RESULT: FAIL"));
    while (1)
      delay(100);
  }

  bool allPassed = true;

  // =========================================
  // Test auto-zero frequency readback
  // =========================================
  Serial.println(F("Auto-Zero Frequency Readback:"));
  Serial.println(F("Value    Readback   Status"));
  Serial.println(F("-----    --------   ------"));

  for (uint8_t i = 0; i < numTests; i++) {
    uint8_t setValue = testValues[i];

    // Set the auto-zero frequency
    as7343.setAutoZeroFrequency(setValue);

    // Small delay for register write
    delay(10);

    // Read back the value
    uint8_t readValue = as7343.getAutoZeroFrequency();

    // Check if it matches
    bool pass = (readValue == setValue);
    if (!pass) {
      allPassed = false;
    }

    // Print result with aligned columns
    if (setValue < 10) {
      Serial.print(F("  "));
    } else if (setValue < 100) {
      Serial.print(F(" "));
    }
    Serial.print(setValue);
    Serial.print(F("      "));

    if (readValue < 10) {
      Serial.print(F("  "));
    } else if (readValue < 100) {
      Serial.print(F(" "));
    }
    Serial.print(readValue);
    Serial.print(F("        "));

    Serial.println(pass ? F("PASS") : F("FAIL"));
  }

  Serial.println();

  // =========================================
  // Results
  // =========================================
  Serial.print(F("Auto-zero readback: "));
  Serial.println(allPassed ? F("PASS") : F("FAIL"));

  Serial.print(F("RESULT: "));
  Serial.println(allPassed ? F("PASS") : F("FAIL"));
}

void loop() {
  // Nothing to do
}
