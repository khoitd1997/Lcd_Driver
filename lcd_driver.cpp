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
#include "driverlib/uart.h"
#include "utils/uartstdio.h"

// hardware
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"

// application
#include "general_timer/general_timer.hpp"
#include "tiva_utils/bit_manipulation.h"

/* Timing Variable */

#define COM_TIME_SCALER 10000

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

#define LCD_DATA_WAIT_NANOSEC \
  TIVA_MAX_RISE_TIME + LCD_PULSE_WIDTH_NANOSEC - (LCD_DATA_SETUP_TIME_NANOSEC)

#define LCD_STARTUP_COMMAND 0b110000
#define LCD_BEGIN_COMMAND 0b100000
#define LCD_CLEAR_COMMAND 0b1
#define LCD_RETURN_HOME_COMMAND 0b10
#define LCD_JUMP_LINE_COMMAND 0xc0

#define LCD_CHAR_A 0b01000001
#define LCD_CHAR_0 0b00110000

static void enableClockPeripheral(const uint32_t& clockMask) {
  SysCtlPeripheralEnable(clockMask);
  while (!SysCtlPeripheralReady(clockMask)) {
    // wait for clock to be ready
  }
}

static void pinDescCheck(uint32_t pinDesc[PIN_DESCRIPTION_LEN]) {
  const uint32_t clockFlag = pinDesc[PIN_DESC_CLOCK_INDEX];
  const uint32_t portFlag  = pinDesc[PIN_DESC_PORT_INDEX];
  const uint32_t pinFlag   = pinDesc[PIN_DESC_PIN_INDEX];

  assert((clockFlag == SYSCTL_PERIPH_GPIOB) || (clockFlag == SYSCTL_PERIPH_GPIOA) ||
         (clockFlag == SYSCTL_PERIPH_GPIOC) || (clockFlag == SYSCTL_PERIPH_GPIOD) ||
         (clockFlag == SYSCTL_PERIPH_GPIOE) || (clockFlag == SYSCTL_PERIPH_GPIOF));

  assert((portFlag == GPIO_PORTA_BASE) || (portFlag == GPIO_PORTB_BASE) ||
         (portFlag == GPIO_PORTC_BASE) || (portFlag == GPIO_PORTD_BASE) ||
         (portFlag == GPIO_PORTE_BASE) || (portFlag == GPIO_PORTF_BASE));

  assert((pinFlag == GPIO_PIN_0) || (pinFlag == GPIO_PIN_1) || (pinFlag == GPIO_PIN_2) ||
         (pinFlag == GPIO_PIN_5) || (pinFlag == GPIO_PIN_3) || (pinFlag == GPIO_PIN_6) ||
         (pinFlag == GPIO_PIN_4) || (pinFlag == GPIO_PIN_7));

  // check for specially allocated pins that should not be used
  assert(!(GPIO_PIN_0 == pinFlag && GPIO_PORTA_BASE == portFlag) &&
         !(GPIO_PIN_1 == pinFlag && GPIO_PORTA_BASE == portFlag) &&
         !(GPIO_PIN_5 == pinFlag && GPIO_PORTA_BASE == portFlag) &&
         !(GPIO_PIN_4 == pinFlag && GPIO_PORTA_BASE == portFlag) &&
         !(GPIO_PIN_3 == pinFlag && GPIO_PORTA_BASE == portFlag) &&
         !(GPIO_PIN_2 == pinFlag && GPIO_PORTA_BASE == portFlag) &&

         !(GPIO_PIN_3 == pinFlag && GPIO_PORTB_BASE == portFlag) &&
         !(GPIO_PIN_2 == pinFlag && GPIO_PORTB_BASE == portFlag) &&

         !(GPIO_PIN_3 == pinFlag && GPIO_PORTC_BASE == portFlag) &&
         !(GPIO_PIN_2 == pinFlag && GPIO_PORTC_BASE == portFlag) &&
         !(GPIO_PIN_1 == pinFlag && GPIO_PORTC_BASE == portFlag) &&
         !(GPIO_PIN_0 == pinFlag && GPIO_PORTC_BASE == portFlag) &&

         !(GPIO_PIN_7 == pinFlag && GPIO_PORTD_BASE == portFlag) &&

         !(GPIO_PIN_0 == pinFlag && GPIO_PORTF_BASE == portFlag));
}

