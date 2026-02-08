/*!
 *  @file Adafruit_AS7343.h
 *
 *  @mainpage Adafruit AS7343 14-Channel Multi-Spectral Sensor
 *
 *  @section intro_sec Introduction
 *
 *  I2C Driver for the AS7343 14-Channel Multi-Spectral Sensor
 *
 *  This is a library for the Adafruit AS7343 breakout:
 *  https://www.adafruit.com/product/XXXX
 *
 *  Adafruit invests time and resources providing this open source code,
 *  please support Adafruit and open-source hardware by purchasing products from
 *  Adafruit!
 *
 *  @section dependencies Dependencies
 *  This library depends on the Adafruit BusIO library
 *
 *  @section author Author
 *
 *  Limor 'ladyada' Fried with assistance from Claude Code
 *
 *  @section license License
 *
 *  BSD (see license.txt)
 */

#ifndef _ADAFRUIT_AS7343_H
#define _ADAFRUIT_AS7343_H

#include "Arduino.h"
#include <Adafruit_BusIO_Register.h>
#include <Adafruit_I2CDevice.h>
#include <Wire.h>

#define AS7343_I2CADDR_DEFAULT 0x39 ///< AS7343 default i2c address
#define AS7343_CHIP_ID 0x81         ///< AS7343 part ID

// Register addresses (REG_BANK = 1 for 0x58-0x66)
#define AS7343_AUXID 0x58 ///< Auxiliary ID
#define AS7343_REVID 0x59 ///< Revision ID
#define AS7343_ID 0x5A    ///< Part ID register
#define AS7343_CFG10 0x65 ///< FD_PERS configuration
#define AS7343_CFG12 0x66 ///< SP_TH_CH configuration
#define AS7343_GPIO 0x6B  ///< GPIO control register

// Register addresses (REG_BANK = 0 for 0x80+)
#define AS7343_ENABLE 0x80       ///< Main enable register
#define AS7343_ATIME 0x81        ///< Integration time multiplier
#define AS7343_WTIME 0x83        ///< Wait time
#define AS7343_SP_TH_L 0x84      ///< Spectral low threshold (16-bit)
#define AS7343_SP_TH_H 0x86      ///< Spectral high threshold (16-bit)
#define AS7343_STATUS2 0x90      ///< Status 2 (AVALID, saturation)
#define AS7343_STATUS3 0x91      ///< Status 3 (interrupt source)
#define AS7343_STATUS 0x93       ///< Main status register
#define AS7343_ASTATUS 0x94      ///< ADC status
#define AS7343_DATA_0_L 0x95     ///< First data register (low byte)
#define AS7343_STATUS5 0xBB      ///< Status 5 (SINT_FD, SINT_SMUX)
#define AS7343_STATUS4 0xBC      ///< Status 4 (FIFO_OV, triggers)
#define AS7343_CFG0 0xBF         ///< Config 0 (REG_BANK, LOW_POWER)
#define AS7343_CFG1 0xC6         ///< Config 1 (AGAIN)
#define AS7343_CFG3 0xC7         ///< Config 3 (SAI)
#define AS7343_CFG6 0xF5         ///< Config 6 (SMUX_CMD)
#define AS7343_CFG8 0xC9         ///< Config 8 (FIFO_TH)
#define AS7343_CFG9 0xCA         ///< Config 9 (SIEN_FD, SIEN_SMUX)
#define AS7343_LED 0xCD          ///< LED control register
#define AS7343_PERS 0xCF         ///< Persistence register
#define AS7343_ASTEP_L 0xD4      ///< Integration step size low byte
#define AS7343_ASTEP_H 0xD5      ///< Integration step size high byte
#define AS7343_CFG20 0xD6        ///< Config 20 (auto_smux, FD_FIFO_8b)
#define AS7343_AGC_GAIN_MAX 0xD7 ///< AGC gain max
#define AS7343_AZ_CONFIG 0xDE    ///< Auto-zero config
#define AS7343_FD_CFG0 0xDF      ///< Flicker detection config
#define AS7343_FD_TIME_1 0xE0    ///< Flicker detection time LSB
#define AS7343_FD_TIME_2 0xE2    ///< Flicker detection time MSB + gain
#define AS7343_FD_STATUS 0xE3    ///< Flicker detection status
#define AS7343_INTENAB 0xF9      ///< Interrupt enable
#define AS7343_CONTROL 0xFA      ///< Control register
#define AS7343_FIFO_MAP 0xFC     ///< FIFO channel mapping
#define AS7343_FIFO_LVL 0xFD     ///< FIFO level
#define AS7343_FDATA_L 0xFE      ///< FIFO data low byte
#define AS7343_FDATA_H 0xFF      ///< FIFO data high byte

