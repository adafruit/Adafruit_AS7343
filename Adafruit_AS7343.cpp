/*!
 *  @file Adafruit_AS7343.cpp
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
 *  Copyright 2026 Limor 'ladyada' Fried with assistance from Claude Code
 *
 *  BSD (see license.txt)
 */

#include "Adafruit_AS7343.h"

/**
 * @brief Construct a new Adafruit_AS7343 object
 */
Adafruit_AS7343::Adafruit_AS7343() {}

/**
 * @brief Destroy the Adafruit_AS7343 object
 */
Adafruit_AS7343::~Adafruit_AS7343() {
  if (i2c_dev) {
    delete i2c_dev;
  }
}

/**
 * @brief Sets up the hardware and initializes I2C
 * @param i2c_addr The I2C address to be used (default 0x39)
 * @param wire The Wire object to be used for I2C connections
 * @return true if initialization was successful, otherwise false
 */
bool Adafruit_AS7343::begin(uint8_t i2c_addr, TwoWire *wire) {
  if (i2c_dev) {
    delete i2c_dev;
  }

  i2c_dev = new Adafruit_I2CDevice(i2c_addr, wire);

  if (!i2c_dev->begin()) {
    return false;
  }

  return _init();
}

/**
 * @brief Initializer for post I2C init
 * @return true if chip identified and initialized
 */
bool Adafruit_AS7343::_init() {
  // Switch to bank 1 to read ID register
  if (!setBank(true)) {
    return false;
  }

  // Read part ID
  Adafruit_BusIO_Register id_reg = Adafruit_BusIO_Register(i2c_dev, AS7343_ID);
  uint8_t chip_id = id_reg.read();

  // Switch back to bank 0
  setBank(false);

  if (chip_id != AS7343_CHIP_ID) {
    return false;
  }

  // Power on
  if (!powerOn(true)) {
    return false;
  }

  // Set default gain (256x)
  if (!setGain(AS7343_GAIN_256X)) {
    return false;
  }

  // Set default integration time: ATIME=29, ASTEP=599 = ~50ms
  if (!setATIME(29)) {
    return false;
  }
  if (!setASTEP(599)) {
    return false;
  }

  // Set 18-channel mode by default
  if (!setSMUXMode(AS7343_SMUX_18CH)) {
    return false;
  }

  // Configure GPIO as output (signals measurement start by default)
  if (!setGPIOOutput(true)) {
    return false;
  }

  return true;
}

/**
 * @brief Switch register bank access
 * @param bank1 true for bank 1 (0x58-0x7F), false for bank 0 (0x80+)
 * @return true on success
 */
bool Adafruit_AS7343::setBank(bool bank1) {
  Adafruit_BusIO_Register cfg0_reg =
      Adafruit_BusIO_Register(i2c_dev, AS7343_CFG0);
  Adafruit_BusIO_RegisterBits reg_bank =
      Adafruit_BusIO_RegisterBits(&cfg0_reg, 1, 4);
  return reg_bank.write(bank1 ? 1 : 0);
}

/**
 * @brief Enable or disable power to the device
 * @param enable true to power on, false to power off
 * @return true on success
 */
bool Adafruit_AS7343::powerOn(bool enable) {
  setBank(false);
  Adafruit_BusIO_Register enable_reg =
      Adafruit_BusIO_Register(i2c_dev, AS7343_ENABLE);
  Adafruit_BusIO_RegisterBits pon_bit =
      Adafruit_BusIO_RegisterBits(&enable_reg, 1, 0);
  return pon_bit.write(enable ? 1 : 0);
}

/**
 * @brief Enable or disable low power idle mode
 * @param enable true to enable low power mode
 * @return true on success
 */
bool Adafruit_AS7343::enableLowPower(bool enable) {
  setBank(false);
  Adafruit_BusIO_Register cfg0_reg =
      Adafruit_BusIO_Register(i2c_dev, AS7343_CFG0);
  Adafruit_BusIO_RegisterBits low_power_bit =
      Adafruit_BusIO_RegisterBits(&cfg0_reg, 1, 5);
  return low_power_bit.write(enable ? 1 : 0);
}

