#ifndef _GENERAL_TIMER_HPP
#define _GENERAL_TIMER_HPP

#include <cinttypes>

enum TimerUnit : uint64_t {
  UNIT_MILLISEC = 1000,
  UNIT_MICROSEC = 1000000,
  UNIT_NANOSEC  = 1000000000
};

// the sole purpose is for timing, doesn't implement all features of the tiva timer
class GeneralTimer {
 private:
  static bool _isConfigured;
  double      _tickToTimeScale;
  uint64_t    tickToTime(const uint64_t& tickCount);
  uint64_t    timeToTick(const uint64_t& time);
  uint64_t    getTimeStamp(uint32_t timerBase, uint32_t timerName);

 public:
  GeneralTimer(TimerUnit timerUnit);
  void     startTimer(uint64_t& timeStamp);
  uint64_t stopTimer(const uint64_t& intialTimeStamp);
  void     wait(const uint64_t& timeToWait);
};

#endif