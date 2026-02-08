# Adafruit AS7343 Library [![Build Status](https://github.com/adafruit/Adafruit_AS7343/workflows/Arduino%20Library%20CI/badge.svg)](https://github.com/adafruit/Adafruit_AS7343/actions)

Arduino library for the AS7343 14-Channel Multi-Spectral Sensor.

## Description

The AS7343 is a 14-channel multi-spectral sensor covering wavelengths from 380nm to 1000nm. It features:

- **14 spectral channels** with peak wavelengths from violet (405nm) to near-IR (855nm)
- **6 independent 16-bit ADCs** with automatic channel multiplexing (auto-SMUX)
- **Programmable gain** from 0.5x to 2048x
- **Configurable integration time**
- **Built-in LED driver** (4-258mA)
- **100/120Hz flicker detection**
- **I2C interface** at address 0x39

## Spectral Channels

| Channel | Peak λ | Description |
|---------|--------|-------------|
| F1 | 405nm | Violet |
| F2 | 425nm | Violet-blue |
| FZ | 450nm | Blue |
| F3 | 475nm | Blue-cyan |
| F4 | 515nm | Green |
| F5 | 550nm | Green-yellow |
| FY | 555nm | Yellow-green (wide) |
| FXL | 600nm | Orange (wide) |
| F6 | 640nm | Red |
| F7 | 690nm | Deep red |
| F8 | 745nm | Near-IR |
| NIR | 855nm | Near-IR |
| VIS | - | Clear/broadband |
| FD | - | Flicker detection |

## Installation

This library is available through the Arduino Library Manager. Search for "Adafruit AS7343".

### Dependencies

- [Adafruit BusIO](https://github.com/adafruit/Adafruit_BusIO)

## Usage

```cpp
#include <Adafruit_AS7343.h>

Adafruit_AS7343 as7343;

void setup() {
  Serial.begin(115200);
  
  if (!as7343.begin()) {
    Serial.println("Could not find AS7343");
    while (1);
  }
  
  as7343.setGain(AS7343_GAIN_256X);
  as7343.setATIME(29);
  as7343.setASTEP(599);
}

void loop() {
  uint16_t readings[18];
  
  as7343.startMeasurement();
  while (!as7343.dataReady()) {
    delay(1);
  }
  as7343.readAllChannels(readings);
  
  Serial.print("F1 (405nm): "); Serial.println(readings[AS7343_CHANNEL_F1]);
  Serial.print("NIR (855nm): "); Serial.println(readings[AS7343_CHANNEL_NIR]);
  
  delay(500);
}
```

## License

BSD License - see license.txt

Adafruit invests time and resources providing this open source code, please support Adafruit and open-source hardware by purchasing products from Adafruit!