/**
 * @brief Set the spectral measurement gain
 * @param gain The gain setting from as7343_gain_t
 * @return true on success
 */
bool Adafruit_AS7343::setGain(as7343_gain_t gain) {
  setBank(false);
  Adafruit_BusIO_Register cfg1_reg =
      Adafruit_BusIO_Register(i2c_dev, AS7343_CFG1);
  Adafruit_BusIO_RegisterBits again_bits =
      Adafruit_BusIO_RegisterBits(&cfg1_reg, 5, 0);
  return again_bits.write(gain);
}

/**
 * @brief Get the current gain setting
 * @return The current gain as as7343_gain_t
 */
as7343_gain_t Adafruit_AS7343::getGain() {
  setBank(false);
  Adafruit_BusIO_Register cfg1_reg =
      Adafruit_BusIO_Register(i2c_dev, AS7343_CFG1);
  Adafruit_BusIO_RegisterBits again_bits =
      Adafruit_BusIO_RegisterBits(&cfg1_reg, 5, 0);
  return (as7343_gain_t)again_bits.read();
}

/**
 * @brief Set ATIME (integration time multiplier)
 * @param atime Value 0-255
 * @return true on success
 */
bool Adafruit_AS7343::setATIME(uint8_t atime) {
  setBank(false);
  Adafruit_BusIO_Register atime_reg =
      Adafruit_BusIO_Register(i2c_dev, AS7343_ATIME);
  return atime_reg.write(atime);
}

/**
 * @brief Get the current ATIME value
 * @return ATIME value
 */
uint8_t Adafruit_AS7343::getATIME() {
  setBank(false);
  Adafruit_BusIO_Register atime_reg =
      Adafruit_BusIO_Register(i2c_dev, AS7343_ATIME);
  return atime_reg.read();
}

/**
 * @brief Set ASTEP (integration step size)
 * @param astep Value 0-65534 (65535 reserved)
 * @return true on success
 */
bool Adafruit_AS7343::setASTEP(uint16_t astep) {
  setBank(false);
  Adafruit_BusIO_Register astep_reg =
      Adafruit_BusIO_Register(i2c_dev, AS7343_ASTEP_L, 2, LSBFIRST);
  return astep_reg.write(astep);
}

/**
 * @brief Get the current ASTEP value
 * @return ASTEP value
 */
uint16_t Adafruit_AS7343::getASTEP() {
  setBank(false);
  Adafruit_BusIO_Register astep_reg =
      Adafruit_BusIO_Register(i2c_dev, AS7343_ASTEP_L, 2, LSBFIRST);
  return astep_reg.read();
}

/**
 * @brief Calculate integration time in milliseconds
 * @return Integration time in ms
 */
float Adafruit_AS7343::getIntegrationTime() {
  uint8_t atime = getATIME();
  uint16_t astep = getASTEP();
  // t_int = (ATIME + 1) × (ASTEP + 1) × 2.78 µs
  return (float)(atime + 1) * (float)(astep + 1) * 0.00278f;
}

/**
 * @brief Set the auto-SMUX channel cycling mode
 * @param mode The SMUX mode from as7343_smux_mode_t
 * @return true on success
 */
bool Adafruit_AS7343::setSMUXMode(as7343_smux_mode_t mode) {
  setBank(false);
  Adafruit_BusIO_Register cfg20_reg =
      Adafruit_BusIO_Register(i2c_dev, AS7343_CFG20);
  Adafruit_BusIO_RegisterBits auto_smux_bits =
      Adafruit_BusIO_RegisterBits(&cfg20_reg, 2, 5);
  return auto_smux_bits.write(mode);
}

/**
 * @brief Get the current auto-SMUX mode
 * @return The current SMUX mode as as7343_smux_mode_t
 */
as7343_smux_mode_t Adafruit_AS7343::getSMUXMode() {
  setBank(false);
  Adafruit_BusIO_Register cfg20_reg =
      Adafruit_BusIO_Register(i2c_dev, AS7343_CFG20);
  Adafruit_BusIO_RegisterBits auto_smux_bits =
      Adafruit_BusIO_RegisterBits(&cfg20_reg, 2, 5);
  return (as7343_smux_mode_t)auto_smux_bits.read();
}

