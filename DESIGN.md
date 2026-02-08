# AS7343 Driver Design

## Chip Overview

- **Part:** AS7343 (ams OSRAM)
- **I2C Address:** 0x39 (7-bit)
- **Part ID Register:** 0x5A, expected value 0x81
- **Features:** 14-channel multi-spectral sensor (380-1000nm), 6 ADCs, flicker detection, LED driver, FIFO buffer

## Spectral Channels

| Channel | Peak λ (nm) | FWHM (nm) | Description |
|---------|-------------|-----------|-------------|
| F1 | 405 | 30 | Violet |
| F2 | 425 | 22 | Violet-blue |
| FZ | 450 | 55 | Blue |
| F3 | 475 | 30 | Blue-cyan |
| F4 | 515 | 40 | Green |
| FY | 555 | 100 | Yellow-green (wide) |
| F5 | 550 | 35 | Green-yellow |
| FXL | 600 | 80 | Orange (wide) |
| F6 | 640 | 50 | Red |
| F7 | 690 | 55 | Deep red |
| F8 | 745 | 60 | Near-IR |
| NIR | 855 | 54 | Near-IR |
| VIS | broadband | - | Clear/visible |
| FD | broadband | - | Flicker detection |

## Sensor Architecture

The AS7343 has a 5x5 photodiode array with 6 independent 16-bit ADCs. Since there are 14 channels but only 6 ADCs, the device uses a **SMUX** (Sensor Multiplexer) to route channels to ADCs.

**Auto-SMUX Modes (CFG20 register 0xD6):**
- `0`: 6 channels per cycle (FZ, FY, FXL, NIR, 2xVIS, FD)
- `2`: 12 channels, 2 cycles auto
- `3`: 18 channels, 3 cycles auto (all channels)

For full spectral data, use auto_smux = 3 (18-channel mode).

## Typed Enums

### Gain (AGAIN, 5-bit, register 0xC6)

| Value | Name | Gain |
|-------|------|------|
| 0 | GAIN_0_5X | 0.5x |
| 1 | GAIN_1X | 1x |
| 2 | GAIN_2X | 2x |
| 3 | GAIN_4X | 4x |
| 4 | GAIN_8X | 8x |
| 5 | GAIN_16X | 16x |
| 6 | GAIN_32X | 32x |
| 7 | GAIN_64X | 64x |
| 8 | GAIN_128X | 128x |
| 9 | GAIN_256X | 256x (default) |
| 10 | GAIN_512X | 512x |
| 11 | GAIN_1024X | 1024x |
| 12 | GAIN_2048X | 2048x |

## Integration Time

**Formula:** `t_int = (ATIME + 1) × (ASTEP + 1) × 2.78 µs`

**Defaults:** ATIME=29, ASTEP=599 → ~50 ms

## Data Register Mapping (18-channel mode)

When auto_smux = 3, three measurement cycles occur:

| Cycle | DATA_0 | DATA_1 | DATA_2 | DATA_3 | DATA_4 | DATA_5 |
|-------|--------|--------|--------|--------|--------|--------|
| 1 | FZ | FY | FXL | NIR | VIS_TL | VIS_BR |
| 2 | F2 | F3 | F4 | F6 | VIS_TL | VIS_BR |
| 3 | F1 | F7 | F8 | F5 | VIS_TL | VIS_BR |

All channels are read across 3 cycles and stored in DATA_0 through DATA_17.

## Register Bank Switching

The AS7343 has two register banks:
- **Bank 0** (REG_BANK=0): Registers 0x80 and above
- **Bank 1** (REG_BANK=1): Registers 0x58 to 0x7F

Switch via CFG0 (0xBF) bit 4.
