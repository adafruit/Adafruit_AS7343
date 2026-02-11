/*!
 * @file basic_readings.ino
 *
 * Basic example for the Adafruit AS7343 14-Channel Multi-Spectral Sensor
 *
 * Reads all 18 channels (14 spectral + VIS + FD in 3 cycles) and prints
 * the values to Serial.
 */

#include <Adafruit_AS7343.h>

Adafruit_AS7343 as7343;

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    delay(10); // Wait for Serial on native USB boards
  }

  Serial.println("Adafruit AS7343 Basic Readings");

  if (!as7343.begin()) {
    Serial.println("Could not find AS7343 sensor!");
    while (1) {
      delay(10);
    }
  }

  Serial.println("AS7343 found!");

  // Configure sensor
  as7343.setGain(AS7343_GAIN_64X);
  as7343.setATIME(29);  // Integration cycles
  as7343.setASTEP(599); // Step size

  Serial.print("Integration time: ");
  Serial.print(as7343.getIntegrationTime());
  Serial.println(" ms");
}

void loop() {
  uint16_t readings[18];

  // Read all channels (starts measurement, waits, reads internally)
  if (!as7343.readAllChannels(readings)) {
    Serial.println("Read failed!");
    delay(500);
    return;
  }

  // Print spectral channels (wavelength order)
  Serial.println("\n--- Spectral Readings ---");

  Serial.print("F1  (405nm violet):     ");
  Serial.println(readings[AS7343_CHANNEL_F1]);

  Serial.print("F2  (425nm violet-blue):");
  Serial.println(readings[AS7343_CHANNEL_F2]);

  Serial.print("FZ  (450nm blue):       ");
  Serial.println(readings[AS7343_CHANNEL_FZ]);

  Serial.print("F3  (475nm blue-cyan):  ");
  Serial.println(readings[AS7343_CHANNEL_F3]);

  Serial.print("F4  (515nm green):      ");
  Serial.println(readings[AS7343_CHANNEL_F4]);

  Serial.print("F5  (550nm green-yel):  ");
  Serial.println(readings[AS7343_CHANNEL_F5]);

  Serial.print("FY  (555nm yellow-grn): ");
  Serial.println(readings[AS7343_CHANNEL_FY]);

  Serial.print("FXL (600nm orange):     ");
  Serial.println(readings[AS7343_CHANNEL_FXL]);

  Serial.print("F6  (640nm red):        ");
  Serial.println(readings[AS7343_CHANNEL_F6]);

  Serial.print("F7  (690nm deep red):   ");
  Serial.println(readings[AS7343_CHANNEL_F7]);

  Serial.print("F8  (745nm near-IR):    ");
  Serial.println(readings[AS7343_CHANNEL_F8]);

  Serial.print("NIR (855nm near-IR):    ");
  Serial.println(readings[AS7343_CHANNEL_NIR]);

  // Print clear/VIS channels (one from each cycle)
  Serial.print("VIS (clear):            ");
  Serial.println(readings[AS7343_CHANNEL_VIS_TL_0]);

  delay(500);
}
