/*!
 * AS7343 Threshold Readback Test
 *
 * Tests setLowThreshold/getLowThreshold and setHighThreshold/getHighThreshold
 * Hardware: Metro Mini + AS7343 on I2C
 */

#include <Adafruit_AS7343.h>

Adafruit_AS7343 as7343;

bool allPassed = true;

void printHeader(const char *title, const char *col1, const char *col2,
                 const char *col3) {
  Serial.println(title);
  Serial.print(col1);
  Serial.print(col2);
  Serial.println(col3);
  char dashes[40];
  memset(dashes, '-', sizeof(dashes) - 1);
  dashes[sizeof(dashes) - 1] = '\0';
  Serial.print(dashes + (sizeof(dashes) - 11));
  Serial.print(dashes + (sizeof(dashes) - 11));
  Serial.println(dashes + (sizeof(dashes) - 9));
}

void printRow(uint16_t value, uint16_t readback, bool pass) {
  char buf[40];
  snprintf(buf, sizeof(buf), "%-10u %-10u %s", value, readback,
           pass ? "PASS" : "FAIL");
  Serial.println(buf);
  if (!pass)
    allPassed = false;
}

void setup() {
  Serial.begin(115200);
  while (!Serial)
    delay(10);
  delay(500);

  Serial.println(F("AS7343 Threshold Test"));
  Serial.println(F("====================="));
  Serial.println();

  if (!as7343.begin()) {
    Serial.println(F("ERROR: AS7343 not found!"));
    Serial.println(F("RESULT: FAIL"));
    while (1)
      delay(10);
  }

  // Test values
  uint16_t testValues[] = {0, 1000, 32768, 65535};
  const int numValues = 4;

  // Low threshold test
  printHeader("Low Threshold Readback:", "Value      ", "Readback   ", "Status");
  for (int i = 0; i < numValues; i++) {
    as7343.setLowThreshold(testValues[i]);
    uint16_t readback = as7343.getLowThreshold();
    printRow(testValues[i], readback, readback == testValues[i]);
  }
  Serial.println();

  // High threshold test
  printHeader("High Threshold Readback:", "Value      ", "Readback   ", "Status");
  for (int i = 0; i < numValues; i++) {
    as7343.setHighThreshold(testValues[i]);
    uint16_t readback = as7343.getHighThreshold();
    printRow(testValues[i], readback, readback == testValues[i]);
  }
  Serial.println();

  // Independence test
  Serial.println(F("Independence test:"));
  uint16_t lowVal = 1234;
  uint16_t highVal = 5678;

  as7343.setLowThreshold(lowVal);
  as7343.setHighThreshold(highVal);

  uint16_t readLow = as7343.getLowThreshold();
  uint16_t readHigh = as7343.getHighThreshold();

  Serial.print(F("  Set low="));
  Serial.print(lowVal);
  Serial.print(F(", high="));
  Serial.println(highVal);

  bool indepPass = (readLow == lowVal && readHigh == highVal);
  Serial.print(F("  Read low: "));
  Serial.print(readLow);
  Serial.print(F(", high: "));
  Serial.print(readHigh);
  Serial.print(F(" - "));
  Serial.println(indepPass ? F("PASS") : F("FAIL"));
  if (!indepPass)
    allPassed = false;

  Serial.println();
  Serial.print(F("Threshold readback: "));
  Serial.println(allPassed ? F("PASS") : F("FAIL"));
  Serial.print(F("RESULT: "));
  Serial.println(allPassed ? F("PASS") : F("FAIL"));
}

void loop() { delay(1000); }