LcdDriver::LcdDriver(const LcdConfig& lcdconfig)
    : _lcdConfig(lcdconfig), _generalTimer(GeneralTimer(UNIT_NANOSEC)) {}

void LcdDriver::init(void) {
  // initialize clock and set pin types
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
  parallelModeSwitch(false);
}

/* Pin stuffs */

void LcdDriver::pinModeSwitch(const uint32_t pinDesc[PIN_DESCRIPTION_LEN], const bool& isInput) {
  if (isInput) {
    GPIOPinTypeGPIOInput(pinDesc[PIN_DESC_PORT_INDEX], pinDesc[PIN_DESC_PIN_INDEX]);
  } else {
    GPIOPinTypeGPIOOutput(pinDesc[PIN_DESC_PORT_INDEX], pinDesc[PIN_DESC_PIN_INDEX]);
  }
}

void LcdDriver::pinWrite(const uint32_t pinDesc[PIN_DESCRIPTION_LEN], const bool& output) {
  uint8_t pinOutput = output ? pinDesc[PIN_DESC_PIN_INDEX] : 0;
  GPIOPinWrite(pinDesc[PIN_DESC_PORT_INDEX], pinDesc[PIN_DESC_PIN_INDEX], pinOutput);
}

bool LcdDriver::pinRead(const uint32_t pinDesc[PIN_DESCRIPTION_LEN]) {
  return GPIOPinRead(pinDesc[PIN_DESC_PORT_INDEX], pinDesc[PIN_DESC_PIN_INDEX]) ? true : false;
}

void LcdDriver::pinPadConfig(const uint32_t pinDesc[PIN_DESCRIPTION_LEN]) {
  // 8 mA drive strength with slew rate control to ensure rise/fall time is in specs
  GPIOPadConfigSet(pinDesc[PIN_DESC_PORT_INDEX],
                   pinDesc[PIN_DESC_PIN_INDEX],
                   GPIO_STRENGTH_8MA,
                   GPIO_PIN_TYPE_STD);
}

/* Parallel Stuff */
void LcdDriver::parallelModeSwitch(const bool& isInput) {
  for (uint32_t pin = 0; pin < TOTAL_PARALLEL_PIN; ++pin) {
    pinModeSwitch(_lcdConfig.parallelPinList[pin], isInput);
  }
}

void LcdDriver::parallelDataWrite(const uint32_t* dataList,
                                  const uint32_t& dataLen,
                                  const bool&     isDataReg) {
  comSetup(isDataReg, false);
  uint32_t totalBitPerPin = 8 / TOTAL_PARALLEL_PIN;

  for (uint32_t dataIndex = 0; dataIndex < dataLen - 1; ++dataIndex) {
    for (uint32_t bitIndex = totalBitPerPin - 1; bitIndex != -1; --bitIndex) {
      for (uint32_t pin = 0; pin < TOTAL_PARALLEL_PIN; ++pin) {
        pinWrite(_lcdConfig.parallelPinList[pin],
                 bit_get(dataList[dataIndex] >> 4 * bitIndex, BIT(pin)));
      }
      comMaintain(isDataReg, false);
    }
  }

  for (uint32_t bitIndex = totalBitPerPin - 1; bitIndex != -1; --bitIndex) {
    for (uint32_t pin = 0; pin < TOTAL_PARALLEL_PIN; ++pin) {
      pinWrite(_lcdConfig.parallelPinList[pin],
               bit_get(dataList[dataLen - 1] >> 4 * bitIndex, BIT(pin)));
    }
    if (0 != bitIndex) { comMaintain(isDataReg, false); }
  }
  comStop(isDataReg, false);
}

