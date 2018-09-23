#include "lcd_driver.hpp"

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

using namespace lcddriver;

int main(void) {
  // 80 MHz clock
  SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ | SYSCTL_OSC_MAIN);
  LcdConfig lcdConfig;

  // B6
  lcdConfig.backLightPin[PIN_DESC_CLOCK_INDEX] = SYSCTL_PERIPH_GPIOB;
  lcdConfig.backLightPin[PIN_DESC_PORT_INDEX]  = GPIO_PORTB_BASE;
  lcdConfig.backLightPin[PIN_DESC_PIN_INDEX]   = GPIO_PIN_6;

  // B7 to RS
  lcdConfig.regSelectPin[PIN_DESC_CLOCK_INDEX] = SYSCTL_PERIPH_GPIOB;
  lcdConfig.regSelectPin[PIN_DESC_PORT_INDEX]  = GPIO_PORTB_BASE;
  lcdConfig.regSelectPin[PIN_DESC_PIN_INDEX]   = GPIO_PIN_7;

  // F4 to RW
  lcdConfig.readWritePin[PIN_DESC_CLOCK_INDEX] = SYSCTL_PERIPH_GPIOF;
  lcdConfig.readWritePin[PIN_DESC_PORT_INDEX]  = GPIO_PORTF_BASE;
  lcdConfig.readWritePin[PIN_DESC_PIN_INDEX]   = GPIO_PIN_4;

  // E3 to EN
  lcdConfig.enablePin[PIN_DESC_CLOCK_INDEX] = SYSCTL_PERIPH_GPIOE;
  lcdConfig.enablePin[PIN_DESC_PORT_INDEX]  = GPIO_PORTE_BASE;
  lcdConfig.enablePin[PIN_DESC_PIN_INDEX]   = GPIO_PIN_3;

  // E2 to D4
  lcdConfig.parallelPinList[0][PIN_DESC_CLOCK_INDEX] = SYSCTL_PERIPH_GPIOE;
  lcdConfig.parallelPinList[0][PIN_DESC_PORT_INDEX]  = GPIO_PORTE_BASE;
  lcdConfig.parallelPinList[0][PIN_DESC_PIN_INDEX]   = GPIO_PIN_2;

  // E1 to D5
  lcdConfig.parallelPinList[1][PIN_DESC_CLOCK_INDEX] = SYSCTL_PERIPH_GPIOE;
  lcdConfig.parallelPinList[1][PIN_DESC_PORT_INDEX]  = GPIO_PORTE_BASE;
  lcdConfig.parallelPinList[1][PIN_DESC_PIN_INDEX]   = GPIO_PIN_1;

  // E0 to D6
  lcdConfig.parallelPinList[2][PIN_DESC_CLOCK_INDEX] = SYSCTL_PERIPH_GPIOE;
  lcdConfig.parallelPinList[2][PIN_DESC_PORT_INDEX]  = GPIO_PORTE_BASE;
  lcdConfig.parallelPinList[2][PIN_DESC_PIN_INDEX]   = GPIO_PIN_0;

  // D7 to D7
  lcdConfig.parallelPinList[3][PIN_DESC_CLOCK_INDEX] = SYSCTL_PERIPH_GPIOD;
  lcdConfig.parallelPinList[3][PIN_DESC_PORT_INDEX]  = GPIO_PORTD_BASE;
  lcdConfig.parallelPinList[3][PIN_DESC_PIN_INDEX]   = GPIO_PIN_6;

  // create and itialize lcd driver class
  auto lcdDriver = LcdDriver(lcdConfig);
  lcdDriver.init();
  lcdDriver.enable();

  // get timer with millisecond scale
  auto generalTimer = GeneralTimer(UNIT_MILLISEC);

  // create character pattern
  uint8_t charPattern0[] = {0b11111, 0b11000, 0b10100, 0b10111, 0b10101, 0b10101, 0b10101, 0b11111};
  uint8_t charPattern1[] = {0b10000, 0b01111, 0b01001, 0b01001, 0b01001, 0b01001, 0b01001, 0b01001};
  uint8_t charPattern2[] = {0b10000, 0b01000, 0b01011, 0b01110, 0b01010, 0b00010, 0b00010, 0b00010};
  lcdDriver.newCustomCharAdd(charPattern0, 0);
  lcdDriver.newCustomCharAdd(charPattern1, 1);
  lcdDriver.newCustomCharAdd(charPattern2, 2);

  for (;;) {
    lcdDriver.displayWrite("`0`1`2");
    lcdDriver.displayAppend("\nA string");
    generalTimer.wait(2000);
  }
  return 0;
}
