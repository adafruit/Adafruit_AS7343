/*!
 * @file 16_gpio_test.ino
 * @brief Hardware test for AS7343 GPIO functionality
 *
 * Tests GPIO pin control:
 * - Output mode with value setting
 * - GPIO inversion setting
 * - Input mode reading
 *
 * Note: getGPIOValue() reads the actual pin state (GPIO_IN bit), not the
 * output register value. Without external circuitry, read-back won't match
 * what was written - this is expected behavior.
 *
 * GPIO register is in bank 1, so this also tests bank switching.
 */

#include <Adafruit_AS7343.h>

Adafruit_AS7343 as7343;
bool allPassed = true;

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);
  delay(500);

  Serial.println(F("AS7343 GPIO Test"));
  Serial.println(F("================"));
  Serial.println();

  if (!as7343.begin()) {
    Serial.println(F("ERROR: AS7343 not found!"));
    Serial.println(F("RESULT: FAIL"));
    while (1) delay(10);
  }

  // Output Mode Test
  Serial.println(F("Output Mode Test:"));

  // Set to output mode
  bool ok = as7343.setGPIOOutput(true);
  Serial.print(F("  setGPIOOutput(true): "));
  Serial.println(ok ? F("OK") : F("FAIL"));
  if (!ok) allPassed = false;

  // Set high
  ok = as7343.setGPIOValue(true);
  Serial.print(F("  setGPIOValue(true): "));
  Serial.println(ok ? F("OK") : F("FAIL"));
  if (!ok) allPassed = false;

  // Read - note: reads GPIO_IN (actual pin state), not GPIO_OUT
  bool val = as7343.getGPIOValue();
  Serial.print(F("  getGPIOValue(): "));
  Serial.println(val ? F("true") : F("false"));

  // Set low
  ok = as7343.setGPIOValue(false);
  Serial.print(F("  setGPIOValue(false): "));
  Serial.println(ok ? F("OK") : F("FAIL"));
  if (!ok) allPassed = false;

  val = as7343.getGPIOValue();
  Serial.print(F("  getGPIOValue(): "));
  Serial.println(val ? F("true") : F("false"));

  Serial.println();

  // Inversion Test
  Serial.println(F("Inversion Test:"));
  ok = as7343.setGPIOInverted(true);
  Serial.print(F("  setGPIOInverted(true): "));
  Serial.println(ok ? F("OK") : F("FAIL"));
  if (!ok) allPassed = false;

  ok = as7343.setGPIOInverted(false);
  Serial.print(F("  setGPIOInverted(false): "));
  Serial.println(ok ? F("OK") : F("FAIL"));
  if (!ok) allPassed = false;

  Serial.println();

  // Input Mode Test
  Serial.println(F("Input Mode Test:"));
  ok = as7343.setGPIOOutput(false);
  Serial.print(F("  setGPIOOutput(false): "));
  Serial.println(ok ? F("OK") : F("FAIL"));
  if (!ok) allPassed = false;

  val = as7343.getGPIOValue();
  Serial.print(F("  getGPIOValue(): "));
  Serial.print(val ? F("true") : F("false"));
  Serial.println(F(" (depends on pin state)"));

  Serial.println();

  // Summary
  Serial.print(F("GPIO API: "));
  Serial.println(allPassed ? F("PASS") : F("FAIL"));

  Serial.print(F("RESULT: "));
  Serial.println(allPassed ? F("PASS") : F("FAIL"));
}

void loop() {
  delay(1000);
}