/**
 * @brief Start spectral measurement
 * @return true on success
 */
bool Adafruit_AS7343::startMeasurement() {
  setBank(false);
  Adafruit_BusIO_Register enable_reg =
      Adafruit_BusIO_Register(i2c_dev, AS7343_ENABLE);
  Adafruit_BusIO_RegisterBits sp_en_bit =
      Adafruit_BusIO_RegisterBits(&enable_reg, 1, 1);
  return sp_en_bit.write(1);
}

/**
 * @brief Stop spectral measurement
 * @return true on success
 */
bool Adafruit_AS7343::stopMeasurement() {
  setBank(false);
  Adafruit_BusIO_Register enable_reg =
      Adafruit_BusIO_Register(i2c_dev, AS7343_ENABLE);
  Adafruit_BusIO_RegisterBits sp_en_bit =
      Adafruit_BusIO_RegisterBits(&enable_reg, 1, 1);
  return sp_en_bit.write(0);
}

/**
 * @brief Check if spectral measurement data is ready
 * @return true if data is ready to read
 */
bool Adafruit_AS7343::dataReady() {
  setBank(false);
  Adafruit_BusIO_Register status2_reg =
      Adafruit_BusIO_Register(i2c_dev, AS7343_STATUS2);
  Adafruit_BusIO_RegisterBits avalid_bit =
      Adafruit_BusIO_RegisterBits(&status2_reg, 1, 6);
  return avalid_bit.read() == 1;
}

/**
 * @brief Read all spectral channels into a buffer
 *
 * Buffer size depends on SMUX mode:
 * - 6CH mode: 6 values
 * - 12CH mode: 12 values
 * - 18CH mode: 18 values
 *
 * @param readings_buffer Pointer to buffer to store readings
 * @return true on success
 */
bool Adafruit_AS7343::readAllChannels(uint16_t *readings_buffer) {
  setBank(false);
  // Determine how many channels based on mode
  as7343_smux_mode_t mode = getSMUXMode();
  uint8_t num_channels = 6;
  if (mode == AS7343_SMUX_12CH) {
    num_channels = 12;
  } else if (mode == AS7343_SMUX_18CH) {
    num_channels = 18;
  }

  // Read ASTATUS first to latch all data
  Adafruit_BusIO_Register astatus_reg =
      Adafruit_BusIO_Register(i2c_dev, AS7343_ASTATUS);
  astatus_reg.read();

  // Read all data registers in one burst
  Adafruit_BusIO_Register data_reg =
      Adafruit_BusIO_Register(i2c_dev, AS7343_DATA_0_L, 2);
  return data_reg.read((uint8_t *)readings_buffer, num_channels * 2);
}

/**
 * @brief Read a single spectral channel
 * @param channel The channel to read
 * @return The channel reading
 */
uint16_t Adafruit_AS7343::readChannel(as7343_channel_t channel) {
  setBank(false);
  // Read ASTATUS to latch data
  Adafruit_BusIO_Register astatus_reg =
      Adafruit_BusIO_Register(i2c_dev, AS7343_ASTATUS);
  astatus_reg.read();

  // Read the specific channel
  Adafruit_BusIO_Register channel_reg = Adafruit_BusIO_Register(
      i2c_dev, AS7343_DATA_0_L + (channel * 2), 2, LSBFIRST);
  return channel_reg.read();
}

/**
 * @brief Enable or disable the LED driver
 * @param enable true to turn on LED
 * @return true on success
 */
bool Adafruit_AS7343::enableLED(bool enable) {
  setBank(false);
  Adafruit_BusIO_Register led_reg =
      Adafruit_BusIO_Register(i2c_dev, AS7343_LED);
  Adafruit_BusIO_RegisterBits led_act_bit =
      Adafruit_BusIO_RegisterBits(&led_reg, 1, 7);
  return led_act_bit.write(enable ? 1 : 0);
}

/**
 * @brief Set the LED drive current
 * @param current_ma Current in milliamps (4-258, even values only)
 * @return true on success
 */
