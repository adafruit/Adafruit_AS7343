/*
 * AS7343 SMUX Mode Test
 *
 * Tests the Auto-SMUX mode switching functionality.
 * AS7343 has 14 channels but only 6 ADCs, so SMUX cycles through channels:
 * - AS7343_SMUX_6CH (0): 6 channels per cycle
 * - AS7343_SMUX_12CH (2): 12 channels, 2 cycles
 * - AS7343_SMUX_18CH (3): 18 channels, 3 cycles (default)
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

bool allReadbackPass = true;

void setup() {
  Serial.begin(115200);
  while (!Serial)
    delay(10);

  Serial.println(F("AS7343 SMUX Mode Test"));
  Serial.println(F("====================="));

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

  // Turn NeoPixels ON at moderate brightness
  Serial.println(F("NeoPixels ON..."));
  Serial.println();
  for (int i = 0; i < NEOPIXEL_COUNT; i++) {
    pixels.setPixelColor(i, pixels.Color(64, 64, 64));
  }
  pixels.show();
  delay(100);

  // Print header
  Serial.println(F("Mode    Readback  Channels  Sample Values"));
  Serial.println(F("----    --------  --------  -------------"));

  // Test 6-channel mode
  test6CHMode();

  // Test 12-channel mode
  test12CHMode();

  // Test 18-channel mode
  test18CHMode();

  Serial.println();

  // Turn NeoPixels OFF
  pixels.clear();
  pixels.show();

  // Print summary
  Serial.print(F("Mode readback: "));
  Serial.println(allReadbackPass ? F("PASS") : F("FAIL"));

  Serial.print(F("RESULT: "));
  Serial.println(allReadbackPass ? F("PASS") : F("FAIL"));
}

void loop() {
  // Test complete
  delay(1000);
}

void test6CHMode() {
  // Set 6-channel mode
  if (!as7343.setSMUXMode(AS7343_SMUX_6CH)) {
    Serial.println(F("6CH     FAIL      -         setSMUXMode failed"));
    allReadbackPass = false;
    return;
  }

  // Verify readback
  as7343_smux_mode_t mode = as7343.getSMUXMode();
  bool readbackOK = (mode == AS7343_SMUX_6CH);
  if (!readbackOK)
    allReadbackPass = false;

  // Clear buffer
  memset(readings, 0, sizeof(readings));

  // Start measurement and wait for data
  as7343.startMeasurement();
  if (!waitForDataReady()) {
    Serial.print(F("6CH     "));
    Serial.print(readbackOK ? F("PASS") : F("FAIL"));
    Serial.println(F("      -         Timeout waiting for data"));
    return;
  }

  as7343.readAllChannels(readings);

  // Print results: FZ, FY, FXL, NIR, VIS_TL, VIS_BR
  Serial.print(F("6CH     "));
  Serial.print(readbackOK ? F("PASS") : F("FAIL"));
  Serial.print(F("      6         "));
  Serial.print(F("FZ="));
  Serial.print(readings[0]);
  Serial.print(F(" FY="));
  Serial.print(readings[1]);
  Serial.print(F(" FXL="));
  Serial.print(readings[2]);
  Serial.print(F(" NIR="));
  Serial.print(readings[3]);
  Serial.print(F(" VIS_TL="));
  Serial.print(readings[4]);
  Serial.print(F(" VIS_BR="));
  Serial.println(readings[5]);
}

void test12CHMode() {
  // Set 12-channel mode
  if (!as7343.setSMUXMode(AS7343_SMUX_12CH)) {
    Serial.println(F("12CH    FAIL      -         setSMUXMode failed"));
    allReadbackPass = false;
    return;
  }

  // Verify readback
  as7343_smux_mode_t mode = as7343.getSMUXMode();
  bool readbackOK = (mode == AS7343_SMUX_12CH);
  if (!readbackOK)
    allReadbackPass = false;

  // Clear buffer
  memset(readings, 0, sizeof(readings));

  // Start measurement and wait for data
  as7343.startMeasurement();
  if (!waitForDataReady()) {
    Serial.print(F("12CH    "));
    Serial.print(readbackOK ? F("PASS") : F("FAIL"));
    Serial.println(F("      -         Timeout waiting for data"));
    return;
  }

  as7343.readAllChannels(readings);

  // Print results: First 6 + Last 6 (F2, F3, F4, F6, VIS_TL_1, VIS_BR_1)
  Serial.print(F("12CH    "));
  Serial.print(readbackOK ? F("PASS") : F("FAIL"));
  Serial.print(F("      12        "));
  Serial.print(F("FZ="));
  Serial.print(readings[0]);
  Serial.print(F(" FY="));
  Serial.print(readings[1]);
  Serial.print(F(" ... F6="));
  Serial.print(readings[9]);
  Serial.print(F(" VIS_TL="));
  Serial.print(readings[10]);
  Serial.print(F(" VIS_BR="));
  Serial.println(readings[11]);
}

void test18CHMode() {
  // Set 18-channel mode
  if (!as7343.setSMUXMode(AS7343_SMUX_18CH)) {
    Serial.println(F("18CH    FAIL      -         setSMUXMode failed"));
    allReadbackPass = false;
    return;
  }

  // Verify readback
  as7343_smux_mode_t mode = as7343.getSMUXMode();
  bool readbackOK = (mode == AS7343_SMUX_18CH);
  if (!readbackOK)
    allReadbackPass = false;

  // Clear buffer
  memset(readings, 0, sizeof(readings));

  // Start measurement and wait for data
  as7343.startMeasurement();
  if (!waitForDataReady()) {
    Serial.print(F("18CH    "));
    Serial.print(readbackOK ? F("PASS") : F("FAIL"));
    Serial.println(F("      -         Timeout waiting for data"));
    return;
  }

  as7343.readAllChannels(readings);

  // Print results: First 2 + ... + Last few (F5, VIS_TL_2, VIS_BR_2)
  Serial.print(F("18CH    "));
  Serial.print(readbackOK ? F("PASS") : F("FAIL"));
  Serial.print(F("      18        "));
  Serial.print(F("FZ="));
  Serial.print(readings[0]);
  Serial.print(F(" FY="));
  Serial.print(readings[1]);
  Serial.print(F(" ... F5="));
  Serial.print(readings[15]);
  Serial.print(F(" VIS_TL="));
  Serial.print(readings[16]);
  Serial.print(F(" VIS_BR="));
  Serial.println(readings[17]);
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
