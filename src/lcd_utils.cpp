#include "lcd_driver.hpp"

#include <cassert>
#include <cctype>
#include <cstdint>

// peripheral
#include "driverlib/gpio.h"
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
/* Pin stuffs */

void LcdDriver::pinDescCheck(uint32_t pinDesc[PIN_DESCRIPTION_LEN]) {
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

/* command helper */
uint8_t LcdDriver::entryModeCommandCreate(const bool& cursorRightDir,
                                          const bool& displayShiftEnabled) {
  uint8_t result = 0;
  bit_set(result, BIT(2));
  cursorRightDir ? bit_set(result, BIT(1)) : 0;
  displayShiftEnabled ? bit_set(result, BIT(0)) : 0;
  return result;
}
uint8_t LcdDriver::displayCommandCreate(const bool& displayOn,
                                        const bool& cursorOn,
                                        const bool& isCursorBlink) {
  uint8_t result = 0;
  bit_set(result, BIT(3));
  displayOn ? bit_set(result, BIT(2)) : 0;
  cursorOn ? bit_set(result, BIT(1)) : 0;
  isCursorBlink ? bit_set(result, BIT(0)) : 0;
  return result;
}
uint8_t LcdDriver::functionSetCommandCreate(const bool& is8BitDataLen,
                                            const bool& is2Lines,
                                            const bool& is5x10Font) {
  uint8_t result = 0;
  bit_set(result, BIT(5));
  is8BitDataLen ? bit_set(result, BIT(4)) : 0;
  is2Lines ? bit_set(result, BIT(3)) : 0;
  is5x10Font ? bit_set(result, BIT(2)) : 0;
  return result;
}

uint8_t LcdDriver::cursorDisplayShiftCommandCreate(const bool& isShiftDisplay,
                                                   const bool& isRight) {
  uint8_t result = 0;
  bit_set(result, BIT(4));
  isShiftDisplay ? bit_set(result, BIT(3)) : 0;
  isRight ? bit_set(result, BIT(2)) : 0;
  return result;
}

/* Parallel Stuff */
void LcdDriver::parallelModeSwitch(const bool& isInput) {
  for (uint32_t pin = 0; pin < TOTAL_PARALLEL_PIN; ++pin) {
    pinModeSwitch(_lcdConfig.parallelPinList[pin], isInput);
  }
}

void LcdDriver::parallelDataWriteSingle(const uint8_t& data, const bool& isDataReg) {
  parallelModeSwitch(false);
  comSetup(isDataReg, false);
  for (int32_t bitIndex = _totalBitPerPin - 1; bitIndex != -1; --bitIndex) {
    for (uint32_t pin = 0; pin < TOTAL_PARALLEL_PIN; ++pin) {
      pinWrite(_lcdConfig.parallelPinList[pin], bit_get(data >> 4 * bitIndex, BIT(pin)));
    }
    if (0 != bitIndex) { comMaintain(false); }
  }
  comStop();
}

void LcdDriver::parallelDataWrite(const uint8_t*  dataList,
                                  const uint32_t& dataLen,
                                  const bool&     isDataReg) {
  parallelModeSwitch(false);
  comSetup(isDataReg, false);

  for (uint32_t dataIndex = 0; dataIndex < dataLen; ++dataIndex) {
    for (int32_t bitIndex = _totalBitPerPin - 1; bitIndex != -1; --bitIndex) {
      for (uint32_t pin = 0; pin < TOTAL_PARALLEL_PIN; ++pin) {
        pinWrite(_lcdConfig.parallelPinList[pin],
                 bit_get(dataList[dataIndex] >> 4 * bitIndex, BIT(pin)));
      }
      if ((0 != bitIndex) || ((dataLen - 1) != dataIndex)) { comMaintain(false); }
    }
  }
  comStop();
}

void LcdDriver::parallelDataRead(const bool&     isDataReg,
                                 uint8_t*        readDataBuf,
                                 const uint32_t& totalReadData) {
  parallelModeSwitch(true);
  comSetup(isDataReg, true);

  for (uint32_t dataBufIndex = 0; dataBufIndex < totalReadData; ++dataBufIndex) {
    readDataBuf[dataBufIndex] = 0;
  }

  for (uint32_t dataIndex = 0; dataIndex < totalReadData; ++dataIndex) {
    for (int32_t bitIndex = _totalBitPerPin - 1; bitIndex != -1; --bitIndex) {
      for (uint32_t pin = 0; pin < TOTAL_PARALLEL_PIN; ++pin) {
        if (pinRead(_lcdConfig.parallelPinList[pin])) {
          bit_set(readDataBuf[dataIndex], BIT(pin + (4 * bitIndex)));
        }
      }
      if (0 != bitIndex || ((totalReadData - 1) != dataIndex)) { comMaintain(true); }
    }
  }

  comStop();
}

void LcdDriver::dataWrite4Bit(const uint32_t& dataToWrite, const bool& stopAfterWrite) {
  comSetup(false, false);
  for (uint32_t pin = 0; pin < TOTAL_PARALLEL_PIN; ++pin) {
    pinWrite(_lcdConfig.parallelPinList[pin],
             bit_get(dataToWrite >> ((4 == TOTAL_PARALLEL_PIN) ? 4 : 0), BIT(pin)));
  }
  if (stopAfterWrite) {
    comStop();
  } else {
    comMaintain(false);
  }
}

void LcdDriver::addrCounterChange(const uint8_t& addr, const bool& isDataRam) {
  parallelDataWriteSingle(addr | (isDataRam ? BIT(7) : BIT(6)), false);
}

/* RAM stuffs */

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
                 isdigit(data[strIndex + 1]) &&
                 (data[strIndex + 1] - '0' < MAX_TOTAL_CUSTOM_PATTERN)) {
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

uint8_t LcdDriver::instructionDataRead(void) {
  uint8_t dataBuf[1] = {0};
  parallelDataRead(false, dataBuf, 1);
  return dataBuf[0];
}

bool LcdDriver::lcdIsBusy(void) { return (bool)bit_get(instructionDataRead(), BIT(LCD_BUSY_BIT)); }

}  // namespace lcddriver