uint32_t LcdDriver::parallelDataRead(const bool& isDataReg, const bool& isReadMode) {
  comSetup(isDataReg, isReadMode);
  uint32_t result = 0;
  for (uint32_t pin = 0; pin < TOTAL_PARALLEL_PIN; ++pin) {
    if (pinRead(_lcdConfig.parallelPinList[pin])) { bit_set(result, BIT(pin)); }
  }

  return result;
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
  // setup so that the lcd knows that we want to talk with it
  registerSelect(isDataReg);
  comModeSwitch(isReadMode);
  _generalTimer.wait(LCD_ADDR_SETUP_TIME_NANOSEC - TIVA_MAX_RISE_TIME);
  comSwitch(true);
  _generalTimer.wait(LCD_DATA_WAIT_NANOSEC);
}

void LcdDriver::comStop(const bool& isDataReg, const bool& isReadMode) {
  _generalTimer.wait(LCD_DATA_SETUP_TIME_NANOSEC);
  comSwitch(false);
  _generalTimer.wait(LCD_DATA_HOLD_TIME_NANOSEC + TIVA_MAX_FALSE_TIME);
}

void LcdDriver::comMaintain(const bool& isDataReg, const bool& isReadMode) {
  _generalTimer.wait(LCD_DATA_SETUP_TIME_NANOSEC);
  comSwitch(false);
  _generalTimer.wait(LCD_MIN_CYCLE_TIME_NANOSEC - LCD_DATA_WAIT_NANOSEC);
  comSwitch(true);
  _generalTimer.wait(LCD_DATA_WAIT_NANOSEC);
}

void LcdDriver::beginSeqWrite(void) {
  uint32_t startupCommand = LCD_BEGIN_COMMAND;
  if (4 == TOTAL_PARALLEL_PIN) { startupCommand >>= 4; }

  comSetup(false, false);
  for (uint32_t pin = 0; pin < TOTAL_PARALLEL_PIN; ++pin) {
    pinWrite(_lcdConfig.parallelPinList[pin], bit_get(startupCommand, BIT(pin)));
  }
  comMaintain(false, false);
}

void LcdDriver::configWrite(void) {
  uint32_t configData[5] = {0};
  configData[0]          = createFunctionSetCommand(false, true, false);
  configData[1]          = createDisplayCommand(true, true, true);
  configData[2]          = LCD_CLEAR_COMMAND;
  configData[3]          = createEntryModeCommand(true, false);

  beginSeqWrite();

  parallelDataWrite(configData, 4, false);
}

void LcdDriver::enable(void) {
  _generalTimer.wait(LCD_WARM_UP_TIME_NANOSEC);

  startupSeqWrite();

  _generalTimer.wait(LCD_FIRST_INIT_TIME_NANOSEC);

  startupSeqWrite();
  _generalTimer.wait(LCD_SECOND_INIT_TIME_NANOSEC);

  startupSeqWrite();
  configWrite();
}

void LcdDriver::startupSeqWrite(void) {
  uint32_t startupCommand = LCD_STARTUP_COMMAND;
  if (4 == TOTAL_PARALLEL_PIN) { startupCommand >>= 4; }

  comSetup(false, false);
  for (uint32_t pin = 0; pin < TOTAL_PARALLEL_PIN; ++pin) {
    pinWrite(_lcdConfig.parallelPinList[pin], bit_get(startupCommand, BIT(pin)));
  }
  comStop(false, false);
}

/* Led Stuff */

void LcdDriver::backLedSwitch(const bool& isBackLedOn) {
  pinWrite(_lcdConfig.backLightPin, isBackLedOn);
}

/* User Stuff */

/* command helper */
uint32_t LcdDriver::createEntryModeCommand(const bool& cursorRightDir,
                                           const bool& displayShiftEnabled) {
  uint32_t result = 0;
  bit_set(result, BIT(2));
  cursorRightDir ? bit_set(result, BIT(1)) : 0;
  displayShiftEnabled ? bit_set(result, BIT(0)) : 0;
  return result;
}
uint32_t LcdDriver::createDisplayCommand(const bool& displayOn,
                                         const bool& cursorOn,
                                         const bool& isCursorBlink) {
  uint32_t result = 0;
  bit_set(result, BIT(3));
  displayOn ? bit_set(result, BIT(2)) : 0;
  cursorOn ? bit_set(result, BIT(1)) : 0;
  isCursorBlink ? bit_set(result, BIT(0)) : 0;
  return result;
}
uint32_t LcdDriver::createFunctionSetCommand(const bool& is8BitDataLen,
                                             const bool& is2Lines,
                                             const bool& is5x10Font) {
  uint32_t result = 0;
  bit_set(result, BIT(5));
  is8BitDataLen ? bit_set(result, BIT(4)) : 0;
  is2Lines ? bit_set(result, BIT(3)) : 0;
  is5x10Font ? bit_set(result, BIT(2)) : 0;
  return result;
}

