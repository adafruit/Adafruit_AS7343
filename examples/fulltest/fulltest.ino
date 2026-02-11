/*
 * Full test sketch for AS7343 14-Channel Multi-Spectral Sensor
 *
 * Written by Limor 'ladyada' Fried with assistance from Claude Code for
 * Adafruit Industries. MIT license, check license.txt for more information
 *
 * Displays configuration and continuous spectral readings.
 */

#include <Adafruit_AS7343.h>

Adafruit_AS7343 as7343;

void setup() {
  Serial.begin(115200);
  while (!Serial)
    delay(10);

  Serial.println(F("AS7343 Full Test"));
  Serial.println(F("================"));

  if (!as7343.begin()) {
    Serial.println(F("Couldn't find AS7343 chip"));
    while (1)
      delay(10);
  }

  Serial.println(F("AS7343 found!"));

  // === Chip Information ===
  Serial.println(F("\n--- Chip Information ---"));
  Serial.print(F("Part ID: 0x"));
  Serial.println(as7343.getPartID(), HEX);
  Serial.print(F("Revision ID: 0x"));
  Serial.println(as7343.getRevisionID(), HEX);
  Serial.print(F("Aux ID: 0x"));
  Serial.println(as7343.getAuxID(), HEX);

  // === Spectral Engine Configuration ===
  Serial.println(F("\n--- Spectral Configuration ---"));

  as7343.setGain(AS7343_GAIN_64X);
  Serial.print(F("Gain: "));
  switch (as7343.getGain()) {
  case AS7343_GAIN_0_5X:
    Serial.println(F("0.5x"));
    break;
  case AS7343_GAIN_1X:
    Serial.println(F("1x"));
    break;
  case AS7343_GAIN_2X:
    Serial.println(F("2x"));
    break;
  case AS7343_GAIN_4X:
    Serial.println(F("4x"));
    break;
  case AS7343_GAIN_8X:
    Serial.println(F("8x"));
    break;
  case AS7343_GAIN_16X:
    Serial.println(F("16x"));
    break;
  case AS7343_GAIN_32X:
    Serial.println(F("32x"));
    break;
  case AS7343_GAIN_64X:
    Serial.println(F("64x"));
    break;
  case AS7343_GAIN_128X:
    Serial.println(F("128x"));
    break;
  case AS7343_GAIN_256X:
    Serial.println(F("256x"));
    break;
  case AS7343_GAIN_512X:
    Serial.println(F("512x"));
    break;
  case AS7343_GAIN_1024X:
    Serial.println(F("1024x"));
    break;
  case AS7343_GAIN_2048X:
    Serial.println(F("2048x"));
    break;
  default:
    Serial.println(F("Unknown"));
    break;
  }

  as7343.setATIME(29);
  Serial.print(F("ATIME: "));
  Serial.println(as7343.getATIME());

  as7343.setASTEP(599);
  Serial.print(F("ASTEP: "));
  Serial.println(as7343.getASTEP());

  Serial.print(F("Integration Time: "));
  Serial.print(as7343.getIntegrationTime());
  Serial.println(F(" ms"));

  // === SMUX Configuration ===
  Serial.println(F("\n--- SMUX Configuration ---"));

  as7343.setSMUXMode(AS7343_SMUX_18CH);
  Serial.print(F("Mode: "));
  switch (as7343.getSMUXMode()) {
  case AS7343_SMUX_6CH:
    Serial.println(F("6 channels"));
    break;
  case AS7343_SMUX_12CH:
    Serial.println(F("12 channels (2 cycles)"));
    break;
  case AS7343_SMUX_18CH:
    Serial.println(F("18 channels (3 cycles)"));
    break;
  default:
    Serial.println(F("Unknown"));
    break;
  }

  // === Wait Time Configuration ===
  Serial.println(F("\n--- Wait Time Configuration ---"));

  as7343.setWaitTime(100);
  Serial.print(F("Wait Time: "));
  Serial.print(as7343.getWaitTime());
  Serial.println(F(" (disabled by default)"));

  // === Interrupt Configuration ===
  Serial.println(F("\n--- Interrupt Configuration ---"));

  as7343.setPersistence(4);
  Serial.print(F("Persistence: "));
  Serial.println(as7343.getPersistence());

  // NOTE: Threshold channel register r/w works but has no observed effect
  // on threshold comparison - comparison is always on CH0 regardless.
  as7343.setThresholdChannel(0);
  Serial.print(F("Threshold Channel: "));
  Serial.println(as7343.getThresholdChannel());

  as7343.setLowThreshold(100);
  Serial.print(F("Low Threshold: "));
  Serial.println(as7343.getLowThreshold());

  as7343.setHighThreshold(60000);
  Serial.print(F("High Threshold: "));
  Serial.println(as7343.getHighThreshold());

  // === LED Driver Configuration ===
  Serial.println(F("\n--- LED Driver Configuration ---"));

  as7343.setLEDCurrent(20);
  Serial.print(F("LED Current: "));
  Serial.print(as7343.getLEDCurrent());
  Serial.println(F(" mA"));

  Serial.print(F("LED: "));
  Serial.println(F("Off (use enableLED(true) to turn on)"));

  // === Start continuous reading ===
  Serial.println(F("\n--- Spectral Readings ---"));
  Serial.println(F("Channel wavelengths: F1=405nm, F2=425nm, FZ=450nm, "
                   "F3=475nm, F4=515nm, F5=550nm,"));
  Serial.println(
      F("FY=555nm, FXL=600nm, F6=640nm, F7=690nm, F8=745nm, NIR=855nm\n"));

  delay(200); // Let sensor stabilize
}

void loop() {
  uint16_t readings[18];

  if (!as7343.readAllChannels(readings)) {
    Serial.println(F("Read failed!"));
    delay(500);
    return;
  }

  // Print in wavelength order for easier reading
  Serial.print(F("F1:"));
  Serial.print(readings[AS7343_CHANNEL_F1]);
  Serial.print(F("\tF2:"));
  Serial.print(readings[AS7343_CHANNEL_F2]);
  Serial.print(F("\tFZ:"));
  Serial.print(readings[AS7343_CHANNEL_FZ]);
  Serial.print(F("\tF3:"));
  Serial.print(readings[AS7343_CHANNEL_F3]);
  Serial.print(F("\tF4:"));
  Serial.print(readings[AS7343_CHANNEL_F4]);
  Serial.print(F("\tF5:"));
  Serial.print(readings[AS7343_CHANNEL_F5]);

  Serial.print(F("\tFY:"));
  Serial.print(readings[AS7343_CHANNEL_FY]);
  Serial.print(F("\tFXL:"));
  Serial.print(readings[AS7343_CHANNEL_FXL]);
  Serial.print(F("\tF6:"));
  Serial.print(readings[AS7343_CHANNEL_F6]);
  Serial.print(F("\tF7:"));
  Serial.print(readings[AS7343_CHANNEL_F7]);
  Serial.print(F("\tF8:"));
  Serial.print(readings[AS7343_CHANNEL_F8]);
  Serial.print(F("\tNIR:"));
  Serial.println(readings[AS7343_CHANNEL_NIR]);

  delay(100);
}