bool Adafruit_AS7343::setLEDCurrent(uint16_t current_ma) {
  setBank(false);
  // Clamp to valid range
  if (current_ma < 4) {
    current_ma = 4;
  }
  if (current_ma > 258) {
    current_ma = 258;
  }

  // Calculate register value: current = 4 + (val × 2), so val = (current - 4) /
  // 2
  uint8_t val = (current_ma - 4) / 2;

  Adafruit_BusIO_Register led_reg =
      Adafruit_BusIO_Register(i2c_dev, AS7343_LED);
  Adafruit_BusIO_RegisterBits led_drive_bits =
      Adafruit_BusIO_RegisterBits(&led_reg, 7, 0);
  return led_drive_bits.write(val);
}

/**
 * @brief Get the current LED drive setting
 * @return LED current in milliamps
 */
uint16_t Adafruit_AS7343::getLEDCurrent() {
  setBank(false);
  Adafruit_BusIO_Register led_reg =
      Adafruit_BusIO_Register(i2c_dev, AS7343_LED);
  Adafruit_BusIO_RegisterBits led_drive_bits =
      Adafruit_BusIO_RegisterBits(&led_reg, 7, 0);
  uint8_t val = led_drive_bits.read();
  return 4 + (val * 2);
}

/**
 * @brief Enable or disable flicker detection
 * @param enable true to enable
 * @return true on success
 */
bool Adafruit_AS7343::enableFlickerDetection(bool enable) {
  setBank(false);
  Adafruit_BusIO_Register enable_reg =
      Adafruit_BusIO_Register(i2c_dev, AS7343_ENABLE);
  Adafruit_BusIO_RegisterBits fden_bit =
      Adafruit_BusIO_RegisterBits(&enable_reg, 1, 6);
  return fden_bit.write(enable ? 1 : 0);
}

/**
 * @brief Get the flicker detection status register
 * @return Raw FD_STATUS register value
 */
uint8_t Adafruit_AS7343::getFlickerStatus() {
  setBank(false);
  Adafruit_BusIO_Register fd_status_reg =
      Adafruit_BusIO_Register(i2c_dev, AS7343_FD_STATUS);
  return fd_status_reg.read();
}

/**
 * @brief Get the detected flicker frequency
 * @return Flicker frequency (0, 100, or 120 Hz)
 */
as7343_flicker_t Adafruit_AS7343::getFlickerFrequency() {
  uint8_t status = getFlickerStatus();

  // Check if 120Hz valid and detected
  if ((status & 0x08) && (status & 0x02)) {
    return AS7343_FLICKER_120HZ;
  }
  // Check if 100Hz valid and detected
  if ((status & 0x04) && (status & 0x01)) {
    return AS7343_FLICKER_100HZ;
  }

  return AS7343_FLICKER_NONE;
}

/**
 * @brief Enable or disable spectral threshold interrupt
 * @param enable true to enable
 * @return true on success
 */
bool Adafruit_AS7343::enableSpectralInterrupt(bool enable) {
  setBank(false);
  Adafruit_BusIO_Register intenab_reg =
      Adafruit_BusIO_Register(i2c_dev, AS7343_INTENAB);
  Adafruit_BusIO_RegisterBits sp_ien_bit =
      Adafruit_BusIO_RegisterBits(&intenab_reg, 1, 3);
  return sp_ien_bit.write(enable ? 1 : 0);
}

/**
 * @brief Enable or disable FIFO buffer interrupt
 * @param enable true to enable
 * @return true on success
 */
bool Adafruit_AS7343::enableFIFOInterrupt(bool enable) {
  setBank(false);
  Adafruit_BusIO_Register intenab_reg =
      Adafruit_BusIO_Register(i2c_dev, AS7343_INTENAB);
  Adafruit_BusIO_RegisterBits f_ien_bit =
      Adafruit_BusIO_RegisterBits(&intenab_reg, 1, 2);
  return f_ien_bit.write(enable ? 1 : 0);
}

/**
 * @brief Enable or disable system interrupt
 * @param enable true to enable
 * @return true on success
 */
bool Adafruit_AS7343::enableSystemInterrupt(bool enable) {
  setBank(false);
  Adafruit_BusIO_Register intenab_reg =
      Adafruit_BusIO_Register(i2c_dev, AS7343_INTENAB);
  Adafruit_BusIO_RegisterBits sien_bit =
      Adafruit_BusIO_RegisterBits(&intenab_reg, 1, 0);
  return sien_bit.write(enable ? 1 : 0);
}