/**
 * @brief Gain settings for spectral measurements
 */
typedef enum {
  AS7343_GAIN_0_5X = 0,   ///< 0.5x gain
  AS7343_GAIN_1X = 1,     ///< 1x gain
  AS7343_GAIN_2X = 2,     ///< 2x gain
  AS7343_GAIN_4X = 3,     ///< 4x gain
  AS7343_GAIN_8X = 4,     ///< 8x gain
  AS7343_GAIN_16X = 5,    ///< 16x gain
  AS7343_GAIN_32X = 6,    ///< 32x gain
  AS7343_GAIN_64X = 7,    ///< 64x gain
  AS7343_GAIN_128X = 8,   ///< 128x gain
  AS7343_GAIN_256X = 9,   ///< 256x gain (default)
  AS7343_GAIN_512X = 10,  ///< 512x gain
  AS7343_GAIN_1024X = 11, ///< 1024x gain
  AS7343_GAIN_2048X = 12, ///< 2048x gain
} as7343_gain_t;

/**
 * @brief Auto-SMUX channel cycling modes
 */
typedef enum {
  AS7343_SMUX_6CH = 0,  ///< 6 channels (FZ, FY, FXL, NIR, VIS, FD)
  AS7343_SMUX_12CH = 2, ///< 12 channels, 2 auto cycles
  AS7343_SMUX_18CH = 3, ///< 18 channels, 3 auto cycles (all channels)
} as7343_smux_mode_t;

/**
 * @brief Spectral channel identifiers for 18-channel mode
 *
 * Index matches the order data appears in 18-channel auto_smux mode
 */
typedef enum {
  AS7343_CHANNEL_FZ = 0,        ///< 450nm (blue)
  AS7343_CHANNEL_FY = 1,        ///< 555nm (yellow-green)
  AS7343_CHANNEL_FXL = 2,       ///< 600nm (orange)
  AS7343_CHANNEL_NIR = 3,       ///< 855nm (near-IR)
  AS7343_CHANNEL_VIS_TL_0 = 4,  ///< Clear (top-left) cycle 1
  AS7343_CHANNEL_VIS_BR_0 = 5,  ///< Clear (both-right) cycle 1
  AS7343_CHANNEL_F2 = 6,        ///< 425nm (violet-blue)
  AS7343_CHANNEL_F3 = 7,        ///< 475nm (blue-cyan)
  AS7343_CHANNEL_F4 = 8,        ///< 515nm (green)
  AS7343_CHANNEL_F6 = 9,        ///< 640nm (red)
  AS7343_CHANNEL_VIS_TL_1 = 10, ///< Clear (top-left) cycle 2
  AS7343_CHANNEL_VIS_BR_1 = 11, ///< Clear (both-right) cycle 2
  AS7343_CHANNEL_F1 = 12,       ///< 405nm (violet)
  AS7343_CHANNEL_F7 = 13,       ///< 690nm (deep red)
  AS7343_CHANNEL_F8 = 14,       ///< 745nm (near-IR)
  AS7343_CHANNEL_F5 = 15,       ///< 550nm (green-yellow)
  AS7343_CHANNEL_VIS_TL_2 = 16, ///< Clear (top-left) cycle 3
  AS7343_CHANNEL_VIS_BR_2 = 17, ///< Clear (both-right) cycle 3
} as7343_channel_t;

