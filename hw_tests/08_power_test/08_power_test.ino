/*!
 * AS7343 Power Control Test
 *
 * Tests the power control API:
 * - powerOn(bool) - main power enable (PON bit in ENABLE register)
 * - enableLowPower(bool) - low power idle mode
 *
 * Hardware: AS7343 on I2C, NeoPixel ring on Pin 6
 */

#include <Adafruit_AS7343.h>
#include <Adafruit_NeoPixel.h>

#define NEOPIXEL_PIN 6
#define NUM_PIXELS 16

Adafruit_AS7343 as7343;
Adafruit_NeoPixel pixels(NUM_PIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

bool testPassed = true;

void setAllPixels(uint8_t r, uint8_t g, uint8_t b) {
  for (int i = 0; i < NUM_PIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(r, g, b));
  }
  pixels.show();
}

// Take a reading and return F4 channel value
uint16_t takeReading() {
  as7343.startMeasurement();
  while (!as7343.dataReady()) {
    delay(5);
  }
  return as7343.readChannel(AS7343_CHANNEL_F4);
}

void setup() {
  Serial.begin(115200);
  while (!Serial)
    delay(10);

  Serial.println(F("AS7343 Power Control Test"));
  Serial.println(F("========================="));
  Serial.println();

  // Initialize NeoPixels (start OFF)
  pixels.begin();
  setAllPixels(0, 0, 0);

  // Initialize AS7343
  if (!as7343.begin()) {
    Serial.println(F("ERROR: AS7343 not found!"));
    Serial.println(F("RESULT: FAIL"));
    while (1)
      delay(10);
  }

  // Configure for consistent readings
  as7343.setGain(AS7343_GAIN_128X);
  as7343.setATIME(29);
  as7343.setASTEP(599);

  // Turn NeoPixels ON
  Serial.println(F("NeoPixels ON..."));
  setAllPixels(64, 64, 64);
  delay(100);

  // Initial reading
  uint16_t initialReading = takeReading();
  Serial.print(F("Initial reading (powered): F4="));
  Serial.println(initialReading);
  Serial.println();

  // Power OFF test
  Serial.println(F("Power OFF test:"));
  bool offResult = as7343.powerOn(false);
  Serial.print(F("  powerOn(false): "));
  if (offResult) {
    Serial.println(F("OK"));
  } else {
    Serial.println(F("FAIL"));
    testPassed = false;
  }

  delay(50);

  // Try to read after power off - sensor may return stale/zero data
  // Note: With power off, the sensor may not respond properly
  // We can't use takeReading() as it may hang, so just note the state
  Serial.println(F("  Reading after power off: (sensor powered down)"));
  Serial.println();

  // Power ON test
  Serial.println(F("Power ON test:"));
  bool onResult = as7343.powerOn(true);
  Serial.print(F("  powerOn(true): "));
  if (onResult) {
    Serial.println(F("OK"));
  } else {
    Serial.println(F("FAIL"));
    testPassed = false;
  }

  delay(100); // Let sensor stabilize

  uint16_t afterOnReading = takeReading();
  Serial.print(F("  Reading after power on: F4="));
  Serial.println(afterOnReading);

  // Verify reading recovered
  if (afterOnReading > 0) {
    Serial.println(F("  Sensor recovered: OK"));
  } else {
    Serial.println(F("  Sensor recovered: FAIL (reading=0)"));
    testPassed = false;
  }
  Serial.println();

  // Low Power Mode test
  Serial.println(F("Low Power Mode test:"));

  bool lpOnResult = as7343.enableLowPower(true);
  Serial.print(F("  enableLowPower(true): "));
  if (lpOnResult) {
    Serial.println(F("OK"));
  } else {
    Serial.println(F("FAIL"));
    testPassed = false;
  }

  delay(50);

  bool lpOffResult = as7343.enableLowPower(false);
  Serial.print(F("  enableLowPower(false): "));
  if (lpOffResult) {
    Serial.println(F("OK"));
  } else {
    Serial.println(F("FAIL"));
    testPassed = false;
  }
  Serial.println();

  // Turn NeoPixels OFF
  setAllPixels(0, 0, 0);

  // Final result
  Serial.print(F("Power control: "));
  Serial.println(testPassed ? F("PASS") : F("FAIL"));
  Serial.print(F("RESULT: "));
  Serial.println(testPassed ? F("PASS") : F("FAIL"));
}

void loop() {
  // Nothing to do
  delay(1000);
}