/**
 * @brief Get the main status register
 * @return STATUS register value
 */
uint8_t Adafruit_AS7343::getStatus() {
  setBank(false);
  Adafruit_BusIO_Register status_reg =
      Adafruit_BusIO_Register(i2c_dev, AS7343_STATUS);
  return status_reg.read();
}

/**
 * @brief Clear all status flags
 * @return true on success
 */
bool Adafruit_AS7343::clearStatus() {
  setBank(false);
  Adafruit_BusIO_Register status_reg =
      Adafruit_BusIO_Register(i2c_dev, AS7343_STATUS);
  uint8_t status = status_reg.read();
  return status_reg.write(status); // Writing back clears flags
}

/**
 * @brief Set the low threshold for spectral interrupt
 * @param threshold 16-bit threshold value
 * @return true on success
 */
bool Adafruit_AS7343::setLowThreshold(uint16_t threshold) {
  setBank(false);
  Adafruit_BusIO_Register th_reg =
      Adafruit_BusIO_Register(i2c_dev, AS7343_SP_TH_L, 2, LSBFIRST);
  return th_reg.write(threshold);
}

/**
 * @brief Set the high threshold for spectral interrupt
 * @param threshold 16-bit threshold value
 * @return true on success
 */
bool Adafruit_AS7343::setHighThreshold(uint16_t threshold) {
  setBank(false);
  Adafruit_BusIO_Register th_reg =
      Adafruit_BusIO_Register(i2c_dev, AS7343_SP_TH_H, 2, LSBFIRST);
  return th_reg.write(threshold);
}

/**
 * @brief Get the low threshold value
 * @return 16-bit threshold value
 */
uint16_t Adafruit_AS7343::getLowThreshold() {
  setBank(false);
  Adafruit_BusIO_Register th_reg =
      Adafruit_BusIO_Register(i2c_dev, AS7343_SP_TH_L, 2, LSBFIRST);
  return th_reg.read();
}

/**
 * @brief Get the high threshold value
 * @return 16-bit threshold value
 */
uint16_t Adafruit_AS7343::getHighThreshold() {
  setBank(false);
  Adafruit_BusIO_Register th_reg =
      Adafruit_BusIO_Register(i2c_dev, AS7343_SP_TH_H, 2, LSBFIRST);
  return th_reg.read();
}

/**
 * @brief Enable or disable wait time between measurements
 * @param enable true to enable wait
 * @return true on success
 */
bool Adafruit_AS7343::enableWait(bool enable) {
  setBank(false);
  Adafruit_BusIO_Register enable_reg =
      Adafruit_BusIO_Register(i2c_dev, AS7343_ENABLE);
  Adafruit_BusIO_RegisterBits wen_bit =
      Adafruit_BusIO_RegisterBits(&enable_reg, 1, 3);
  return wen_bit.write(enable ? 1 : 0);
}

/**
 * @brief Set the wait time between measurements
 *
 * Wait time = (WTIME + 1) * 2.78ms
 * Range: 2.78ms (0) to 711ms (255)
 *
 * @param wtime Wait time value 0-255
 * @return true on success
 */
bool Adafruit_AS7343::setWaitTime(uint8_t wtime) {
  setBank(false);
  Adafruit_BusIO_Register wtime_reg =
      Adafruit_BusIO_Register(i2c_dev, AS7343_WTIME);
  return wtime_reg.write(wtime);
}

/**
 * @brief Get the current wait time value
 * @return Wait time value
 */
uint8_t Adafruit_AS7343::getWaitTime() {
  setBank(false);
  Adafruit_BusIO_Register wtime_reg =
      Adafruit_BusIO_Register(i2c_dev, AS7343_WTIME);
  return wtime_reg.read();
}

/**
 * @brief Set the interrupt persistence filter
 *
 * Number of consecutive measurements outside thresholds needed to trigger
 * interrupt. 0=every cycle, 1-3=1-3 cycles, 4=5 cycles, 5=10 cycles, etc.
 *
 * @param persistence Persistence value 0-15
 * @return true on success
 */
