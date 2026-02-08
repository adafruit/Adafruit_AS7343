/*!
 * @file 09_wait_time_test.ino
 * @brief Hardware test for AS7343 wait time - measures actual timing
 *
 * Tests that WTIME actually affects measurement cycle duration:
 * 1. Register readback verification
 * 2. Timing test: measures real cycle time with wait disabled vs enabled
 *    at different WTIME values and checks the delta matches expected wait
 *
 * Wait time formula: t_wait = (WTIME + 1) x 2.78 ms
 * Range: 2.78ms (WTIME=0) to 711.68ms (WTIME=255)
 *
 * Hardware: AS7343 on I2C, NeoPixel ring (16 LEDs) on pin 6
 */

#include <Adafruit_AS7343.h>
#include <Adafruit_NeoPixel.h>

#define NEOPIXEL_PIN 6
#define NEOPIXEL_COUNT 16
#define NUM_TIMING_SAMPLES 3

Adafruit_AS7343 as7343;
Adafruit_NeoPixel pixels(NEOPIXEL_COUNT, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

// Calculate expected wait time in ms from WTIME register value
float calcWaitTime(uint8_t wtime) { return (wtime + 1) * 2.78; }

// Measure time between consecutive readings in continuous mode (ms).
// Wait time only applies BETWEEN cycles, so we need to let it run
// continuously and time from one dataReady to the next.
float measureCycleTime() {
  uint16_t buf[6];

  as7343.stopMeasurement();
  delay(5);

  // Start continuous measurement
  as7343.startMeasurement();

  // Wait for first reading (discard - primes the pipeline)
  while (!as7343.dataReady()) {
    delay(1);
  }
  as7343.readAllChannels(buf);

  // Now time from this reading to the NEXT one
  unsigned long start = millis();
  while (!as7343.dataReady()) {
    delayMicroseconds(500);
  }
  unsigned long elapsed = millis() - start;
  as7343.readAllChannels(buf);

  as7343.stopMeasurement();
  return (float)elapsed;
}

// Average cycle time over multiple samples
float averageCycleTime() {
  float total = 0;
  for (int i = 0; i < NUM_TIMING_SAMPLES; i++) {
    total += measureCycleTime();
  }
  return total / NUM_TIMING_SAMPLES;
}

void setup() {
  Serial.begin(115200);
  while (!Serial)
    delay(10);

  Serial.println(F("AS7343 Wait Time Test"));
  Serial.println(F("====================="));
  Serial.println();

  // Initialize NeoPixels and keep OFF
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

  // Use 6-channel mode and short integration for fast baseline
  as7343.setSMUXMode(AS7343_SMUX_6CH);
  as7343.setATIME(0);
  as7343.setASTEP(599); // ~1.67ms integration

  bool allReadbacksOK = true;
  bool timingTestOK = true;

  // ==========================================
  // Test 1: Wait Time Readback
  // ==========================================
  const uint8_t testWTIME[] = {0, 50, 100, 255};
  const uint8_t numTests = sizeof(testWTIME) / sizeof(testWTIME[0]);

  Serial.println(F("Register Readback:"));
  Serial.println(F("WTIME  Readback  Expected(ms)  Status"));
  Serial.println(F("-----  --------  ------------  ------"));

  for (uint8_t i = 0; i < numTests; i++) {
    uint8_t wtime = testWTIME[i];
    as7343.setWaitTime(wtime);
    delay(10);
    uint8_t readback = as7343.getWaitTime();
    float expected = calcWaitTime(wtime);
    bool pass = (readback == wtime);
    if (!pass)
      allReadbacksOK = false;

    Serial.print(wtime < 100 ? (wtime < 10 ? "  " : " ") : "");
    Serial.print(wtime);
    Serial.print(F("      "));
    Serial.print(readback < 100 ? (readback < 10 ? "  " : " ") : "");
    Serial.print(readback);
    Serial.print(F("      "));
    Serial.print(expected, 1);
    Serial.print(expected < 100 ? (expected < 10 ? "         " : "        ") : "       ");
    Serial.println(pass ? F("PASS") : F("FAIL"));
  }
  Serial.println();

  // ==========================================
  // Test 2: Actual Timing Measurement
  // ==========================================
  // We measure cycle-to-cycle time in continuous mode with wait off,
  // then with two different WTIME values. We check:
  //   a) Wait ON is slower than wait OFF
  //   b) 2x WTIME gives roughly 2x the added delay (proportional)
  Serial.println(F("Timing Measurement (averaging 3 cycles each):"));
  Serial.println();

  // Baseline: wait disabled
  as7343.enableWait(false);
  as7343.setWaitTime(0);
  float baselineMs = averageCycleTime();
  Serial.print(F("  Baseline (wait OFF):    "));
  Serial.print(baselineMs, 1);
  Serial.println(F(" ms"));

  // Short wait: WTIME=5
  const uint8_t wtime_short = 5;
  as7343.setWaitTime(wtime_short);
  as7343.enableWait(true);
  float shortMs = averageCycleTime();
  float shortDelta = shortMs - baselineMs;

  Serial.print(F("  WTIME="));
  Serial.print(wtime_short);
  Serial.print(F("   (wait ON):    "));
  Serial.print(shortMs, 1);
  Serial.print(F(" ms  (delta +"));
  Serial.print(shortDelta, 1);
  Serial.println(F(" ms)"));

  // Longer wait: WTIME=10
  const uint8_t wtime_long = 10;
  as7343.setWaitTime(wtime_long);
  as7343.enableWait(true);
  float longMs = averageCycleTime();
  float longDelta = longMs - baselineMs;

  Serial.print(F("  WTIME="));
  Serial.print(wtime_long);
  Serial.print(F("  (wait ON):    "));
  Serial.print(longMs, 1);
  Serial.print(F(" ms  (delta +"));
  Serial.print(longDelta, 1);
  Serial.println(F(" ms)"));
  Serial.println();

  // Check a) wait adds measurable time (at least 10ms above baseline)
  bool waitAddsTime = (shortDelta > 10.0);
  Serial.print(F("  Wait adds time:        "));
  Serial.println(waitAddsTime ? F("PASS") : F("FAIL"));
  if (!waitAddsTime)
    timingTestOK = false;

  // Check b) longer WTIME is actually slower
  bool orderPass = (longDelta > shortDelta);
  Serial.print(F("  Longer WTIME slower:   "));
  Serial.println(orderPass ? F("PASS") : F("FAIL"));
  if (!orderPass)
    timingTestOK = false;

  // Check c) proportionality - WTIME=10 delta should be roughly
  // (10+1)/(5+1) = 1.83x the WTIME=5 delta. Allow wide tolerance (1.2-2.5x)
  float expectedRatio = (float)(wtime_long + 1) / (float)(wtime_short + 1);
  float actualRatio = longDelta / shortDelta;
  bool ratioPass = (actualRatio > expectedRatio * 0.65) &&
                   (actualRatio < expectedRatio * 1.35);
  Serial.print(F("  Proportional (ratio "));
  Serial.print(actualRatio, 2);
  Serial.print(F(" vs expected ~"));
  Serial.print(expectedRatio, 2);
  Serial.print(F("): "));
  Serial.println(ratioPass ? F("PASS") : F("FAIL"));
  if (!ratioPass)
    timingTestOK = false;

  // Clean up
  as7343.enableWait(false);
  as7343.stopMeasurement();

  // ==========================================
  // Summary
  // ==========================================
  Serial.println();
  Serial.print(F("Register readback: "));
  Serial.println(allReadbacksOK ? F("PASS") : F("FAIL"));
  Serial.print(F("Timing test:       "));
  Serial.println(timingTestOK ? F("PASS") : F("FAIL"));

  bool allPass = allReadbacksOK && timingTestOK;
  Serial.print(F("RESULT: "));
  Serial.println(allPass ? F("PASS") : F("FAIL"));
}

void loop() {
  // Nothing to do
}
