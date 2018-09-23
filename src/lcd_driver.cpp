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
#include "tiva_utils/bit_manipulation.h"

// debug
#include "driverlib/uart.h"
#include "utils/uartstdio.h"

namespace lcddriver {

#define MAX_LCD_X 15  // limited by the horizontal length of the LCD
#define MAX_LCD_Y 1   // limited by the vertical len of LCD

/* Timing Variable */

#define COM_TIME_SCALER 7000

// waiting phase time
#define LCD_WARM_UP_TIME_NANOSEC 49000000
#define LCD_FIRST_INIT_TIME_NANOSEC 4500000
#define LCD_SECOND_INIT_TIME_NANOSEC 150000

// data cycle time
#define LCD_PULSE_WIDTH_NANOSEC 200 * COM_TIME_SCALER
#define LCD_MIN_CYCLE_TIME_NANOSEC 410 * COM_TIME_SCALER

// setup and hold time

// for writing
#define LCD_DATA_SETUP_TIME_NANOSEC 45 * COM_TIME_SCALER
#define LCD_DATA_HOLD_TIME_NANOSEC 15 * COM_TIME_SCALER

// for address based
#define LCD_ADDR_SETUP_TIME_NANOSEC 35 * COM_TIME_SCALER
#define LCD_ADDR_HOLD_TIME_NANOSEC 15 * COM_TIME_SCALER

#define TIVA_MAX_RISE_TIME 13
#define TIVA_MAX_FALSE_TIME 14

#define LCD_DATA_WRITE_WAIT_NANOSEC \
  TIVA_MAX_RISE_TIME + LCD_PULSE_WIDTH_NANOSEC - (LCD_DATA_SETUP_TIME_NANOSEC)

#define LCD_DATA_READ_WAIT_NANOSEC 800  // based on datasheet

#define LCD_STARTUP_COMMAND 0b110000
#define LCD_BEGIN_COMMAND 0b100000
#define LCD_CLEAR_COMMAND 0b1
#define LCD_RETURN_HOME_COMMAND 0b10
#define LCD_JUMP_LINE_COMMAND 0xc0

#define LCD_BUSY_BIT 7
#define LCD_ADDR_COUNTER_MASK 0x7f

#define LCD_MEMUSED_PER_x8_CHAR 8

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

  pinDescCheck(_lcdConfig.backLightPin);
  enableClockPeripheral(_lcdConfig.backLightPin[PIN_DESC_CLOCK_INDEX]);
  pinModeSwitch(_lcdConfig.backLightPin, false);
  pinPadConfig(_lcdConfig.backLightPin);

  for (uint32_t pin = 0; pin < TOTAL_PARALLEL_PIN; ++pin) {
    pinDescCheck(_lcdConfig.parallelPinList[pin]);
    enableClockPeripheral(_lcdConfig.parallelPinList[pin][PIN_DESC_CLOCK_INDEX]);
  }

  // switch all pins to input mode
  parallelModeSwitch(false);
}

/* com Stuff*/

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
  const uint32_t waitTime = isReadMode ? LCD_DATA_READ_WAIT_NANOSEC : LCD_DATA_WRITE_WAIT_NANOSEC;
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
  const uint32_t waitTime = isReadMode ? LCD_DATA_READ_WAIT_NANOSEC : LCD_DATA_WRITE_WAIT_NANOSEC;
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

// content
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
  assert(customCharSlot < MAX_CUSTOM_PATTERN);
  assert(charPattern);

  addrCounterChange(customCharSlot * LCD_MEMUSED_PER_x8_CHAR, false);
  ramDataWrite(charPattern, CUSTOM_CHAR_PATTERN_LEN, false);
}

void LcdDriver::addrCounterChange(const uint8_t& addr, const bool& isDataRam) {
  parallelDataWriteSingle(addr | (isDataRam ? BIT(7) : BIT(6)), false);
}

/* RAM stuffs */
void LcdDriver::ramDataWrite(const uint8_t* data, const uint32_t dataLen, const bool& isTextMode) {
  assert(data);
  assert(dataLen > 0);

  for (uint32_t strIndex = 0; strIndex < dataLen; ++strIndex) {
    if (isTextMode) {
      if (isspace(data[strIndex])) {
        if (0x0a == data[strIndex]) {
          // newline case
          parallelDataWriteSingle(LCD_JUMP_LINE_COMMAND, false);
        } else if (0x20 == data[strIndex]) {
          parallelDataWriteSingle(data[strIndex], true);
        }
        // parse special character
      } else if (('`' == data[strIndex]) && (strIndex < dataLen - 1) &&
                 isdigit(data[strIndex + 1]) && (data[strIndex + 1] - '0' < MAX_CUSTOM_PATTERN)) {
        parallelDataWriteSingle(data[strIndex + 1] - '0', true);
        ++strIndex;
      } else {
        parallelDataWriteSingle(data[strIndex], true);
      }
    } else {
      parallelDataWriteSingle(data[strIndex], true);
    }
  }
}

uint8_t LcdDriver::addrCounterGet(void) {
  return bit_get(instructionDataRead(), LCD_ADDR_COUNTER_MASK);
}

bool LcdDriver::lcdIsBusy(void) { return (bool)bit_get(instructionDataRead(), BIT(LCD_BUSY_BIT)); }

uint8_t LcdDriver::instructionDataRead(void) {
  uint8_t dataBuf[1] = {0};
  parallelDataRead(false, dataBuf, 1);
  return dataBuf[0];
}

void LcdDriver::ramDataRead(uint8_t*        returnData,
                            const uint32_t& totalDataRead,
                            const uint8_t&  startingRamAddr,
                            const bool&     isDataRam) {
  assert(returnData);
  assert(totalDataRead > 0);

  // set address b4 read
  addrCounterChange(startingRamAddr, isDataRam);

  parallelDataRead(true, returnData, totalDataRead);
}
}  // namespace lcddriver