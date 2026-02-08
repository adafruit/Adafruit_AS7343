/*!
 * @file 12_persistence_test.ino
 * @brief Hardware test for AS7343 persistence setting
 *
 * Tests setPersistence() and getPersistence() for all values 0-15.
 * Persistence determines how many consecutive out-of-threshold
 * readings are needed before an interrupt fires.
 *
 * Hardware: AS7343 on I2C
 * Board: Metro Mini (arduino:avr:uno)
 */

#include <Adafruit_AS7343.h>

Adafruit_AS7343 as7343;
bool allPassed = true;

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);

  Serial.println(F("AS7343 Persistence Test"));
  Serial.println(F("======================="));
  Serial.println();

  if (!as7343.begin()) {
    Serial.println(F("ERROR: AS7343 not found!"));
    Serial.println(F("RESULT: FAIL"));
    while (1) delay(10);
  }

  // Test persistence readback for values 0-15
  Serial.println(F("Persistence Readback:"));
  Serial.println(F("Value    Readback   Status"));
  Serial.println(F("-----    --------   ------"));

  for (uint8_t val = 0; val <= 15; val++) {
    as7343.setPersistence(val);
    uint8_t readback = as7343.getPersistence();

    Serial.print(val);
    if (val < 10) Serial.print(F(" "));
    Serial.print(F("       "));
    Serial.print(readback);
    if (readback < 10) Serial.print(F(" "));
    Serial.print(F("         "));

    if (readback == val) {
      Serial.println(F("PASS"));
    } else {
      Serial.println(F("FAIL"));
      allPassed = false;
    }
  }

  Serial.println();
  Serial.print(F("Persistence readback: "));
  Serial.println(allPassed ? F("PASS") : F("FAIL"));

  Serial.println();
  Serial.print(F("RESULT: "));
  Serial.println(allPassed ? F("PASS") : F("FAIL"));
}

void loop() {
  delay(1000);
}
