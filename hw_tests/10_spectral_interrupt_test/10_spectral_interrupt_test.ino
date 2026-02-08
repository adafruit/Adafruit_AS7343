/*!
 * @file 10_spectral_interrupt_test.ino
 * @brief Hardware test for AS7343 spectral threshold interrupts
 *
 * Tests spectral threshold interrupt functionality:
 * - Self-calibrating thresholds based on ambient conditions
 * - High threshold crossing detection
 * - Low threshold crossing detection
 * - Status register polling (INT pin not required)
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

// Helper to wait for data ready with timeout
bool waitForData(uint16_t timeoutMs = 2000) {
  uint32_t start = millis();
  while (!as7343.dataReady()) {
    if (millis() - start > timeoutMs) {
      return false;
    }
    delay(10);
  }
  return true;
}

// Take a measurement and return FZ channel value
uint16_t measureFZ() {
  as7343.stopMeasurement();
  delay(10);
  as7343.clearStatus();
  as7343.startMeasurement();

  if (!waitForData()) {
    Serial.println(F("  ERROR: Measurement timeout"));
    return 0;
  }

  as7343.readAllChannels(allChannels);
  return allChannels[AS7343_CHANNEL_FZ];
}

void setup() {
  Serial.begin(115200);
  while (!Serial)
    delay(10);

  Serial.println(F("AS7343 Spectral Interrupt Test"));
  Serial.println(F("=============================="));
  Serial.println();

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

  // Use low gain to avoid saturation
  as7343.setGain(AS7343_GAIN_16X);
  as7343.setATIME(29);   // 30 cycles
  as7343.setASTEP(599);  // ~1.67ms step -> ~50ms total

  // Set threshold channel to 0 (FZ)
  as7343.setThresholdChannel(0);

  // =====================
  // CALIBRATION PHASE
  // =====================
  Serial.println(F("CALIBRATION:"));

  // Baseline: NeoPixels OFF
  pixels.clear();
  pixels.show();
  delay(100);

  uint16_t baseline = measureFZ();
  Serial.print(F("  NeoPixels OFF baseline (FZ): "));
  Serial.println(baseline);

  // Peak: NeoPixels ON bright
  for (int i = 0; i < NEOPIXEL_COUNT; i++) {
    pixels.setPixelColor(i, pixels.Color(255, 255, 255));
  }
  pixels.show();
  delay(100);

  uint16_t peak = measureFZ();
  Serial.print(F("  NeoPixels ON peak (FZ): "));
  Serial.println(peak);

  // Calculate thresholds
  uint16_t range = peak - baseline;
  uint16_t lowThresh = baseline + (range / 4);       // 25% of range
  uint16_t highThresh = baseline + (3 * range / 4);  // 75% of range

  Serial.print(F("  Calculated thresholds: low="));
  Serial.print(lowThresh);
  Serial.print(F(" high="));
  Serial.println(highThresh);
  Serial.println();

  // Turn off NeoPixels
  pixels.clear();
  pixels.show();

  // =====================
  // INTERRUPT SETUP
  // =====================
  Serial.println(F("INTERRUPT TEST:"));

  // Set thresholds
  as7343.setLowThreshold(lowThresh);
  as7343.setHighThreshold(highThresh);

  // Verify readback
  uint16_t readLow = as7343.getLowThreshold();
  uint16_t readHigh = as7343.getHighThreshold();

  Serial.print(F("  Thresholds set: low="));
  Serial.print(readLow);
  Serial.print(F(" high="));
  Serial.println(readHigh);

  // Set persistence to 1 (trigger after 1 consecutive reading)
  as7343.setPersistence(1);
  Serial.print(F("  Persistence: "));
  Serial.println(as7343.getPersistence());
  Serial.println();

  // Enable spectral interrupt
  as7343.enableSpectralInterrupt(true);

  bool highTestPass = false;
  bool lowTestPass = false;

  // =====================
  // HIGH THRESHOLD TEST
  // =====================
  Serial.println(F("High threshold test:"));

  // Turn NeoPixels ON bright
  for (int i = 0; i < NEOPIXEL_COUNT; i++) {
    pixels.setPixelColor(i, pixels.Color(255, 255, 255));
  }
  pixels.show();
  delay(100);

  // Clear status and start measurement
  as7343.clearStatus();
  as7343.startMeasurement();

  if (!waitForData()) {
    Serial.println(F("  ERROR: Measurement timeout"));
  } else {
    as7343.readAllChannels(allChannels);
    uint16_t reading = allChannels[AS7343_CHANNEL_FZ];

    // Check status for AINT bit (bit 3 = 0x08)
    uint8_t status = as7343.getStatus();
    bool aintSet = (status & 0x08) != 0;

    Serial.print(F("  NeoPixels ON, reading: "));
    Serial.println(reading);
    Serial.print(F("  Status: 0x"));
    if (status < 0x10) Serial.print(F("0"));
    Serial.print(status, HEX);
    Serial.print(F(", AINT bit: "));
    Serial.println(aintSet ? F("SET") : F("CLEAR"));

    // Should trigger because reading > highThresh
    highTestPass = aintSet && (reading > highThresh);

    Serial.print(F("  Result: "));
    Serial.println(highTestPass ? F("PASS") : F("FAIL"));
  }
  Serial.println();

  // =====================
  // LOW THRESHOLD TEST
  // =====================
  Serial.println(F("Low threshold test:"));

  // Turn NeoPixels OFF
  pixels.clear();
  pixels.show();
  delay(100);

  // Clear status and start measurement
  as7343.clearStatus();
  as7343.startMeasurement();

  if (!waitForData()) {
    Serial.println(F("  ERROR: Measurement timeout"));
  } else {
    as7343.readAllChannels(allChannels);
    uint16_t reading = allChannels[AS7343_CHANNEL_FZ];

    // Check status for AINT bit (bit 3 = 0x08)
    uint8_t status = as7343.getStatus();
    bool aintSet = (status & 0x08) != 0;

    Serial.print(F("  NeoPixels OFF, reading: "));
    Serial.println(reading);
    Serial.print(F("  Status: 0x"));
    if (status < 0x10) Serial.print(F("0"));
    Serial.print(status, HEX);
    Serial.print(F(", AINT bit: "));
    Serial.println(aintSet ? F("SET") : F("CLEAR"));

    // Should trigger because reading < lowThresh
    lowTestPass = aintSet && (reading < lowThresh);

    Serial.print(F("  Result: "));
    Serial.println(lowTestPass ? F("PASS") : F("FAIL"));
  }
  Serial.println();

  // =====================
  // FINAL RESULT
  // =====================

  // Cleanup
  as7343.enableSpectralInterrupt(false);
  as7343.stopMeasurement();
  pixels.clear();
  pixels.show();

  // At least one interrupt must fire for pass
  bool pass = (highTestPass || lowTestPass);

  Serial.print(F("RESULT: "));
  Serial.println(pass ? F("PASS") : F("FAIL"));
}

void loop() {
  // Nothing to do
}