uint32_t LcdDriver::createCursorDisplayShiftCommand(const bool& isShiftDisplay,
                                                    const bool& isRight) {
  uint32_t result = 0;
  bit_set(result, BIT(4));
  isShiftDisplay ? bit_set(result, BIT(3)) : 0;
  isRight ? bit_set(result, BIT(2)) : 0;
  return result;
}

// content
void LcdDriver::displayWrite(const char* dataToWrite) {
  assert(dataToWrite);
  assert(LCD_MAX_PRINT_STRING >= strlen(dataToWrite));

  uint32_t startComamndList[] = {LCD_CLEAR_COMMAND};
  parallelDataWrite(startComamndList, 1, false);
  ramDataWrite(dataToWrite);
}

void LcdDriver::displayAppend(const char* dataToAppend) {}
/* RAM stuffs */
void LcdDriver::ramDataWrite(const char* data) {
  uint32_t dataList[1] = {0};

  for (uint32_t strIndex = 0; strIndex <= strlen(data); ++strIndex) {
    if (isalpha(data[strIndex])) {
      dataList[0] = data[strIndex];
      parallelDataWrite(dataList, 1, true);
    } else if (isspace(data[strIndex])) {
      if (0x0a == data[strIndex]) {
        // newline case
        dataList[0] = LCD_JUMP_LINE_COMMAND;
        parallelDataWrite(dataList, 1, false);
      } else if (0x20 == data[strIndex]) {
        dataList[0] = data[strIndex];
        parallelDataWrite(dataList, 1, true);
      }
    }
  }

  // uint32_t dataList[]     = {LCD_CHAR_0,
  //                        LCD_CHAR_0,
  //                        LCD_CHAR_0,
  //                        LCD_CHAR_0,
  //                        LCD_CHAR_0,
  //                        LCD_CHAR_0,
  //                        LCD_CHAR_0,
  //                        LCD_CHAR_0,
  //                        LCD_CHAR_0,
  //                        LCD_CHAR_0,
  //                        LCD_CHAR_0,
  //                        LCD_CHAR_0,
  //                        LCD_CHAR_0,
  //                        LCD_CHAR_0};
  // uint32_t commandList[3] = {0};

  // uint32_t startComamndList[5] = {LCD_RETURN_HOME_COMMAND, 0x80};

  // commandList[0] = createCursorDisplayShiftCommand(true, true);
  // commandList[1] = createCursorDisplayShiftCommand(true, true);
  // commandList[2] = createCursorDisplayShiftCommand(true, true);

  // _generalTimer.wait(500000000);
  // parallelDataWrite(startComamndList, 2, false);
  // _generalTimer.wait(500000000);
  // parallelDataWrite(dataList, 2, true);
  // parallelDataWrite(startComamndList, 1, false);
  // _generalTimer.wait(500000000);
  // parallelDataWrite(dataList, 2, true);
  // _generalTimer.wait(500000000);

  // parallelDataWrite(commandList, 1, false);
  // _generalTimer.wait(500000000);
  // parallelDataWrite(dataList, 1, true);
  // parallelDataWrite(commandList, 1, false);
  // _generalTimer.wait(500000000);
  // parallelDataWrite(dataList, 1, true);
  // parallelDataWrite(commandList, 1, false);
  // _generalTimer.wait(500000000);
  for (;;) {
    // parallelDataWrite(commandList, 1, false);
    // _generalTimer.wait(900000000);
    // parallelDataWrite(dataList, 2, true);
    // _generalTimer.wait(900000000);
  }
}
void ramDataRead(uint8_t* returnData) {}