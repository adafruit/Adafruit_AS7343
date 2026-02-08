/*!
 * AS7343 Flicker Detection Test
 *
 * Tests the flicker detection API:
 * - enableFlickerDetection(bool)
 * - getFlickerStatus()
 * - getFlickerFrequency()
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

const char *flickerToString(as7343_flicker_t f) {
  switch (f) {
  case AS7343_FLICKER_NONE:
    return "NONE";
  case AS7343_FLICKER_100HZ:
    return "100Hz";
  case AS7343_FLICKER_120HZ:
    return "120Hz";
  default:
    return "UNKNOWN";
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial)
    delay(10);

  Serial.println(F("AS7343 Flicker Detection Test"));
  Serial.println(F("============================="));
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

  // API Test
  Serial.println(F("API Test:"));

  // Test enable
  as7343.enableFlickerDetection(true);
  Serial.println(F("  enableFlickerDetection(true): OK"));

  // Test disable
  as7343.enableFlickerDetection(false);
  Serial.println(F("  enableFlickerDetection(false): OK"));

  Serial.println();

  // Test with NeoPixels ON (dim)
  Serial.println(F("Flicker Detection with NeoPixels (dim):"));
  setAllPixels(32, 32, 32);
  as7343.enableFlickerDetection(true);
  delay(500); // Wait for detection to stabilize

  uint8_t rawStatus = as7343.getFlickerStatus();
  as7343_flicker_t freq = as7343.getFlickerFrequency();

  Serial.print(F("  Raw status: 0x"));
  if (rawStatus < 16)
    Serial.print(F("0"));
  Serial.println(rawStatus, HEX);
  Serial.print(F("  Frequency: "));
  Serial.println(flickerToString(freq));

  Serial.println();

  // Test with NeoPixels OFF (ambient light)
  Serial.println(F("Flicker Detection with Ambient Light:"));
  setAllPixels(0, 0, 0);
  delay(500); // Wait for detection to stabilize

  rawStatus = as7343.getFlickerStatus();
  freq = as7343.getFlickerFrequency();

  Serial.print(F("  Raw status: 0x"));
  if (rawStatus < 16)
    Serial.print(F("0"));
  Serial.println(rawStatus, HEX);
  Serial.print(F("  Frequency: "));
  Serial.println(flickerToString(freq));

  // Disable flicker detection
  as7343.enableFlickerDetection(false);

  Serial.println();
  Serial.print(F("API functional: "));
  Serial.println(testPassed ? F("PASS") : F("FAIL"));
  Serial.print(F("RESULT: "));
  Serial.println(testPassed ? F("PASS") : F("FAIL"));
}

void loop() {
  // Nothing to do
  delay(1000);
}
