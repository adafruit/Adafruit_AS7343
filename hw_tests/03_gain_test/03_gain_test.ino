/*!
 * @file 03_gain_test.ino
 * @brief Hardware test for AS7343 gain settings
 *
 * Tests all 13 gain settings (0.5x to 2048x) and verifies:
 * - Gain readback matches set value
 * - Readings increase with gain (until saturation)
 *
 * Hardware: AS7343 on I2C, NeoPixel ring (16 LEDs) on pin 6
 */

#include <Adafruit_AS7343.h>
#include <Adafruit_NeoPixel.h>

#define NEOPIXEL_PIN 6
#define NEOPIXEL_COUNT 16

Adafruit_AS7343 as7343;
Adafruit_NeoPixel pixels(NEOPIXEL_COUNT, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

// Gain labels for output
const char *gainLabels[] = {"0.5X",  "1X",    "2X",    "4X",   "8X",
                            "16X",   "32X",   "64X",   "128X", "256X",
                            "512X",  "1024X", "2048X"};

// Store readings for each gain
uint16_t readings[13];
uint16_t allChannels[18];

void setup() {
  Serial.begin(115200);
  while (!Serial)
    delay(10);

  Serial.println(F("AS7343 Gain Test"));
  Serial.println(F("================"));

  // Initialize NeoPixels
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

  // Use shorter integration time to avoid saturation at high gains
  as7343.setATIME(9);    // 10 cycles
  as7343.setASTEP(999);  // ~2.78ms step -> ~27ms total

  // Turn NeoPixels ON at moderate brightness
  Serial.println(F("NeoPixels ON (moderate)..."));
  for (int i = 0; i < NEOPIXEL_COUNT; i++) {
    pixels.setPixelColor(i, pixels.Color(64, 64, 64));
  }
  pixels.show();
  delay(100);

  Serial.println();
  Serial.println(F("Gain     F4 Reading"));
  Serial.println(F("----     ----------"));

  bool allReadbacksOK = true;
  int increasingCount = 0;

  // Test all 13 gain settings
  for (uint8_t g = 0; g <= 12; g++) {
    as7343_gain_t gain = (as7343_gain_t)g;

    // Stop any existing measurement
    as7343.stopMeasurement();
    delay(10);

    // Set gain
    as7343.setGain(gain);
    delay(5);

    // Verify readback
    as7343_gain_t readback = as7343.getGain();
    if (readback != gain) {
      allReadbacksOK = false;
    }

    // Start fresh measurement
    as7343.startMeasurement();

    // Wait for data ready (18-channel mode needs ~3 cycles)
    uint16_t timeout = 0;
    while (!as7343.dataReady()) {
      delay(10);
      timeout++;
      if (timeout > 200) {
        Serial.print(gainLabels[g]);
        Serial.println(F("     TIMEOUT"));
        readings[g] = 0;
        continue;
      }
    }

    // Read all channels to get fresh data
    as7343.readAllChannels(allChannels);

    // Get F4 channel (index 8 - green, good response)
    readings[g] = allChannels[AS7343_CHANNEL_F4];

    // Print result
    Serial.print(gainLabels[g]);
    // Pad for alignment
    if (g < 5)
      Serial.print(F("     "));
    else if (g < 10)
      Serial.print(F("    "));
    else
      Serial.print(F("   "));
    Serial.println(readings[g]);

    // Check if reading increased from previous (skip first)
    if (g > 0 && readings[g] > readings[g - 1]) {
      increasingCount++;
    }
  }

  // Turn NeoPixels OFF
  pixels.clear();
  pixels.show();
  Serial.println();

  // Report readback verification
  Serial.print(F("Readback verification: "));
  Serial.println(allReadbacksOK ? F("PASS") : F("FAIL"));

  // Check that 2x > 1x (basic scaling check)
  bool scalingOK = (readings[2] > readings[1]);

  // Check that at least 10 of 12 transitions show increase
  // (allow for saturation at high gains)
  bool trendOK = (increasingCount >= 10);

  Serial.print(F("Gain scaling check: "));
  Serial.println((scalingOK && trendOK) ? F("PASS") : F("FAIL"));

  // Final result
  Serial.print(F("RESULT: "));
  Serial.println((allReadbacksOK && scalingOK && trendOK) ? F("PASS") : F("FAIL"));
}

void loop() {
  // Nothing to do
}
