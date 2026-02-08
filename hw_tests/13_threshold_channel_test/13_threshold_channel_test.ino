/*
 * AS7343 Threshold Channel Test
 *
 * Tests setThresholdChannel() and getThresholdChannel() for values 0-5.
 * The threshold channel selects which ADC channel is compared against thresholds.
 * This register (CFG12) is in bank 1, so this test also exercises bank switching.
 */

#include <Adafruit_AS7343.h>

Adafruit_AS7343 as7343;

bool allPassed = true;

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);

  Serial.println(F("AS7343 Threshold Channel Test"));
  Serial.println(F("============================="));
  Serial.println();

  if (!as7343.begin()) {
    Serial.println(F("ERROR: Could not find AS7343 sensor!"));
    Serial.println(F("RESULT: FAIL"));
    while (1) delay(10);
  }

  // Test threshold channel readback for values 0-5
  Serial.println(F("Threshold Channel Readback:"));
  Serial.println(F("Channel  Readback   Status"));
  Serial.println(F("-------  --------   ------"));

  for (uint8_t ch = 0; ch <= 5; ch++) {
    as7343.setThresholdChannel(ch);
    uint8_t readback = as7343.getThresholdChannel();

    Serial.print(ch);
    Serial.print(F("        "));
    Serial.print(readback);
    Serial.print(F("          "));

    if (readback == ch) {
      Serial.println(F("PASS"));
    } else {
      Serial.println(F("FAIL"));
      allPassed = false;
    }
  }

  Serial.println();
  Serial.print(F("Threshold channel readback: "));
  Serial.println(allPassed ? F("PASS") : F("FAIL"));
  Serial.print(F("RESULT: "));
  Serial.println(allPassed ? F("PASS") : F("FAIL"));
}

void loop() {
  delay(1000);
}
