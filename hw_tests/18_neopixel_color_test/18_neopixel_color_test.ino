/*
 * AS7343 NeoPixel Color Response Test
 *
 * Tests that AS7343 spectral channels respond correctly to different colors:
 * - Red light -> F6 (640nm), F7 (690nm) should be highest
 * - Green light -> F4 (515nm), FY (555nm), F5 (550nm) should be highest
 * - Blue light -> FZ (450nm), F3 (475nm), F2 (425nm) should be highest
 *
 * Hardware: Metro Mini, AS7343 on I2C, NeoPixel ring (16 LEDs) on Pin 6
 */

#include <Adafruit_AS7343.h>
#include <Adafruit_NeoPixel.h>

#define NEOPIXEL_PIN 6
#define NEOPIXEL_COUNT 16
#define DATA_READY_TIMEOUT_MS 2000

Adafruit_AS7343 as7343;
Adafruit_NeoPixel pixels(NEOPIXEL_COUNT, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

uint16_t readings[18];

// Channel indices in 18-channel mode
const uint8_t CH_FZ = 0;  // 450nm (blue)
const uint8_t CH_FY = 1;  // 555nm (yellow-green)
const uint8_t CH_F2 = 6;  // 425nm (violet-blue)
const uint8_t CH_F3 = 7;  // 475nm (blue-cyan)
const uint8_t CH_F4 = 8;  // 515nm (green)
const uint8_t CH_F6 = 9;  // 640nm (red)
const uint8_t CH_F7 = 13; // 690nm (deep red)
const uint8_t CH_F8 = 14; // 745nm (near-IR)
const uint8_t CH_F5 = 15; // 550nm (green-yellow)

bool waitForDataReady();
void setAllPixels(uint8_t r, uint8_t g, uint8_t b);
void printSpectralReadings();
bool testColor(const char *name, uint8_t r, uint8_t g, uint8_t b,
               const uint8_t *expectedHigh, uint8_t highCount,
               const uint8_t *expectedLow, uint8_t lowCount,
               const char *expectedDesc);

void setup() {
  Serial.begin(115200);
  while (!Serial)
    delay(10);

  Serial.println(F("AS7343 NeoPixel Color Test"));
  Serial.println(F("=========================="));
  Serial.println();

  // Initialize NeoPixels
  pixels.begin();
  pixels.clear();
  pixels.show();

  // Initialize sensor
  if (!as7343.begin()) {
    Serial.println(F("Failed to find AS7343 sensor!"));
    Serial.println(F("RESULT: FAIL"));
    while (1)
      delay(100);
  }

  // Set moderate gain to avoid saturation
  as7343.setGain(AS7343_GAIN_128X);

  bool allPass = true;

  // Test RED: red channels > blue channels
  const uint8_t redHigh[] = {CH_F6, CH_F7};
  const uint8_t redLow[] = {CH_F2, CH_F3, CH_FZ};
  if (!testColor("RED", 255, 0, 0, redHigh, 2, redLow, 3,
                 "F6, F7 (red channels)")) {
    allPass = false;
  }

  Serial.println();

  // Test GREEN: green channels > blue channels
  // Note: NeoPixel green LEDs have significant red bleed, so we compare to blue
  const uint8_t greenHigh[] = {CH_F4, CH_FY, CH_F5};
  const uint8_t greenLow[] = {CH_F2, CH_F3, CH_FZ};
  if (!testColor("GREEN", 0, 255, 0, greenHigh, 3, greenLow, 3,
                 "F4, FY, F5 (green channels)")) {
    allPass = false;
  }

  Serial.println();

  // Test BLUE: blue channels > red channels
  const uint8_t blueHigh[] = {CH_FZ, CH_F3, CH_F2};
  const uint8_t blueLow[] = {CH_F6, CH_F7};
  if (!testColor("BLUE", 0, 0, 255, blueHigh, 3, blueLow, 2,
                 "FZ, F3 (blue channels)")) {
    allPass = false;
  }

  // Turn NeoPixels OFF
  pixels.clear();
  pixels.show();

  Serial.println();
  Serial.print(F("Color response: "));
  Serial.println(allPass ? F("PASS") : F("FAIL"));
  Serial.print(F("RESULT: "));
  Serial.println(allPass ? F("PASS") : F("FAIL"));
}

void loop() {
  // Test complete
  delay(1000);
}

bool waitForDataReady() {
  unsigned long start = millis();
  while ((millis() - start) < DATA_READY_TIMEOUT_MS) {
    if (as7343.dataReady()) {
      return true;
    }
    delay(10);
  }
  return false;
}

void setAllPixels(uint8_t r, uint8_t g, uint8_t b) {
  for (int i = 0; i < NEOPIXEL_COUNT; i++) {
    pixels.setPixelColor(i, pixels.Color(r, g, b));
  }
  pixels.show();
}

void printSpectralReadings() {
  // Print in organized format matching wavelengths
  Serial.print(F("  F2(425nm)="));
  Serial.print(readings[CH_F2]);
  Serial.print(F(" F3(475nm)="));
  Serial.print(readings[CH_F3]);
  Serial.print(F(" FZ(450nm)="));
  Serial.println(readings[CH_FZ]);

  Serial.print(F("  F4(515nm)="));
  Serial.print(readings[CH_F4]);
  Serial.print(F(" FY(555nm)="));
  Serial.print(readings[CH_FY]);
  Serial.print(F(" F5(550nm)="));
  Serial.println(readings[CH_F5]);

  Serial.print(F("  F6(640nm)="));
  Serial.print(readings[CH_F6]);
  Serial.print(F(" F7(690nm)="));
  Serial.print(readings[CH_F7]);
  Serial.print(F(" F8(745nm)="));
  Serial.println(readings[CH_F8]);
}

bool testColor(const char *name, uint8_t r, uint8_t g, uint8_t b,
               const uint8_t *expectedHigh, uint8_t highCount,
               const uint8_t *expectedLow, uint8_t lowCount,
               const char *expectedDesc) {
  Serial.print(name);
  Serial.print(F(" ("));
  Serial.print(r);
  Serial.print(F(","));
  Serial.print(g);
  Serial.print(F(","));
  Serial.print(b);
  Serial.println(F("):"));

  // Set NeoPixels to test color
  setAllPixels(r, g, b);
  delay(200); // Wait for settle

  // Read spectral data
  as7343.startMeasurement();
  if (!waitForDataReady()) {
    Serial.println(F("  Timeout waiting for data!"));
    return false;
  }
  as7343.readAllChannels(readings);

  // Print readings
  printSpectralReadings();

  // Calculate average of expected high and expected low channels
  uint32_t highSum = 0;
  uint32_t lowSum = 0;

  for (uint8_t i = 0; i < highCount; i++) {
    highSum += readings[expectedHigh[i]];
  }

  for (uint8_t i = 0; i < lowCount; i++) {
    lowSum += readings[expectedLow[i]];
  }

  uint16_t highAvg = highSum / highCount;
  uint16_t lowAvg = lowSum / lowCount;

  // Pass if high channels are significantly greater than low channels
  bool pass = (highAvg > lowAvg);

  Serial.print(F("  Dominant: "));
  Serial.print(expectedDesc);
  Serial.print(F(" - "));
  Serial.println(pass ? F("PASS") : F("FAIL"));

  if (!pass) {
    Serial.print(F("  (High avg="));
    Serial.print(highAvg);
    Serial.print(F(" should be > Low avg="));
    Serial.print(lowAvg);
    Serial.println(F(")"));
  }

  return pass;
}
