/*!
 * @file 14_saturation_test.ino
 * @brief Hardware test for AS7343 saturation detection
 *
 * Tests saturation detection API:
 * - isAnalogSaturated() - analog circuit saturation
 * - isDigitalSaturated() - ADC counter saturated (reading = 65535)
 *
 * Hardware: AS7343 on I2C, NeoPixel ring (16 LEDs) on pin 6
 */

#include <Adafruit_AS7343.h>
#include <Adafruit_NeoPixel.h>

#define NEOPIXEL_PIN 6
#define NEOPIXEL_COUNT 16

Adafruit_AS7343 as7343;
Adafruit_NeoPixel pixels(NEOPIXEL_COUNT, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

uint16_t allChannels[18];

void setup() {
  Serial.begin(115200);
  while (!Serial)
    delay(10);

  Serial.println(F("AS7343 Saturation Test"));
  Serial.println(F("======================"));
  Serial.println();

  // Initialize NeoPixels (start OFF)
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

  bool apiOK = true;

  // =========================================
  // Test 1: Low light (should NOT saturate)
  // =========================================
  Serial.println(F("Low light test (NeoPixels OFF, 64X gain):"));

  // NeoPixels OFF
  pixels.clear();
  pixels.show();
  delay(100);

  // Moderate gain, moderate integration
  as7343.setGain(AS7343_GAIN_64X);
  as7343.setATIME(29);   // 30 cycles
  as7343.setASTEP(599);  // ~1.7ms step -> ~50ms total

  // Start measurement
  as7343.startMeasurement();

  // Wait for data ready
  uint16_t timeout = 0;
  while (!as7343.dataReady()) {
    delay(10);
    timeout++;
    if (timeout > 200) {
      Serial.println(F("  TIMEOUT waiting for data"));
      apiOK = false;
      break;
    }
  }

  if (timeout <= 200) {
    // Read all channels
    as7343.readAllChannels(allChannels);

    uint16_t lowReading = allChannels[AS7343_CHANNEL_F4];
    bool lowAnalog = as7343.isAnalogSaturated();
    bool lowDigital = as7343.isDigitalSaturated();

    Serial.print(F("  Sample reading (F4): "));
    Serial.println(lowReading);
    Serial.print(F("  Analog saturated: "));
    Serial.println(lowAnalog ? F("YES") : F("NO"));
    Serial.print(F("  Digital saturated: "));
    Serial.println(lowDigital ? F("YES") : F("NO"));

    // Low light should NOT saturate
    if (lowAnalog || lowDigital) {
      Serial.println(F("  WARNING: Saturation in low light!"));
    }
  }

  as7343.stopMeasurement();
  Serial.println();

  // =========================================
  // Test 2: Bright light + high gain (may saturate)
  // =========================================
  Serial.println(
      F("High light test (NeoPixels ON full, 2048X gain, long integration):"));

  // NeoPixels ON full brightness
  for (int i = 0; i < NEOPIXEL_COUNT; i++) {
    pixels.setPixelColor(i, pixels.Color(255, 255, 255));
  }
  pixels.show();
  delay(100);

  // Maximum gain, long integration
  as7343.setGain(AS7343_GAIN_2048X);
  as7343.setATIME(99);   // 100 cycles
  as7343.setASTEP(999);  // ~2.78ms step -> ~278ms total

  // Start measurement
  as7343.startMeasurement();

  // Wait for data ready (longer timeout for long integration)
  timeout = 0;
  while (!as7343.dataReady()) {
    delay(50);
    timeout++;
    if (timeout > 100) {
      Serial.println(F("  TIMEOUT waiting for data"));
      apiOK = false;
      break;
    }
  }

  if (timeout <= 100) {
    // Read all channels
    as7343.readAllChannels(allChannels);

    uint16_t highReading = allChannels[AS7343_CHANNEL_F4];
    bool highAnalog = as7343.isAnalogSaturated();
    bool highDigital = as7343.isDigitalSaturated();

    Serial.print(F("  Sample reading (F4): "));
    Serial.println(highReading);
    Serial.print(F("  Analog saturated: "));
    Serial.println(highAnalog ? F("YES") : F("NO"));
    Serial.print(F("  Digital saturated: "));
    Serial.println(highDigital ? F("YES") : F("NO"));

    // Digital saturation = reading of 65535
    if (highReading == 65535) {
      Serial.println(F("  (Reading at max = digital saturation expected)"));
    }
  }

  as7343.stopMeasurement();
  Serial.println();

  // Turn NeoPixels OFF
  pixels.clear();
  pixels.show();

  // =========================================
  // Results
  // =========================================
  Serial.print(F("API functional: "));
  Serial.println(apiOK ? F("PASS") : F("FAIL"));

  Serial.print(F("RESULT: "));
  Serial.println(apiOK ? F("PASS") : F("FAIL"));
}

void loop() {
  // Nothing to do
}
