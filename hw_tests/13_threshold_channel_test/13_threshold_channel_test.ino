/*!
 * @file 13_threshold_channel_test.ino
 * @brief Hardware test for AS7343 threshold channel register
 *
 * Tests setThresholdChannel() and getThresholdChannel() register access.
 *
 * NOTE: Per hardware testing, the SP_TH_CH register does NOT affect
 * threshold comparison or persistence behavior - comparison is ALWAYS
 * on CH0 regardless of this setting. The datasheet claims SP_TH_CH
 * controls the persistence filter channel, but this is not observed
 * in practice. This test verifies register read/write works and that
 * threshold comparison on CH0 functions correctly.
 *
 * Hardware: AS7343 on I2C, NeoPixel ring (16 LEDs) on pin 6
 * Board: Metro Mini (arduino:avr:uno)
 */

#include <Adafruit_AS7343.h>
#include <Adafruit_NeoPixel.h>

#define NEOPIXEL_PIN 6
#define NEOPIXEL_COUNT 16
#define MAX_CYCLES 25

Adafruit_AS7343 as7343;
Adafruit_NeoPixel pixels(NEOPIXEL_COUNT, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

bool readbackPassed = true;
bool hardwarePassed = true;

uint8_t countCyclesToAINT() {
  uint16_t buf[6];
  uint8_t cycles = 0;
  while (cycles < MAX_CYCLES) {
    while (!as7343.dataReady())
      delay(1);
    as7343.readAllChannels(buf);
    cycles++;
    if (as7343.getStatus() & 0x08)
      break;
  }
  return cycles;
}

void setup() {
  Serial.begin(115200);
  while (!Serial)
    delay(10);

  Serial.println(F("AS7343 Threshold Channel Test"));
  Serial.println(F("============================="));
  Serial.println();

  pixels.begin();
  pixels.clear();
  pixels.show();

  if (!as7343.begin()) {
    Serial.println(F("ERROR: AS7343 not found!"));
    Serial.println(F("RESULT: FAIL"));
    while (1)
      delay(10);
  }

  // ==========================================
  // Part 1: Register Readback
  // ==========================================
  Serial.println(F("Part 1: Register Readback"));
  Serial.println(F("Channel  Readback   Status"));
  Serial.println(F("-------  --------   ------"));

  for (uint8_t ch = 0; ch <= 5; ch++) {
    as7343.setThresholdChannel(ch);
    uint8_t readback = as7343.getThresholdChannel();
    Serial.print(ch);
    Serial.print(F("        "));
    Serial.print(readback);
    Serial.print(F("          "));
    if (readback == ch) {
      Serial.println(F("PASS"));
    } else {
      Serial.println(F("FAIL"));
      readbackPassed = false;
    }
  }
  Serial.println();
  Serial.print(F("Register readback: "));
  Serial.println(readbackPassed ? F("PASS") : F("FAIL"));
  Serial.println();

  // ==========================================
  // Part 2: Threshold Comparison (CH0 only)
  // ==========================================
  // NOTE: SP_TH_CH does not affect comparison - always uses CH0
  Serial.println(F("Part 2: Threshold Comparison (always CH0)"));
  Serial.println();

  as7343.setSMUXMode(AS7343_SMUX_6CH);
  as7343.setGain(AS7343_GAIN_4X);
  as7343.setATIME(29);
  as7343.setASTEP(599);

  // Calibration: NeoPixels OFF
  Serial.println(F("Calibration:"));
  pixels.clear();
  pixels.show();
  delay(100);

  as7343.startMeasurement();
  while (!as7343.dataReady())
    delay(1);
  uint16_t ch[6];
  as7343.readAllChannels(ch);
  as7343.stopMeasurement();
  uint16_t ch0_off = ch[0];

  Serial.print(F("  NeoPixels OFF - Ch0: "));
  Serial.println(ch0_off);

  // NeoPixels ON
  for (int i = 0; i < NEOPIXEL_COUNT; i++) {
    pixels.setPixelColor(i, 0, 0, 255);
  }
  pixels.show();
  delay(100);

  as7343.startMeasurement();
  while (!as7343.dataReady())
    delay(1);
  as7343.readAllChannels(ch);
  as7343.stopMeasurement();
  uint16_t ch0_on = ch[0];

  Serial.print(F("  NeoPixels ON  - Ch0: "));
  Serial.println(ch0_on);

  // Set threshold between OFF and ON values
  uint16_t threshold = ch0_off + (ch0_on - ch0_off) / 3;
  Serial.print(F("  Threshold: "));
  Serial.println(threshold);
  Serial.println();

  as7343.enableSpectralInterrupt(false);
  as7343.setLowThreshold(0);
  as7343.setHighThreshold(threshold);
  as7343.setPersistence(4);
  as7343.setThresholdChannel(0);

  // Test A: NeoPixels ON -> Ch0 above threshold -> should trigger
  Serial.println(F("Test A: NeoPixels ON (Ch0 > threshold)"));

  as7343.clearStatus();
  as7343.enableSpectralInterrupt(true);
  as7343.startMeasurement();

  uint8_t cyclesA = countCyclesToAINT();

  as7343.stopMeasurement();
  as7343.enableSpectralInterrupt(false);

  bool passA = (cyclesA < MAX_CYCLES);
  Serial.print(F("  Cycles to AINT: "));
  Serial.print(cyclesA);
  Serial.println(passA ? F(" PASS") : F(" FAIL"));

  // Test B: NeoPixels OFF -> Ch0 below threshold -> should NOT trigger
  Serial.println(F("Test B: NeoPixels OFF (Ch0 < threshold)"));

  pixels.clear();
  pixels.show();
  delay(100);

  as7343.clearStatus();
  as7343.enableSpectralInterrupt(true);
  as7343.startMeasurement();

  uint8_t cyclesB = countCyclesToAINT();

  as7343.stopMeasurement();
  as7343.enableSpectralInterrupt(false);

  bool passB = (cyclesB >= MAX_CYCLES);
  Serial.print(F("  Cycles to AINT: "));
  Serial.print(cyclesB);
  Serial.println(passB ? F(" (timeout) PASS") : F(" FAIL"));

  pixels.clear();
  pixels.show();

  hardwarePassed = passA && passB;

  // ==========================================
  // Summary
  // ==========================================
  Serial.println();
  Serial.println(F("Verification:"));
  Serial.print(F("  a) Light ON triggers AINT:  "));
  Serial.println(passA ? F("PASS") : F("FAIL"));
  Serial.print(F("  b) Light OFF no trigger:    "));
  Serial.println(passB ? F("PASS") : F("FAIL"));

  Serial.println();
  Serial.println(F("Note: SP_TH_CH register r/w works but does NOT"));
  Serial.println(F("      affect threshold comparison (always CH0)"));

  Serial.println();
  Serial.print(F("Register readback: "));
  Serial.println(readbackPassed ? F("PASS") : F("FAIL"));
  Serial.print(F("Hardware test:     "));
  Serial.println(hardwarePassed ? F("PASS") : F("FAIL"));

  Serial.print(F("RESULT: "));
  Serial.println((readbackPassed && hardwarePassed) ? F("PASS") : F("FAIL"));
}

void loop() { delay(1000); }
