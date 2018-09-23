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

  auto lcdDriver = LcdDriver(lcdConfig);
  lcdDriver.init();
  lcdDriver.enable();
  // lcdDriver.displayWrite("Nice\nNewLine");
  // lcdDriver.cursorPositionChange(15, 0);

  for (;;) {
    // for now just loop
  }
  return 0;
}
