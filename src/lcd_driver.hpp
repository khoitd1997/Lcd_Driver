#ifndef _LCD_DRIVER_HPP
#define _LCD_DRIVER_HPP

#include <cstdint>

#include "general_timer/general_timer.hpp"

namespace lcddriver {
// max length of a string that can be printed onto LCD
static const uint32_t LCD_MAX_PRINT_STRING = 32;

// 4 or 8 parallel pins can be used to talk with the driver
static const uint32_t TOTAL_PARALLEL_PIN = 4;

// pin description array stuffs
static const uint32_t PIN_DESCRIPTION_LEN  = 3;
static const uint32_t PIN_DESC_CLOCK_INDEX = 0;
static const uint32_t PIN_DESC_PORT_INDEX  = 1;
static const uint32_t PIN_DESC_PIN_INDEX   = 2;

// custom char constants
static const uint32_t MAX_TOTAL_CUSTOM_PATTERN = 8;
static const uint32_t CUSTOM_CHAR_PATTERN_LEN  = 8;

typedef struct {
  uint32_t regSelectPin[PIN_DESCRIPTION_LEN];
  uint32_t readWritePin[PIN_DESCRIPTION_LEN];
  uint32_t enablePin[PIN_DESCRIPTION_LEN];
  uint32_t backLightPin[PIN_DESCRIPTION_LEN];
  uint32_t parallelPinList[TOTAL_PARALLEL_PIN][PIN_DESCRIPTION_LEN];  // contain D0 to D4
} LcdConfig;

class LcdDriver {
 private:
  uint32_t     _totalBitPerPin;
  LcdConfig    _lcdConfig;
  GeneralTimer _generalTimer;

  // parallel data handling using all the allocated pins
  void parallelDataRead(const bool &isDataReg, uint8_t *readDataBuf, const uint32_t &totalReadData);
  void parallelDataWrite(const uint8_t *dataList, const uint32_t &dataLen, const bool &isDataReg);
  void parallelDataWriteSingle(const uint8_t &data, const bool &isDataReg);
  void parallelModeSwitch(const bool &isInput);

  // pin related
  void pinModeSwitch(const uint32_t pinDesc[PIN_DESCRIPTION_LEN], const bool &isInput);
  void pinWrite(const uint32_t pinDesc[PIN_DESCRIPTION_LEN], const bool &output);
  bool pinRead(const uint32_t pinDesc[PIN_DESCRIPTION_LEN]);
  void pinPadConfig(const uint32_t pinDesc[PIN_DESCRIPTION_LEN]);
  void pinDescCheck(uint32_t pinDesc[PIN_DESCRIPTION_LEN]);

  // communication related
  void comSetup(const bool &isDataReg, const bool &isReadMode);
  void comStop(void);
  void comMaintain(const bool &isReadMode);
  void registerSelect(const bool &isDataReg);
  void configWrite(void);
  void comSwitch(const bool &iscomEnabled);
  void comModeSwitch(const bool &isReadMode);

  // command related
  uint8_t entryModeCommandCreate(const bool &cursorRightDir, const bool &displayShiftEnabled);
  uint8_t displayCommandCreate(const bool &displayOn,
                               const bool &cursorOn,
                               const bool &isCursorBlink);
  uint8_t functionSetCommandCreate(const bool &is8BitDataLen,
                                   const bool &is2Lines,
                                   const bool &is5x10Font);
  uint8_t cursorDisplayShiftCommandCreate(const bool &isShiftDisplay, const bool &isRight);

  // auxilary stuffs
  bool    lcdIsBusy(void);
  uint8_t addrCounterGet(void);
  void    addrCounterChange(const uint8_t &addr, const bool &isDataRam);
  void    dataWrite4Bit(const uint32_t &dataToWrite, const bool &stopAfterWrite);

  // ram stuffs
  uint8_t instructionDataRead(void);
  void    ramDataWrite(const uint8_t *data, const uint32_t dataLen, const bool &isTextMode);
  void    ramDataRead(uint8_t *       returnData,
                      const uint32_t &totalDataRead,
                      const uint8_t & startingRamAddr,
                      const bool &    isDataRam);

 public:
  // initialization and setup
  LcdDriver(const LcdConfig &lcdConfig);
  void init(void);
  void enable(void);

  // adding content onto the LCD
  void displayWrite(const char *dataToWrite);
  void displayAppend(const char *dataToAppend);

  // custom char
  void newCustomCharAdd(const uint8_t   charPattern[CUSTOM_CHAR_PATTERN_LEN],
                        const uint32_t &customCharSlot);

  // cursor and others
  // TODO: implement these
  void displaySwitch(const bool &displayStatus);
  void cursorSwitch(const bool &cursorStatus);
  void cursorPositionChange(const uint8_t &cursorX, const uint8_t &cursorY);
  void cursorBlinkSwitch(const bool &cursorBlinkStatus);

  // back led would need relay or transistor to control it
  void backLedSwitch(const bool &isBackLedOn);
};
}  // namespace lcddriver
#endif