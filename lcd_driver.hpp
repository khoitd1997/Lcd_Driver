#ifndef _LCD_DRIVER_HPP
#define _LCD_DRIVER_HPP

#include <cstdint>

#include "general_timer/general_timer.hpp"

namespace lcddriver {

static const uint32_t LCD_MAX_PRINT_STRING = 32;

static const uint32_t TOTAL_PARALLEL_PIN  = 4;
static const uint32_t PIN_DESCRIPTION_LEN = 3;

static const uint32_t PIN_DESC_CLOCK_INDEX = 0;
static const uint32_t PIN_DESC_PORT_INDEX  = 1;
static const uint32_t PIN_DESC_PIN_INDEX   = 2;

static const uint32_t MAX_CUSTOM_PATTERN      = 16;
static const uint32_t CUSTOM_CHAR_PATTERN_LEN = 8;

typedef struct {
  uint32_t regSelectPin[PIN_DESCRIPTION_LEN];
  uint32_t readWritePin[PIN_DESCRIPTION_LEN];
  uint32_t enablePin[PIN_DESCRIPTION_LEN];
  uint32_t backLightPin[PIN_DESCRIPTION_LEN];
  uint32_t parallelPinList[TOTAL_PARALLEL_PIN][PIN_DESCRIPTION_LEN];  // contain D0 to D4
} LcdConfig;
// TODO: consider error code or exception

class LcdDriver {
 private:
  uint32_t     _totalBitPerPin;
  LcdConfig    _lcdConfig;
  GeneralTimer _generalTimer;

  // parallel data handling
  void parallelDataRead(const bool &isDataReg, uint8_t *readDataBuf, const uint32_t &totalReadData);
  void parallelDataWrite(const uint8_t *dataList, const uint32_t &dataLen, const bool &isDataReg);
  void parallelDataWriteSingle(const uint8_t &data, const bool &isDataReg);
  void parallelModeSwitch(const bool &isInput);

  // single pin
  void pinModeSwitch(const uint32_t pinDesc[PIN_DESCRIPTION_LEN], const bool &isInput);
  void pinWrite(const uint32_t pinDesc[PIN_DESCRIPTION_LEN], const bool &output);
  bool pinRead(const uint32_t pinDesc[PIN_DESCRIPTION_LEN]);
  void pinPadConfig(const uint32_t pinDesc[PIN_DESCRIPTION_LEN]);
  void pinDescCheck(uint32_t pinDesc[PIN_DESCRIPTION_LEN]);

  // com related
  void comSetup(const bool &isDataReg, const bool &isReadMode);
  void comStop(void);
  void comMaintain(const bool &isReadMode);
  void registerSelect(const bool &isDataReg);
  void configWrite(void);
  void comSwitch(const bool &iscomEnabled);
  void comModeSwitch(const bool &isReadMode);
  void startupSeqWrite(void);

  // TODO: rename funcs
  // command related
  uint8_t createEntryModeCommand(const bool &cursorRightDir, const bool &displayShiftEnabled);
  uint8_t createDisplayCommand(const bool &displayOn,
                               const bool &cursorOn,
                               const bool &isCursorBlink);
  uint8_t createFunctionSetCommand(const bool &is8BitDataLen,
                                   const bool &is2Lines,
                                   const bool &is5x10Font);
  uint8_t createCursorDisplayShiftCommand(const bool &isShiftDisplay, const bool &isRight);

  void    beginSeqWrite(void);
  bool    isBusy(void);
  uint8_t getAddrCounter(void);

  // ram stuffs
  void    changeAddrCounter(const uint8_t &addr, const bool &isDataRam);
  uint8_t instructionDataRead(void);
  void    ramDataWrite(const uint8_t *data, const uint32_t dataLen);
  void    ramDataRead(uint8_t *       returnData,
                      const uint32_t &totalDataRead,
                      const uint8_t & startingRamAddr,
                      const bool &    isDataRam);

 public:
  LcdDriver(const LcdConfig &lcdConfig);
  void init(void);
  void enable(void);

  // content
  void displayWrite(const char *dataToWrite);  // flush current buffer and add new data
  void displayAppend(const char *dataToAppend);

  // custom char
  void addNewCustomChar(const uint8_t  charPattern[CUSTOM_CHAR_PATTERN_LEN],
                        const uint32_t customCharSlot);

  // cursor and others
  void displaySwitch(const bool &displayStatus);
  void cursorSwitch(const bool &cursorStatus);
  void cursorPositionChange(const uint8_t &cursorX, const uint8_t &cursorY);
  void cursorBlinkSwitch(const bool &cursorBlinkStatus);

  // back led
  void backLedSwitch(const bool &isBackLedOn);  // would need relay
};
}  // namespace lcddriver
#endif