/**
 * @brief source file for LcdDriver class, contain most user facing methods as well as major
 * backend communication function
 *
 * @file lcd_driver.cpp
 * @author Khoi Trinh
 * @date 2018-09-23
 */

#include "lcd_driver.hpp"

#include <cassert>
#include <cctype>
#include <cstring>

// peripheral
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"

// hardware
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"

// application
#include "general_timer/general_timer.hpp"
#include "lcd_include.hpp"
#include "tiva_utils/bit_manipulation.h"

namespace lcddriver {

/**
 * @brief Helper function that is used to enable the clock for a port then wait until it's ready
 *
 * @param clockMask mask of the peripheral clock, for example: SYSCTL_PERIPH_GPIOB for general gpio
 * of portB
 */
static void enableClockPeripheral(const uint32_t& clockMask) {
  SysCtlPeripheralEnable(clockMask);
  while (!SysCtlPeripheralReady(clockMask)) {
    // wait for clock to be ready
  }
}

LcdDriver::LcdDriver(const LcdConfig& lcdconfig)
    : _lcdConfig(lcdconfig),
      _generalTimer(GeneralTimer(UNIT_NANOSEC)),
      _totalBitPerPin(8 / TOTAL_PARALLEL_PIN) {}

void LcdDriver::init(void) {
  // check, initialize clock, and configure pins
  pinDescCheck(_lcdConfig.regSelectPin);
  enableClockPeripheral(_lcdConfig.regSelectPin[PIN_DESC_CLOCK_INDEX]);
  pinModeSwitch(_lcdConfig.regSelectPin, false);
  pinPadConfig(_lcdConfig.regSelectPin);

  pinDescCheck(_lcdConfig.readWritePin);
  enableClockPeripheral(_lcdConfig.readWritePin[PIN_DESC_CLOCK_INDEX]);
  pinModeSwitch(_lcdConfig.readWritePin, false);
  pinPadConfig(_lcdConfig.readWritePin);

  pinDescCheck(_lcdConfig.enablePin);
  enableClockPeripheral(_lcdConfig.enablePin[PIN_DESC_CLOCK_INDEX]);
  pinModeSwitch(_lcdConfig.enablePin, false);
  pinPadConfig(_lcdConfig.enablePin);

  if (_lcdConfig.useBacklight) {
    pinDescCheck(_lcdConfig.backLightPin);
    enableClockPeripheral(_lcdConfig.backLightPin[PIN_DESC_CLOCK_INDEX]);
    pinModeSwitch(_lcdConfig.backLightPin, false);
    pinPadConfig(_lcdConfig.backLightPin);
  }

  for (uint32_t pin = 0; pin < TOTAL_PARALLEL_PIN; ++pin) {
    pinDescCheck(_lcdConfig.parallelPinList[pin]);
    enableClockPeripheral(_lcdConfig.parallelPinList[pin][PIN_DESC_CLOCK_INDEX]);
  }

  // switch all pins to input mode
  parallelModeSwitch(false);
}

/* LCD bit banging and Communication Stuffs*/

void LcdDriver::registerSelect(const bool& isDataReg) {
  pinWrite(_lcdConfig.regSelectPin, isDataReg);
}

void LcdDriver::comSwitch(const bool& iscomEnabled) {
  pinWrite(_lcdConfig.enablePin, iscomEnabled);
}

void LcdDriver::comModeSwitch(const bool& isReadMode) {
  pinWrite(_lcdConfig.readWritePin, isReadMode);
}

void LcdDriver::comSetup(const bool& isDataReg, const bool& isReadMode) {
  const uint32_t waitTime = isReadMode ? LCD_DATA_READ_DELAY_NANOSEC : LCD_DATA_WRITE_WAIT_NANOSEC;
  // setup so that the lcd knows that we want to talk with it
  registerSelect(isDataReg);
  comModeSwitch(isReadMode);
  _generalTimer.wait(LCD_ADDR_SETUP_TIME_NANOSEC - TIVA_MAX_RISE_TIME);
  comSwitch(true);
  _generalTimer.wait(waitTime);
}

void LcdDriver::comStop(void) {
  _generalTimer.wait(LCD_DATA_SETUP_TIME_NANOSEC);
  comSwitch(false);
  _generalTimer.wait(LCD_DATA_HOLD_TIME_NANOSEC + TIVA_MAX_FALSE_TIME);
}

void LcdDriver::comMaintain(const bool& isReadMode) {
  const uint32_t waitTime = isReadMode ? LCD_DATA_READ_DELAY_NANOSEC : LCD_DATA_WRITE_WAIT_NANOSEC;
  _generalTimer.wait(LCD_DATA_SETUP_TIME_NANOSEC);
  comSwitch(false);
  _generalTimer.wait(LCD_MIN_CYCLE_TIME_NANOSEC - LCD_DATA_WRITE_WAIT_NANOSEC);
  comSwitch(true);
  _generalTimer.wait(waitTime);
}

void LcdDriver::configWrite(void) {
  uint8_t configData[5] = {0};
  configData[0]         = functionSetCommandCreate(false, true, false);
  configData[1]         = displayCommandCreate(true, true, true);
  configData[2]         = LCD_CLEAR_COMMAND;
  configData[3]         = entryModeCommandCreate(true, false);

  dataWrite4Bit(LCD_BEGIN_COMMAND, false);
  parallelDataWrite(configData, 4, false);
}

void LcdDriver::enable(void) {
  _generalTimer.wait(LCD_WARM_UP_TIME_NANOSEC);

  dataWrite4Bit(LCD_STARTUP_COMMAND, true);

  _generalTimer.wait(LCD_FIRST_INIT_TIME_NANOSEC);

  dataWrite4Bit(LCD_STARTUP_COMMAND, true);
  _generalTimer.wait(LCD_SECOND_INIT_TIME_NANOSEC);

  dataWrite4Bit(LCD_STARTUP_COMMAND, true);
  configWrite();
}

/* Led Stuff */

void LcdDriver::backLedSwitch(const bool& isBackLedOn) {
  pinWrite(_lcdConfig.backLightPin, isBackLedOn);
}

/* Content on LCD */
void LcdDriver::displayWrite(const char* dataToWrite) {
  assert(dataToWrite);
  assert(LCD_MAX_PRINT_STRING >= strlen(dataToWrite));

  parallelDataWriteSingle(LCD_CLEAR_COMMAND, false);
  ramDataWrite((uint8_t*)dataToWrite, strlen(dataToWrite), true);
}

void LcdDriver::cursorPositionChange(const uint8_t& cursorX, const uint8_t& cursorY) {
  assert(cursorX <= MAX_LCD_X && cursorY <= MAX_LCD_Y);
  addrCounterChange(cursorY << 6 | cursorX, true);
}

void LcdDriver::displayAppend(const char* dataToAppend) {
  // TODO: consider the effect of reading moving the cursor
  assert(dataToAppend);
  assert(LCD_MAX_PRINT_STRING >= strlen(dataToAppend));
  ramDataWrite((uint8_t*)dataToAppend, strlen(dataToAppend), true);
}

void LcdDriver::newCustomCharAdd(const uint8_t   charPattern[CUSTOM_CHAR_PATTERN_LEN],
                                 const uint32_t& customCharSlot) {
  assert(customCharSlot < MAX_TOTAL_CUSTOM_PATTERN);
  assert(charPattern);

  addrCounterChange(customCharSlot * LCD_MEMUSED_PER_x8_CHAR, false);
  ramDataWrite(charPattern, CUSTOM_CHAR_PATTERN_LEN, false);
}

void LcdDriver::lcdSettingSwitch(const bool& displayOn,
                                 const bool& cursorOn,
                                 const bool& cursorBlinkOn) {
  parallelDataWriteSingle(displayCommandCreate(displayOn, cursorOn, cursorBlinkOn), false);
}

void LcdDriver::lcdReset(void) { parallelDataWriteSingle(LCD_CLEAR_COMMAND, false); }

}  // namespace lcddriver