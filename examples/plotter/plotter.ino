/*
 * Serial Plotter example for AS7343 14-Channel Multi-Spectral Sensor
 *
 * Written by Limor 'ladyada' Fried with assistance from Claude Code for
 * Adafruit Industries. MIT license, check license.txt for more information
 *
 * Outputs spectral data in Arduino Serial Plotter format.
 * Channels are ordered by wavelength: violet -> blue -> green -> yellow ->
 * orange -> red -> NIR
 *
 * Open Tools > Serial Plotter at 115200 baud to view the spectrum.
 */

#include <Adafruit_AS7343.h>

Adafruit_AS7343 as7343;

void setup() {
  Serial.begin(115200);
  while (!Serial)
    delay(10);

  if (!as7343.begin()) {
    Serial.println("AS7343 not found!");
    while (1)
      delay(10);
  }

  // Configure for good plotter response
  as7343.setGain(AS7343_GAIN_256X);
  as7343.setATIME(29);
  as7343.setASTEP(599);
  as7343.setSMUXMode(AS7343_SMUX_18CH);
}

void loop() {
  uint16_t readings[18];

  as7343.startMeasurement();
  while (!as7343.dataReady()) {
    delay(1);
  }
  as7343.readAllChannels(readings);

  // Output in wavelength order for spectrum visualization
  // Format: label:value with tabs between for Serial Plotter

  // Violet/UV (405-425nm)
  Serial.print("405nm_F1:");
  Serial.print(readings[AS7343_CHANNEL_F1]);
  Serial.print("\t");

  Serial.print("425nm_F2:");
  Serial.print(readings[AS7343_CHANNEL_F2]);
  Serial.print("\t");

  // Blue (450-475nm)
  Serial.print("450nm_FZ:");
  Serial.print(readings[AS7343_CHANNEL_FZ]);
  Serial.print("\t");

  Serial.print("475nm_F3:");
  Serial.print(readings[AS7343_CHANNEL_F3]);
  Serial.print("\t");

  // Green (515-555nm)
  Serial.print("515nm_F4:");
  Serial.print(readings[AS7343_CHANNEL_F4]);
  Serial.print("\t");

  Serial.print("550nm_F5:");
  Serial.print(readings[AS7343_CHANNEL_F5]);
  Serial.print("\t");

  Serial.print("555nm_FY:");
  Serial.print(readings[AS7343_CHANNEL_FY]);
  Serial.print("\t");

  // Yellow/Orange (600nm)
  Serial.print("600nm_FXL:");
  Serial.print(readings[AS7343_CHANNEL_FXL]);
  Serial.print("\t");

  // Red (640-690nm)
  Serial.print("640nm_F6:");
  Serial.print(readings[AS7343_CHANNEL_F6]);
  Serial.print("\t");

  Serial.print("690nm_F7:");
  Serial.print(readings[AS7343_CHANNEL_F7]);
  Serial.print("\t");

  // Near-IR (745-855nm)
  Serial.print("745nm_F8:");
  Serial.print(readings[AS7343_CHANNEL_F8]);
  Serial.print("\t");

  Serial.print("855nm_NIR:");
  Serial.print(readings[AS7343_CHANNEL_NIR]);
  Serial.print("\t");

  // Clear/broadband
  Serial.print("Clear:");
  Serial.println(readings[AS7343_CHANNEL_VIS_TL_0]);

  delay(50);
}
