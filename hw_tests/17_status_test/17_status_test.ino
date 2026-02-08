/*!
 * AS7343 Hardware Test #17: Status Register Test
 *
 * Tests getStatus() and clearStatus() functions.
 *
 * Hardware:
 * - AS7343 on I2C
 * - NeoPixel ring (16 LEDs) on Pin 6
 */

#include <Adafruit_AS7343.h>
#include <Adafruit_NeoPixel.h>

#define NEOPIXEL_PIN 6
#define NEOPIXEL_COUNT 16

Adafruit_AS7343 as7343;
Adafruit_NeoPixel pixels(NEOPIXEL_COUNT, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

void printStatusBits(uint8_t status) {
  Serial.print(F("  Raw: 0x"));
  if (status < 0x10)
    Serial.print('0');
  Serial.println(status, HEX);

  Serial.print(F("  Bits: SINT="));
  Serial.print((status & 0x01) ? 1 : 0);
  Serial.print(F(" FINT="));
  Serial.print((status & 0x04) ? 1 : 0);
  Serial.print(F(" AINT="));
  Serial.print((status & 0x08) ? 1 : 0);
  Serial.print(F(" CINT="));
  Serial.print((status & 0x20) ? 1 : 0);
  Serial.print(F(" ASAT="));
  Serial.println((status & 0x80) ? 1 : 0);
}

void setNeoPixels(bool on) {
  uint32_t color = on ? pixels.Color(255, 255, 255) : pixels.Color(0, 0, 0);
  for (int i = 0; i < NEOPIXEL_COUNT; i++) {
    pixels.setPixelColor(i, color);
  }
  pixels.show();
}

void setup() {
  Serial.begin(115200);
  while (!Serial)
    delay(10);
  delay(500);

  Serial.println(F("AS7343 Status Test"));
  Serial.println(F("=================="));
  Serial.println();

  // Initialize NeoPixels
  pixels.begin();
  pixels.setBrightness(255);
  setNeoPixels(false);

  // Initialize sensor
  if (!as7343.begin()) {
    Serial.println(F("Failed to find AS7343!"));
    Serial.println(F("RESULT: FAIL"));
    while (1)
      delay(10);
  }

  bool testPass = true;
  uint8_t status;

  // --- Initial status read ---
  Serial.println(F("Initial status:"));
  status = as7343.getStatus();
  printStatusBits(status);
  Serial.println();

  // --- Generate activity to set status bits ---
  Serial.println(F("After measurement (high gain + bright light):"));

  // Turn NeoPixels ON bright
  setNeoPixels(true);
  delay(50);

  // Set high gain to potentially saturate
  as7343.setGain(AS7343_GAIN_2048X);

  // Enable spectral interrupt with low threshold
  as7343.setLowThreshold(10);   // Very low threshold
  as7343.setHighThreshold(100); // Low high threshold
  as7343.enableSpectralInterrupt(true);

  // Take a measurement
  as7343.startMeasurement();
  delay(100);
  while (!as7343.dataReady()) {
    delay(10);
  }

  // Read status - should have some bits set
  status = as7343.getStatus();
  printStatusBits(status);
  Serial.println();

  // --- Clear status ---
  Serial.println(F("After clearStatus():"));
  bool clearResult = as7343.clearStatus();
  if (!clearResult) {
    Serial.println(F("  clearStatus() returned false!"));
    testPass = false;
  }
  status = as7343.getStatus();
  printStatusBits(status);
  Serial.println();

  // --- Cleanup ---
  setNeoPixels(false);
  as7343.enableSpectralInterrupt(false);

  // --- Final results ---
  Serial.print(F("Status API: "));
  Serial.println(testPass ? F("PASS") : F("FAIL"));
  Serial.print(F("RESULT: "));
  Serial.println(testPass ? F("PASS") : F("FAIL"));
}

void loop() {
  // Nothing to do
}
