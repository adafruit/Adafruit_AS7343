#include <Adafruit_AS7343.h>

Adafruit_AS7343 as7343;

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    delay(10);
  }

  Serial.println("AS7343 Flicker Detection Demo");

  if (!as7343.begin()) {
    Serial.println("AS7343 not found!");
    while (1) {
      delay(10);
    }
  }

  // Enable flicker detection
  as7343.enableFlickerDetection(true);
  Serial.println("Flicker detection enabled");
  Serial.println("Point sensor at light source...\n");
}

void loop() {
  as7343_flicker_t flicker = as7343.getFlickerFrequency();
  uint8_t raw = as7343.getFlickerStatus();

  Serial.print("Flicker: ");
  switch (flicker) {
    case AS7343_FLICKER_100HZ:
      Serial.print("100Hz");
      break;
    case AS7343_FLICKER_120HZ:
      Serial.print("120Hz");
      break;
    default:
      Serial.print("None");
  }
  Serial.print(" (raw status: 0x");
  Serial.print(raw, HEX);
  Serial.println(")");

  delay(500);
}
