#include "lcd_driver.hpp"

#include <cassert>
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

/* Parallel Stuff */
void LcdDriver::parallelModeSwitch(const bool& isInput) {
  for (uint32_t pin = 0; pin < TOTAL_PARALLEL_PIN; ++pin) {
    pinModeSwitch(_lcdConfig.parallelPinList[pin], isInput);
  }
}

void LcdDriver::parallelDataWrite(const uint32_t* dataList,
                                  const uint32_t& dataLen,
                                  const bool&     isDataReg) {
  parallelModeSwitch(false);
  comSetup(isDataReg, false);
  uint32_t _totalBitPerPin = 8 / TOTAL_PARALLEL_PIN;

  for (uint32_t dataIndex = 0; dataIndex < dataLen - 1; ++dataIndex) {
    for (int32_t bitIndex = _totalBitPerPin - 1; bitIndex != -1; --bitIndex) {
      for (uint32_t pin = 0; pin < TOTAL_PARALLEL_PIN; ++pin) {
        pinWrite(_lcdConfig.parallelPinList[pin],
                 bit_get(dataList[dataIndex] >> 4 * bitIndex, BIT(pin)));
      }
      comMaintain(false);
    }
  }

  for (int32_t bitIndex = _totalBitPerPin - 1; bitIndex != -1; --bitIndex) {
    for (uint32_t pin = 0; pin < TOTAL_PARALLEL_PIN; ++pin) {
      pinWrite(_lcdConfig.parallelPinList[pin],
               bit_get(dataList[dataLen - 1] >> 4 * bitIndex, BIT(pin)));
    }
    if (0 != bitIndex) { comMaintain(false); }
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

  for (uint32_t dataIndex = 0; dataIndex < totalReadData - 1; ++dataIndex) {
    for (int32_t bitIndex = _totalBitPerPin - 1; bitIndex != -1; --bitIndex) {
      for (uint32_t pin = 0; pin < TOTAL_PARALLEL_PIN; ++pin) {
        if (pinRead(_lcdConfig.parallelPinList[pin])) {
          bit_set(readDataBuf[dataIndex], BIT(pin + (4 * bitIndex)));
        }
      }
      comMaintain(true);
    }
  }

  for (int32_t bitIndex = _totalBitPerPin - 1; bitIndex != -1; --bitIndex) {
    for (uint32_t pin = 0; pin < TOTAL_PARALLEL_PIN; ++pin) {
      if (pinRead(_lcdConfig.parallelPinList[pin])) {
        bit_set(readDataBuf[totalReadData - 1], BIT(pin + (4 * bitIndex)));
      }
    }
    if (0 != bitIndex) { comMaintain(true); }
  }

  comStop();
}

}  // namespace lcddriver