bool Adafruit_AS7343::setPersistence(uint8_t persistence) {
  setBank(false);
  if (persistence > 15)
    persistence = 15;
  Adafruit_BusIO_Register pers_reg =
      Adafruit_BusIO_Register(i2c_dev, AS7343_PERS);
  Adafruit_BusIO_RegisterBits apers_bits =
      Adafruit_BusIO_RegisterBits(&pers_reg, 4, 0);
  return apers_bits.write(persistence);
}

/**
 * @brief Get the current persistence value
 * @return Persistence value 0-15
 */
uint8_t Adafruit_AS7343::getPersistence() {
  setBank(false);
  Adafruit_BusIO_Register pers_reg =
      Adafruit_BusIO_Register(i2c_dev, AS7343_PERS);
  Adafruit_BusIO_RegisterBits apers_bits =
      Adafruit_BusIO_RegisterBits(&pers_reg, 4, 0);
  return apers_bits.read();
}

/**
 * @brief Set which ADC channel is used for threshold interrupts
 * @param channel ADC channel 0-5
 * @return true on success
 */
bool Adafruit_AS7343::setThresholdChannel(uint8_t channel) {
  if (channel > 5)
    channel = 5;

  // Need to switch to bank 1 for CFG12
  setBank(true);

  Adafruit_BusIO_Register cfg12_reg =
      Adafruit_BusIO_Register(i2c_dev, AS7343_CFG12);
  Adafruit_BusIO_RegisterBits sp_th_ch_bits =
      Adafruit_BusIO_RegisterBits(&cfg12_reg, 3, 0);
  bool result = sp_th_ch_bits.write(channel);

  setBank(false);
  return result;
}

/**
 * @brief Get the current threshold channel
 * @return ADC channel 0-5
 */
uint8_t Adafruit_AS7343::getThresholdChannel() {
  setBank(true);

  Adafruit_BusIO_Register cfg12_reg =
      Adafruit_BusIO_Register(i2c_dev, AS7343_CFG12);
  Adafruit_BusIO_RegisterBits sp_th_ch_bits =
      Adafruit_BusIO_RegisterBits(&cfg12_reg, 3, 0);
  uint8_t channel = sp_th_ch_bits.read();

  setBank(false);
  return channel;
}

/**
 * @brief Check if analog saturation occurred
 *
 * @note This flag may not behave as expected - it can report saturation
 * even when channel readings appear normal. May need further investigation.
 *
 * @return true if analog circuit saturated
 */
bool Adafruit_AS7343::isAnalogSaturated() {
  setBank(false);
  Adafruit_BusIO_Register status2_reg =
      Adafruit_BusIO_Register(i2c_dev, AS7343_STATUS2);
  Adafruit_BusIO_RegisterBits asat_ana_bit =
      Adafruit_BusIO_RegisterBits(&status2_reg, 1, 3);
  return asat_ana_bit.read() == 1;
}

/**
 * @brief Check if digital saturation occurred
 * @return true if ADC counter saturated
 */
bool Adafruit_AS7343::isDigitalSaturated() {
  setBank(false);
  Adafruit_BusIO_Register status2_reg =
      Adafruit_BusIO_Register(i2c_dev, AS7343_STATUS2);
  Adafruit_BusIO_RegisterBits asat_dig_bit =
      Adafruit_BusIO_RegisterBits(&status2_reg, 1, 4);
  return asat_dig_bit.read() == 1;
}

/**
 * @brief Set how often auto-zero is performed
 *
 * Auto-zero compensates for temperature drift. The value sets how many
 * measurement cycles between auto-zero operations.
 * - 0 = never (not recommended)
 * - 1 = every cycle
 * - 255 = only before first measurement (default)
 *
 * @param frequency Number of cycles between auto-zero (0-255)
 * @return true on success
 */
bool Adafruit_AS7343::setAutoZeroFrequency(uint8_t frequency) {
  setBank(false);
  Adafruit_BusIO_Register az_reg =
      Adafruit_BusIO_Register(i2c_dev, AS7343_AZ_CONFIG);
  return az_reg.write(frequency);
}

