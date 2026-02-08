/*!
 * @file 10_spectral_interrupt_test.ino
 * @brief Hardware test for AS7343 spectral threshold interrupts
 *
 * Tests spectral threshold interrupt functionality:
 * - Self-calibrating thresholds based on ambient conditions
 * - High threshold crossing detection
 * - Low threshold crossing detection
 * - Status register AINT bit verification
 * - Physical INT pin verification (active low)
 *
 * Hardware: AS7343 on I2C, NeoPixel ring (16 LEDs) on pin 6
 *           AS7343 INT pin connected to Arduino pin 2
 * Board: Metro Mini (arduino:avr:uno)
 */

#include <Adafruit_AS7343.h>
#include <Adafruit_NeoPixel.h>

#define NEOPIXEL_PIN 6
#define NEOPIXEL_COUNT 16
#define INT_PIN 2 // AS7343 INT pin connected here (active low)

Adafruit_AS7343 as7343;
Adafruit_NeoPixel pixels(NEOPIXEL_COUNT, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

uint16_t allChannels[18];
bool statusTestPass = true;
bool intPinTestPass = true;

// Helper to wait for data ready with timeout
bool waitForData(uint16_t timeoutMs = 2000) {
  uint32_t start = millis();
  while (!as7343.dataReady()) {
    if (millis() - start > timeoutMs)
      return false;
    delay(10);
  }
  return true;
}

// Take a measurement and return CH0 value
uint16_t measureCH0() {
  as7343.stopMeasurement();
  delay(10);
  as7343.clearStatus();
  as7343.startMeasurement();

  if (!waitForData()) {
    Serial.println(F("  ERROR: Measurement timeout"));
    return 0;
  }

  as7343.readAllChannels(allChannels);
  return allChannels[0]; // CH0 in 6-channel mode
}

void setup() {
  Serial.begin(115200);
  while (!Serial)
    delay(10);

  Serial.println(F("AS7343 Spectral Interrupt Test"));
  Serial.println(F("=============================="));
  Serial.println(F("INT pin: D2 (active low)"));
  Serial.println();

  // Initialize INT pin as input with pull-up
  pinMode(INT_PIN, INPUT_PULLUP);

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

  // Use 6-channel mode, low gain
  as7343.setSMUXMode(AS7343_SMUX_6CH);
  as7343.setGain(AS7343_GAIN_4X);
  as7343.setATIME(29);
  as7343.setASTEP(599);

  // =====================
  // CALIBRATION
  // =====================
  Serial.println(F("Calibration:"));

  // Baseline: NeoPixels OFF
  pixels.clear();
  pixels.show();
  delay(100);

  uint16_t baseline = measureCH0();
  Serial.print(F("  NeoPixels OFF (CH0): "));
  Serial.println(baseline);

  // Peak: NeoPixels ON
  for (int i = 0; i < NEOPIXEL_COUNT; i++) {
    pixels.setPixelColor(i, 0, 0, 255); // Blue
  }
  pixels.show();
  delay(100);

  uint16_t peak = measureCH0();
  Serial.print(F("  NeoPixels ON  (CH0): "));
  Serial.println(peak);

  // Calculate thresholds
  uint16_t range = peak - baseline;
  uint16_t lowThresh = baseline + (range / 4);
  uint16_t highThresh = baseline + (3 * range / 4);

  Serial.print(F("  Low threshold:  "));
  Serial.println(lowThresh);
  Serial.print(F("  High threshold: "));
  Serial.println(highThresh);
  Serial.println();

  // =====================
  // SETUP INTERRUPTS
  // =====================
  as7343.setLowThreshold(lowThresh);
  as7343.setHighThreshold(highThresh);
  as7343.setPersistence(0); // Immediate
  as7343.setThresholdChannel(0);

  // Clear any pending interrupts
  as7343.clearStatus();
  delay(10);

  // Check INT pin is HIGH (no interrupt) after clear
  bool intPinIdle = digitalRead(INT_PIN);
  Serial.print(F("INT pin after clear: "));
  Serial.println(intPinIdle ? F("HIGH (good)") : F("LOW (unexpected)"));
  if (!intPinIdle) {
    Serial.println(F("  Warning: INT pin stuck low"));
  }
  Serial.println();

  // Enable spectral interrupt
  as7343.enableSpectralInterrupt(true);

  // =====================
  // HIGH THRESHOLD TEST
  // =====================
  Serial.println(F("High threshold test (NeoPixels ON):"));

  // NeoPixels ON
  for (int i = 0; i < NEOPIXEL_COUNT; i++) {
    pixels.setPixelColor(i, 0, 0, 255);
  }
  pixels.show();
  delay(50);

  // Start measurement, prime pipeline, then check
  as7343.clearStatus();
  as7343.startMeasurement();

  // First cycle - prime
  if (!waitForData()) {
    Serial.println(F("  ERROR: Timeout"));
    statusTestPass = false;
  }
  as7343.readAllChannels(allChannels);
  as7343.clearStatus();

  // Second cycle - actual test
  if (!waitForData()) {
    Serial.println(F("  ERROR: Timeout"));
    statusTestPass = false;
  } else {
    as7343.readAllChannels(allChannels);
    uint16_t reading = allChannels[0];

    uint8_t status = as7343.getStatus();
    bool aintSet = (status & 0x08) != 0;
    bool intPinLow = !digitalRead(INT_PIN);

    Serial.print(F("  CH0 reading: "));
    Serial.print(reading);
    Serial.print(F(" (threshold: "));
    Serial.print(highThresh);
    Serial.println(F(")"));

    Serial.print(F("  Status AINT: "));
    Serial.print(aintSet ? F("SET") : F("CLEAR"));
    bool statusOK = aintSet && (reading > highThresh);
    Serial.println(statusOK ? F(" PASS") : F(" FAIL"));
    if (!statusOK)
      statusTestPass = false;

    Serial.print(F("  INT pin:     "));
    Serial.print(intPinLow ? F("LOW (active)") : F("HIGH"));
    Serial.println(intPinLow ? F(" PASS") : F(" FAIL"));
    if (!intPinLow)
      intPinTestPass = false;

    // Verify clear works
    as7343.clearStatus();
    delay(5);
    bool intPinAfterClear = digitalRead(INT_PIN);
    Serial.print(F("  After clear: "));
    Serial.print(intPinAfterClear ? F("HIGH") : F("LOW"));
    Serial.println(intPinAfterClear ? F(" PASS") : F(" FAIL"));
    if (!intPinAfterClear)
      intPinTestPass = false;
  }

  as7343.stopMeasurement();
  Serial.println();

  // =====================
  // LOW THRESHOLD TEST
  // =====================
  Serial.println(F("Low threshold test (NeoPixels OFF):"));

  // NeoPixels OFF
  pixels.clear();
  pixels.show();
  delay(50);

  // Start measurement, prime pipeline, then check
  as7343.clearStatus();
  as7343.startMeasurement();

  // First cycle - prime
  if (!waitForData()) {
    Serial.println(F("  ERROR: Timeout"));
    statusTestPass = false;
  }
  as7343.readAllChannels(allChannels);
  as7343.clearStatus();

  // Second cycle - actual test
  if (!waitForData()) {
    Serial.println(F("  ERROR: Timeout"));
    statusTestPass = false;
  } else {
    as7343.readAllChannels(allChannels);
    uint16_t reading = allChannels[0];

    uint8_t status = as7343.getStatus();
    bool aintSet = (status & 0x08) != 0;
    bool intPinLow = !digitalRead(INT_PIN);

    Serial.print(F("  CH0 reading: "));
    Serial.print(reading);
    Serial.print(F(" (threshold: "));
    Serial.print(lowThresh);
    Serial.println(F(")"));

    Serial.print(F("  Status AINT: "));
    Serial.print(aintSet ? F("SET") : F("CLEAR"));
    bool statusOK = aintSet && (reading < lowThresh);
    Serial.println(statusOK ? F(" PASS") : F(" FAIL"));
    if (!statusOK)
      statusTestPass = false;

    Serial.print(F("  INT pin:     "));
    Serial.print(intPinLow ? F("LOW (active)") : F("HIGH"));
    Serial.println(intPinLow ? F(" PASS") : F(" FAIL"));
    if (!intPinLow)
      intPinTestPass = false;

    // Verify clear works
    as7343.clearStatus();
    delay(5);
    bool intPinAfterClear = digitalRead(INT_PIN);
    Serial.print(F("  After clear: "));
    Serial.print(intPinAfterClear ? F("HIGH") : F("LOW"));
    Serial.println(intPinAfterClear ? F(" PASS") : F(" FAIL"));
    if (!intPinAfterClear)
      intPinTestPass = false;
  }

  as7343.stopMeasurement();
  Serial.println();

  // =====================
  // SUMMARY
  // =====================
  as7343.enableSpectralInterrupt(false);
  pixels.clear();
  pixels.show();

  Serial.println(F("Summary:"));
  Serial.print(F("  Status register: "));
  Serial.println(statusTestPass ? F("PASS") : F("FAIL"));
  Serial.print(F("  INT pin:         "));
  Serial.println(intPinTestPass ? F("PASS") : F("FAIL"));

  bool allPass = statusTestPass && intPinTestPass;
  Serial.println();
  Serial.print(F("RESULT: "));
  Serial.println(allPass ? F("PASS") : F("FAIL"));
}

void loop() { delay(1000); }
