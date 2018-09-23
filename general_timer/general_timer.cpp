#include "general_timer.hpp"

#include <cassert>
#include <cinttypes>
#include <cstdio>

// TivaC
// peripheral
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"

// hardware
#include "Tivaware_Dep/inc/hw_timer.h"
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"

// timer value config
static const uint64_t TIMER_LOAD = 18446744073709551610;
static const uint32_t TIMER_MODE = TIMER_CFG_PERIODIC_UP;

// timer register configuration
static const uint32_t TIMER_BASE  = WTIMER0_BASE;
static const uint32_t TIMER_CLOCK = SYSCTL_PERIPH_WTIMER0;
static const uint32_t TIMER_NAME  = TIMER_A;

bool GeneralTimer::_isConfigured = false;

GeneralTimer::GeneralTimer(TimerUnit timerUnit)
    : _tickToTimeScale(timerUnit / (double)(SysCtlClockGet())) {
  if (false == _isConfigured) {
    ROM_SysCtlPeripheralEnable(TIMER_CLOCK);
    while (!ROM_SysCtlPeripheralReady(TIMER_CLOCK)) {
      // wait until timer peripheral clock is ready
    }

    // use a concacentated 64 bit timer clock
    TimerConfigure(TIMER_BASE, TIMER_MODE);
    TimerLoadSet64(TIMER_BASE, TIMER_LOAD);
    TimerEnable(TIMER_BASE, TIMER_NAME);
    _isConfigured = true;
  }
}

void GeneralTimer::startTimer(uint64_t& timeStamp) {
  timeStamp = getTimeStamp(TIMER_BASE, TIMER_NAME);
}

uint64_t GeneralTimer::getTimeStamp(uint32_t timerBase, uint32_t timerName) {
  return TimerValueGet64(timerBase);
}

uint64_t tickToNanoSec(uint64_t tickCount);

uint64_t inline GeneralTimer::tickToTime(const uint64_t& tickCount) {
  return (tickCount * _tickToTimeScale);
}
uint64_t inline GeneralTimer::timeToTick(const uint64_t& timeAmount) {
  assert(_tickToTimeScale != 0);
  return (timeAmount / _tickToTimeScale);
}
// return time elapsed in variable unit
uint64_t GeneralTimer::stopTimer(const uint64_t& intialTimeStamp) {
  uint64_t currTimeStamp = getTimeStamp(TIMER_BASE, TIMER_NAME);

  if (currTimeStamp > intialTimeStamp) {
    // no overflow case
    return tickToTime(currTimeStamp - intialTimeStamp);
  } else {
    // this case meant overflow, assuming overflow once
    return tickToTime(((TIMER_LOAD - intialTimeStamp) + currTimeStamp));
  }
}

void GeneralTimer::wait(const uint64_t& timeToWait) {
  uint64_t currTimeStamp  = TimerValueGet64(TIMER_BASE);
  uint64_t tickToWait     = timeToTick(timeToWait);
  uint64_t overFlowOffset = 0;
  while (TimerValueGet64(TIMER_BASE) + overFlowOffset - currTimeStamp < tickToWait) {
    if (currTimeStamp > TimerValueGet64(TIMER_BASE)) {
      overFlowOffset = TIMER_LOAD - currTimeStamp;
    }
    // wait until the count is over
  }
}