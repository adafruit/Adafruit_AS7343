/*!
 * @file 01_begin_test.ino
 * @brief Hardware test: AS7343 begin() and ID verification
 *
 * Tests:
 * - I2C communication with AS7343
 * - Part ID verification (expect 0x81)
 * - Aux ID and Revision ID readback
 */

#include <Adafruit_AS7343.h>

Adafruit_AS7343 apds;

void setup() {
  Serial.begin(115200);
  while (!Serial)
    delay(10);

  Serial.println(F("================================"));
  Serial.println(F("AS7343 Begin Test"));
  Serial.println(F("================================"));

  if (!apds.begin()) {
    Serial.println(F("ERROR: Could not find AS7343!"));
    Serial.println(F("RESULT: FAIL"));
    while (1)
      delay(10);
  }

  Serial.println(F("AS7343 initialized OK"));

  // Read and verify Part ID
  uint8_t partID = apds.getPartID();
  Serial.print(F("Part ID: 0x"));
  Serial.println(partID, HEX);

  // Read Aux ID
  uint8_t auxID = apds.getAuxID();
  Serial.print(F("Aux ID: 0x"));
  Serial.println(auxID, HEX);

  // Read Revision ID
  uint8_t revID = apds.getRevisionID();
  Serial.print(F("Revision ID: 0x"));
  Serial.println(revID, HEX);

  Serial.println(F("--------------------------------"));

  // Verify Part ID
  if (partID == 0x81) {
    Serial.println(F("Part ID verified: PASS"));
    Serial.println(F("RESULT: PASS"));
  } else {
    Serial.print(F("Part ID mismatch! Expected 0x81, got 0x"));
    Serial.println(partID, HEX);
    Serial.println(F("RESULT: FAIL"));
  }
}

void loop() {
  delay(1000);
}