/**
 * @brief Flicker detection frequency results
 */
typedef enum {
  AS7343_FLICKER_NONE = 0,    ///< No flicker detected
  AS7343_FLICKER_100HZ = 100, ///< 100Hz flicker detected
  AS7343_FLICKER_120HZ = 120, ///< 120Hz flicker detected
} as7343_flicker_t;

/*!
 *  @brief  Class that stores state and functions for interacting with
 *          the AS7343 14-Channel Multi-Spectral Sensor
 */
class Adafruit_AS7343 {
public:
  Adafruit_AS7343();
  ~Adafruit_AS7343();

  bool begin(uint8_t i2c_addr = AS7343_I2CADDR_DEFAULT, TwoWire *wire = &Wire);

  // Gain and integration time
  bool setGain(as7343_gain_t gain);
  as7343_gain_t getGain();
  bool setATIME(uint8_t atime);
  uint8_t getATIME();
  bool setASTEP(uint16_t astep);
  uint16_t getASTEP();
  float getIntegrationTime(); // Returns integration time in milliseconds

  // Auto-SMUX mode
  bool setSMUXMode(as7343_smux_mode_t mode);
  as7343_smux_mode_t getSMUXMode();

  // Measurement control
  bool startMeasurement();
  bool stopMeasurement();
  bool dataReady();
  bool readAllChannels(uint16_t *readings_buffer);
  uint16_t readChannel(as7343_channel_t channel);

  // LED driver
  bool enableLED(bool enable);
  bool setLEDCurrent(uint16_t current_ma);
  uint16_t getLEDCurrent();

  // Flicker detection
  bool enableFlickerDetection(bool enable);
  as7343_flicker_t getFlickerFrequency();
  uint8_t getFlickerStatus();

  // Power control
  bool powerOn(bool enable);
  bool enableLowPower(bool enable);

  // Wait time
  bool enableWait(bool enable);
  bool setWaitTime(uint8_t wtime);
  uint8_t getWaitTime();

  // Interrupts
  bool enableSpectralInterrupt(bool enable);
  bool enableFIFOInterrupt(bool enable);
  bool enableSystemInterrupt(bool enable);
  uint8_t getStatus();
  bool clearStatus();
  bool setLowThreshold(uint16_t threshold);
  bool setHighThreshold(uint16_t threshold);
  uint16_t getLowThreshold();
  uint16_t getHighThreshold();
  bool setPersistence(uint8_t persistence);
  uint8_t getPersistence();
  bool setThresholdChannel(uint8_t channel);
  uint8_t getThresholdChannel();

  // Saturation detection
  bool isAnalogSaturated();
  bool isDigitalSaturated();

  // Auto-zero configuration
  bool setAutoZeroFrequency(uint8_t frequency);
  uint8_t getAutoZeroFrequency();

  // Chip info
  uint8_t getPartID();
  uint8_t getRevisionID();
  uint8_t getAuxID();

  // GPIO control
  bool setGPIOOutput(bool enable);   // true = output mode, false = input mode
  bool setGPIOValue(bool high);      // Set output state when in output mode
  bool getGPIOValue();               // Read GPIO input/output state
  bool setGPIOInverted(bool invert); // Invert GPIO polarity

protected:
  virtual bool _init();
  bool setBank(bool bank1); // true = access 0x58-0x7F, false = access 0x80+

  Adafruit_I2CDevice *i2c_dev = NULL; ///< Pointer to I2C bus interface

private:
  uint16_t _channel_readings[18]; ///< Buffer for 18-channel readings
};

#endif
