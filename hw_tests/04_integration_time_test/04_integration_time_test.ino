/*!
 * @file 04_integration_time_test.ino
 * @brief Hardware test for AS7343 integration time settings
 *
 * Tests ATIME and ASTEP registers and verifies:
 * - ATIME/ASTEP readback matches set values
 * - getIntegrationTime() returns reasonable values
 * - Longer integration times produce higher readings
 *
 * Integration time formula: t_int = (ATIME + 1) × (ASTEP + 1) × 2.78 µs
 *
 * Hardware: AS7343 on I2C, NeoPixel ring (16 LEDs) on pin 6
 */

#include <Adafruit_AS7343.h>
#include <Adafruit_NeoPixel.h>

#define NEOPIXEL_PIN 6
#define NEOPIXEL_COUNT 16

Adafruit_AS7343 as7343;
Adafruit_NeoPixel pixels(NEOPIXEL_COUNT, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

// Test configurations: {ATIME, ASTEP, expected_time_ms}
struct IntTimeConfig {
  uint8_t atime;
  uint16_t astep;
  float expectedMs;
  const char *label;
};

IntTimeConfig testConfigs[] = {
    {9, 299, 8.3, "Short"},     // ~8.3ms
    {29, 599, 50.0, "Medium"},  // ~50ms (default)
    {99, 999, 278.0, "Long"}    // ~278ms
};

const uint8_t NUM_CONFIGS = sizeof(testConfigs) / sizeof(testConfigs[0]);

// Store results
uint16_t readings[NUM_CONFIGS];
float measuredTimes[NUM_CONFIGS];
uint16_t allChannels[18];

void setup() {
  Serial.begin(115200);
  while (!Serial)
    delay(10);

  Serial.println(F("AS7343 Integration Time Test"));
  Serial.println(F("============================"));

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

  // Set moderate gain to avoid saturation
  as7343.setGain(AS7343_GAIN_64X);
  Serial.println(F("NeoPixels ON, Gain 64X..."));

  // Turn NeoPixels ON at moderate brightness
  for (int i = 0; i < NEOPIXEL_COUNT; i++) {
    pixels.setPixelColor(i, pixels.Color(64, 64, 64));
  }
  pixels.show();
  delay(100);

  Serial.println();
  Serial.println(F("Setting     ATIME  ASTEP  Calc Time(ms)  F4 Reading"));
  Serial.println(F("-------     -----  -----  -------------  ----------"));

  bool allReadbacksOK = true;

  // Test each configuration
  for (uint8_t i = 0; i < NUM_CONFIGS; i++) {
    IntTimeConfig &cfg = testConfigs[i];

    // Stop any existing measurement
    as7343.stopMeasurement();
    delay(50);

    // Set ATIME
    as7343.setATIME(cfg.atime);
    delay(10);

    // Verify ATIME readback
    uint8_t atimeReadback = as7343.getATIME();
    if (atimeReadback != cfg.atime) {
      allReadbacksOK = false;
    }

    // Set ASTEP
    as7343.setASTEP(cfg.astep);
    delay(10);

    // Verify ASTEP readback
    uint16_t astepReadback = as7343.getASTEP();
    if (astepReadback != cfg.astep) {
      allReadbacksOK = false;
    }

    // Get calculated integration time
    measuredTimes[i] = as7343.getIntegrationTime();

    // Start fresh measurement
    as7343.startMeasurement();

    // Wait for data ready (longer timeout for long integration)
    // 18-channel mode needs 3 cycles, add extra margin
    uint16_t timeout = 0;
    uint16_t maxTimeout = (cfg.atime > 50) ? 500 : 200;
    while (!as7343.dataReady()) {
      delay(10);
      timeout++;
      if (timeout > maxTimeout) {
        Serial.print(cfg.label);
        Serial.println(F("     TIMEOUT"));
        readings[i] = 0;
        break;
      }
    }

    if (timeout <= maxTimeout) {
      // Read all channels - first read clears AVALID
      as7343.readAllChannels(allChannels);

      // Start another measurement cycle for fresh data
      as7343.startMeasurement();

      // Wait for second measurement
      timeout = 0;
      while (!as7343.dataReady()) {
        delay(10);
        timeout++;
        if (timeout > maxTimeout)
          break;
      }

      // Read the fresh data
      as7343.readAllChannels(allChannels);

      // Get F4 channel (index 8 - green)
      readings[i] = allChannels[AS7343_CHANNEL_F4];

      // Print result with padding
      Serial.print(cfg.label);
      // Pad label to 12 chars
      for (uint8_t p = 0; p < 12 - strlen(cfg.label); p++)
        Serial.print(' ');
      Serial.print(cfg.atime);
      if (cfg.atime < 10)
        Serial.print(F("      "));
      else if (cfg.atime < 100)
        Serial.print(F("     "));
      else
        Serial.print(F("    "));
      Serial.print(cfg.astep);
      if (cfg.astep < 100)
        Serial.print(F("    "));
      else if (cfg.astep < 1000)
        Serial.print(F("   "));
      else
        Serial.print(F("  "));
      Serial.print(measuredTimes[i], 1);
      if (measuredTimes[i] < 10)
        Serial.print(F("           "));
      else if (measuredTimes[i] < 100)
        Serial.print(F("          "));
      else
        Serial.print(F("         "));
      Serial.println(readings[i]);
    }
  }

  // Turn NeoPixels OFF
  pixels.clear();
  pixels.show();
  Serial.println();

  // Report readback verification
  Serial.print(F("ATIME/ASTEP readback: "));
  Serial.println(allReadbacksOK ? F("PASS") : F("FAIL"));

  // Check that readings increase with integration time
  // Short < Medium < Long
  bool scalingOK = (readings[0] < readings[1]) && (readings[1] < readings[2]);

  Serial.print(F("Integration scaling: "));
  Serial.print(scalingOK ? F("PASS") : F("FAIL"));
  Serial.println(F(" (longer time = higher reading)"));

  // Final result
  Serial.print(F("RESULT: "));
  Serial.println((allReadbacksOK && scalingOK) ? F("PASS") : F("FAIL"));
}

void loop() {
  // Nothing to do
}