/**
 * @brief Get the current auto-zero frequency setting
 * @return Cycles between auto-zero operations
 */
uint8_t Adafruit_AS7343::getAutoZeroFrequency() {
  setBank(false);
  Adafruit_BusIO_Register az_reg =
      Adafruit_BusIO_Register(i2c_dev, AS7343_AZ_CONFIG);
  return az_reg.read();
}

/**
 * @brief Get the chip part ID
 * @return Part ID (should be 0x81 for AS7343)
 */
uint8_t Adafruit_AS7343::getPartID() {
  setBank(true);

  Adafruit_BusIO_Register id_reg = Adafruit_BusIO_Register(i2c_dev, AS7343_ID);
  uint8_t id = id_reg.read();

  setBank(false);
  return id;
}

/**
 * @brief Get the chip revision ID
 * @return Revision ID (bits 2:0)
 */
uint8_t Adafruit_AS7343::getRevisionID() {
  setBank(true);

  Adafruit_BusIO_Register rev_reg =
      Adafruit_BusIO_Register(i2c_dev, AS7343_REVID);
  Adafruit_BusIO_RegisterBits rev_bits =
      Adafruit_BusIO_RegisterBits(&rev_reg, 3, 0);
  uint8_t rev = rev_bits.read();

  setBank(false);
  return rev;
}

/**
 * @brief Get the chip auxiliary ID
 * @return Auxiliary ID (bits 3:0)
 */
uint8_t Adafruit_AS7343::getAuxID() {
  setBank(true);

  Adafruit_BusIO_Register aux_reg =
      Adafruit_BusIO_Register(i2c_dev, AS7343_AUXID);
  Adafruit_BusIO_RegisterBits aux_bits =
      Adafruit_BusIO_RegisterBits(&aux_reg, 4, 0);
  uint8_t aux = aux_bits.read();

  setBank(false);
  return aux;
}

/**
 * @brief Set GPIO to output or input mode
 * @param enable true for output mode, false for input mode
 * @return true on success
 */
bool Adafruit_AS7343::setGPIOOutput(bool enable) {
  if (!setBank(true)) {
    return false;
  }

  Adafruit_BusIO_Register gpio_reg =
      Adafruit_BusIO_Register(i2c_dev, AS7343_GPIO);
  uint8_t val = gpio_reg.read();

  if (enable) {
    val &= ~(1 << 2);
  } else {
    val |= (1 << 2);
  }

  bool result = gpio_reg.write(val);
  setBank(false);
  return result;
}

/**
 * @brief Set GPIO output state when in output mode
 * @param high true for high output, false for low
 * @return true on success
 */
bool Adafruit_AS7343::setGPIOValue(bool high) {
  if (!setBank(true)) {
    return false;
  }

  Adafruit_BusIO_Register gpio_reg =
      Adafruit_BusIO_Register(i2c_dev, AS7343_GPIO);
  uint8_t val = gpio_reg.read();

  if (high) {
    val |= (1 << 1);
  } else {
    val &= ~(1 << 1);
  }

  bool result = gpio_reg.write(val);
  setBank(false);
  return result;
}

/**
 * @brief Get GPIO input/output state
 * @return true if GPIO reads high
 */
bool Adafruit_AS7343::getGPIOValue() {
  if (!setBank(true)) {
    return false;
  }

  Adafruit_BusIO_Register gpio_reg =
      Adafruit_BusIO_Register(i2c_dev, AS7343_GPIO);
  uint8_t val = gpio_reg.read();

  setBank(false);
  return (val & (1 << 0)) != 0;
}

/**
 * @brief Invert GPIO polarity
 * @param invert true to invert GPIO polarity
 * @return true on success
 */
bool Adafruit_AS7343::setGPIOInverted(bool invert) {
  if (!setBank(true)) {
    return false;
  }

  Adafruit_BusIO_Register gpio_reg =
      Adafruit_BusIO_Register(i2c_dev, AS7343_GPIO);
  uint8_t val = gpio_reg.read();

  if (invert) {
    val |= (1 << 3);
  } else {
    val &= ~(1 << 3);
  }

  bool result = gpio_reg.write(val);
  setBank(false);
  return result;
}
