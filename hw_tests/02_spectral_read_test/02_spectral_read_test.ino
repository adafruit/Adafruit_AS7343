/*
 * AS7343 Spectral Read Test
 * 
 * Tests 18-channel spectral reading with NeoPixel illumination.
 * Hardware: Metro Mini, AS7343 on I2C, NeoPixel ring (16 LEDs) on Pin 6
 */

#include <Adafruit_AS7343.h>
#include <Adafruit_NeoPixel.h>

#define NEOPIXEL_PIN 6
#define NEOPIXEL_COUNT 16
#define DATA_READY_TIMEOUT_MS 2000

Adafruit_AS7343 as7343;
Adafruit_NeoPixel pixels(NEOPIXEL_COUNT, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

uint16_t baseline[18];
uint16_t readings[18];

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);
  
  Serial.println(F("AS7343 Spectral Read Test"));
  Serial.println(F("========================="));
  
  // Initialize NeoPixels
  pixels.begin();
  pixels.clear();
  pixels.show();
  
  // Initialize sensor
  if (!as7343.begin()) {
    Serial.println(F("Failed to find AS7343 sensor!"));
    Serial.println(F("RESULT: FAIL"));
    while (1) delay(100);
  }
  
  // Sensor defaults to 18-channel mode (AS7343_SMUX_18CH)
  
  // Step 1: NeoPixels OFF baseline reading
  Serial.println(F("NeoPixels OFF baseline..."));
  pixels.clear();
  pixels.show();
  delay(100);
  
  as7343.startMeasurement();
  if (!waitForDataReady()) {
    Serial.println(F("Baseline: Timeout waiting for data!"));
    Serial.println(F("RESULT: FAIL"));
    while (1) delay(100);
  }
  as7343.readAllChannels(baseline);
  
  // Step 2: NeoPixels ON white
  Serial.println(F("NeoPixels ON white..."));
  for (int i = 0; i < NEOPIXEL_COUNT; i++) {
    pixels.setPixelColor(i, pixels.Color(255, 255, 255));
  }
  pixels.show();
  delay(200); // Wait for settle
  
  // Step 3-5: Start measurement and wait for data
  Serial.println(F("Waiting for data ready..."));
  as7343.startMeasurement();
  
  if (!waitForDataReady()) {
    Serial.println(F("Timeout waiting for data!"));
    pixels.clear();
    pixels.show();
    Serial.println(F("RESULT: FAIL"));
    while (1) delay(100);
  }
  
  as7343.readAllChannels(readings);
  
  // Step 6: Print all 18 channel values
  Serial.println();
  Serial.println(F("Channel readings (18-channel mode):"));
  
  // Cycle 1: FZ, FY, FXL, NIR, VIS_TL_0, VIS_BR_0
  Serial.print(F("Cycle 1: FZ="));
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
  
  // Cycle 2: F2, F3, F4, F6, VIS_TL_1, VIS_BR_1
  Serial.print(F("Cycle 2: F2="));
  Serial.print(readings[6]);
  Serial.print(F(" F3="));
  Serial.print(readings[7]);
  Serial.print(F(" F4="));
  Serial.print(readings[8]);
  Serial.print(F(" F6="));
  Serial.print(readings[9]);
  Serial.print(F(" VIS_TL="));
  Serial.print(readings[10]);
  Serial.print(F(" VIS_BR="));
  Serial.println(readings[11]);
  
  // Cycle 3: F1, F7, F8, F5, VIS_TL_2, VIS_BR_2
  Serial.print(F("Cycle 3: F1="));
  Serial.print(readings[12]);
  Serial.print(F(" F7="));
  Serial.print(readings[13]);
  Serial.print(F(" F8="));
  Serial.print(readings[14]);
  Serial.print(F(" F5="));
  Serial.print(readings[15]);
  Serial.print(F(" VIS_TL="));
  Serial.print(readings[16]);
  Serial.print(F(" VIS_BR="));
  Serial.println(readings[17]);
  
  // Step 7: Verify readings (check if any values > 0 and changed from baseline)
  Serial.println();
  bool hasNonZero = false;
  bool hasChange = false;
  
  for (int i = 0; i < 18; i++) {
    if (readings[i] > 0) hasNonZero = true;
    if (readings[i] > baseline[i]) hasChange = true;
  }
  
  // Step 8: Turn NeoPixels OFF
  pixels.clear();
  pixels.show();
  
  // Step 9: Print result
  if (hasNonZero && hasChange) {
    Serial.println(F("RESULT: PASS"));
  } else {
    if (!hasNonZero) Serial.println(F("Error: All readings are zero!"));
    if (!hasChange) Serial.println(F("Error: No change from baseline!"));
    Serial.println(F("RESULT: FAIL"));
  }
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
