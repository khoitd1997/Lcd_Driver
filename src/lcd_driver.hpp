/**
 * @brief header file for LcdDriver class
 *
 * @file lcd_driver.hpp
 * @author Khoi Trinh
 * @date 2018-09-23
 */

#ifndef _LCD_DRIVER_HPP
#define _LCD_DRIVER_HPP

#include <cstdint>

#include "general_timer/general_timer.hpp"

/**
 * @brief the namespace of the LcdDriver, all lcd controller related files are under this namespace
 */
namespace lcddriver {

/**
 * @brief Max length of a string that can be printed onto LCD
 */
static const uint32_t LCD_MAX_PRINT_STRING = 32;

/**
 * @brief Total pins used for sending/receiving data from the lcd driver(like D0-D7), the 1602 can
 * talk using either 4 or 8 pins
 */
static const uint32_t TOTAL_PARALLEL_PIN = 4;

/**
 * @brief how long is the array of array describing each pin, there are 3 members for describing:
 * the clock port(like SYSCTL_PERIPH_GPIOB), the gpio port(like GPIO_PORTB_BASE) and the gpio
 * pin(like GPIO_PIN_6)
 */
static const uint32_t PIN_DESCRIPTION_LEN = 3;

/**
 * @brief array index of the member that has the macro for the gpio clock port
 */
static const uint32_t PIN_DESC_CLOCK_INDEX = 0;
/**
 * @brief array index of the member that has the macro for the gpio port
 */
static const uint32_t PIN_DESC_PORT_INDEX = 1;

/**
 * @brief  array index of the member that has the macro for the gpio pin
 */
static const uint32_t PIN_DESC_PIN_INDEX = 2;

/**
 * @brief how many patterns that the lcd controller can have, 8 is for 5x8 font, although 4 for 5x10
 * font
 */
static const uint32_t MAX_TOTAL_CUSTOM_PATTERN = 8;
/**
 * @brief the length of the array describing a custom pattern, it will be 8 byte total
 */
static const uint32_t CUSTOM_CHAR_PATTERN_LEN = 8;

/**
 * @brief the structure used for carrying the lcd controller settings
 * Each pin description is an array of 3 members describing:
 * the clock port(like SYSCTL_PERIPH_GPIOB), the gpio port(like GPIO_PORTB_BASE) and the gpio
 * pin(like GPIO_PIN_6)
 */
typedef struct {
  bool     useBacklight;  //!< whether the backlight can be turned on/off using pin
  uint32_t regSelectPin[PIN_DESCRIPTION_LEN];  //!< arrays describing the RS(register selector) pin
  uint32_t readWritePin[PIN_DESCRIPTION_LEN];  //!< arrays for descirbing the RS(Read/Write pin)
  uint32_t enablePin[PIN_DESCRIPTION_LEN];     //!< arrays for describing the enable pin
  uint32_t backLightPin[PIN_DESCRIPTION_LEN];  //!< arrays for controlling the backlight pin, if you
                                               //!< don't have one just don't set it

  /**
   * @brief multidimensional array containing the description for pin D4-D7(if using 4 pins) or
   * D0-D7(if using 8 pins)
   */
  uint32_t parallelPinList[TOTAL_PARALLEL_PIN][PIN_DESCRIPTION_LEN];
} LcdConfig;

/**
 * @brief The main class for the LcdDriver, all interactions with the 1602 lcd controller will be
 * through this class
 *
 */
class LcdDriver {
 private:
  /**
   * @brief How many bits that each pin has to send, if using 8 then each only has to send 1, but if
   * 4 then each has to do 2
   */
  uint32_t _totalBitPerPin;

  /**
   * @brief The LcdDriver copy of the user config received at constructor
   */
  LcdConfig _lcdConfig;

  /**
   * @brief Instance of general timer used for all timing purposes
   */
  GeneralTimer _generalTimer;

  /**
   * @brief used for reading the data from the controller RAM/program memory
   * The method will follow procedures outlined in the datasheet to intiate and read data from the
   * controller, if using 4 pins it will take two transfers to deliver 8 bits instead of 1 if
   * using 8 pins
   * @param isDataReg true if reading from the data memory(like RAM), false if reading from lcd
   * controller register(where it has things like the busy status)
   * @param readDataBuf buffer for storing the data, array of byte
   * @param totalReadData how many bytes to read
   */
  void parallelDataRead(const bool &isDataReg, uint8_t *readDataBuf, const uint32_t &totalReadData);

  /**
   * @brief used for sending data to the data pins(D0-D7) connected to the LCD
   * The bits are shifted to each pin and then a proper setup and hold time is followed to make sure
   * that the controller receives the data, the method will follow the correct procedure to
   * intitiate connection with the controller
   * @param dataList array of data to be sent
   * @param dataLen how many byte to send
   * @param isDataReg is the destination data memory(like RAM) or lcd controller memory
   */
  void parallelDataWrite(const uint8_t *dataList, const uint32_t &dataLen, const bool &isDataReg);

  /**
   * @brief used for writing a single byte to the lcd controller
   *
   * @param data the byte to be written
   * @param isDataReg is the destination data memory(like RAM) or lcd controller memory
   */
  void parallelDataWriteSingle(const uint8_t &data, const bool &isDataReg);

  /**
   * @brief switch all the data pins(like D0-D7) to input mode or output mode
   * Used for quickly switching between receiving and sending data
   * @param isInput if true then all data pin becomes input else become output
   */
  void parallelModeSwitch(const bool &isInput);

  /**
   * @brief switch a single pin to input/output
   * @param pinDesc array describing the pin
   * @param isInput if true, set pin to input, otherwise, set pin to false
   */
  void pinModeSwitch(const uint32_t pinDesc[PIN_DESCRIPTION_LEN], const bool &isInput);

  /**
   * @brief switch on/off a pin
   * This is the lowest level function, used for toggling control pin for bitbanging during
   * communcication
   * @param pinDesc array describing the pin
   * @param output if true, set pin to high, otherwise, set to low
   */
  void pinWrite(const uint32_t pinDesc[PIN_DESCRIPTION_LEN], const bool &output);

  /**
   * @brief Read whehther a pin is high or low
   * @param pinDesc array describing the pin
   * @return true pin is high
   * @return false pin is low
   */
  bool pinRead(const uint32_t pinDesc[PIN_DESCRIPTION_LEN]);

  /**
   * @brief configure the pins on things like drive strength(affect rise/fall time of signal),
   * push/pull mode
   * @param pinDesc array describing the pin
   */
  void pinPadConfig(const uint32_t pinDesc[PIN_DESCRIPTION_LEN]);

  /**
   * @brief check whether the array describing a pin is valid, will raise assert if not
   * @param pinDesc array describing the pin
   */
  void pinDescCheck(uint32_t pinDesc[PIN_DESCRIPTION_LEN]);

  /**
   * @brief Utility function used to generate signal to start the communication with the lcd
   * controller
   * The method will put the RS, RW line in the correct mode(depending on other params passed to
   * this function), pull high enable pin and then wait for the signal to become stable then exit
   * @param isDataReg true if the target of the communication is the data section of the lcd
   * controller(like DDRAM)
   * @param isReadMode  if true then the transaction is a read one
   */
  void comSetup(const bool &isDataReg, const bool &isReadMode);

  /**
   * @brief stop the communcation by waiting for the current data to be finished transacting and
   * then deassert the enable line
   */
  void comStop(void);

  /**
   * @brief temporarily deassert the enable line, wait for the current data to be finished
   * transacting and then asser the enable line again to continue communication
   * @param isReadMode is this transaction a read one
   */
  void comMaintain(const bool &isReadMode);

  /**
   * @brief select data(like RAM) or program register be switching the RS line
   * @param isDataReg if true then the RS line is pulled high to indicate that transaction will
   * target RAM region, otherwise target the lcd controller program memory
   */
  void registerSelect(const bool &isDataReg);

  /**
   * @brief used for writing configs to controller, uses other functions like
   * functionSetCommandCreate to get the command then push it to the lcd controller
   */
  void configWrite(void);

  /**
   * @brief turn on or off communication by switching the EN pin
   * @param iscomEnabled true then EN pin pulled high, pulled low otherwise
   */
  void comSwitch(const bool &iscomEnabled);

  /**
   * @brief switch to read/write mode by switching RW pin
   * @param isReadMode if true then RW is pulled high to indicate the next transaction to be a read
   * one
   */
  void comModeSwitch(const bool &isReadMode);

  /**
   * @brief Create a command of entry mode category
   *
   * @param cursorRightDir if true, set the direction of movement to the right
   * @param displayShiftEnabled if true, allow the display to be shifted when necessary(check the
   * manual for this)
   * @return uint8_t an entry command that can be sent to the lcd controller
   */
  uint8_t entryModeCommandCreate(const bool &cursorRightDir, const bool &displayShiftEnabled);

  /**
   * @brief Create a command of display category
   *
   * @param displayOn if true, turn on display, otherwise off
   * @param cursorOn if true, turn on cursor, otherwise off
   * @param isCursorBlink if true, blink cursor, otherwise no blink
   * @return uint8_t a display command that can be sent to the lcd controller
   */
  uint8_t displayCommandCreate(const bool &displayOn,
                               const bool &cursorOn,
                               const bool &isCursorBlink);

  /**
   * @brief Create a command of function set category
   *
   * @param is8BitDataLen if true, then send data over 8 pins, otherwise, send data using 4
   * pins(D4-D7)
   * @param is2Lines if true, display 2 lines on lcd, display 1 otherwise
   * @param is5x10Font if true, use 5x10 font, use 5x8 font otherwise
   * @return uint8_t a function set command that can be sent to the lcd controller
   */
  uint8_t functionSetCommandCreate(const bool &is8BitDataLen,
                                   const bool &is2Lines,
                                   const bool &is5x10Font);

  /**
   * @brief create a command to shift the cursor and maybe the display with it
   *
   * @param isShiftDisplay if true allow display to shift if required
   * @param isRight if true, set the movement direction to right
   * @return uint8_t a command that can be sent to the lcd controller to control cursor movement
   */
  uint8_t cursorDisplayShiftCommandCreate(const bool &isShiftDisplay, const bool &isRight);

  /**
   * @brief Read lcd controller memory and see if the lcd is busy with an operation
   * @return true lcd is doing an operation
   * @return false lcd is idle
   */
  bool lcdIsBusy(void);
  /**
   * @brief Read lcd controller memory to retrieve the 7 bits address counter
   * This command can be helpful during debug to probe what memories are being read
   * @return uint8_t the 7 bits address counter
   */
  uint8_t addrCounterGet(void);

  /**
   * @brief Change the address counter on the lcd controller
   * This command is used a lot since it changes the address counter to be ready for the read/write
   * operation
   * @param addr address to change to
   * @param isDataRam if true then the address to change to is part of the dataRAM, otherwise part
   * of CGRAM
   */
  void addrCounterChange(const uint8_t &addr, const bool &isDataRam);

  /**
   * @brief Used to write only 4 bits using D4-D7
   * This command is used mainly during the beginning of the communication where all data is sent
   * using 4 pins only once instead of 4 pins twice like the rest of the communication
   * @param dataToWrite data to be written, most likely a command to setup communication
   * @param stopAfterWrite if true terminate connection after write, otherwise maintain the
   * connection with the lcd controller
   */
  void dataWrite4Bit(const uint32_t &dataToWrite, const bool &stopAfterWrite);

  /**
   * @brief Read the lcd controller data to get the busy status and current address counter
   * @return uint8_t the busy bit(bit 7) and the address counter(bit 0-6)
   */
  uint8_t instructionDataRead(void);

  /**
   * @brief Write to the RAM of the lcd controller, can be data ram or character generator RAM
   *
   * @param data arrays of data to be written
   * @param dataLen len of the array of data
   * @param isTextMode true if in text mode, text mode is helpful for writing to data RAM to be
   * displayed, text mode will interpret special character like \n or $
   */
  void ramDataWrite(const uint8_t *data, const uint32_t dataLen, const bool &isTextMode);

  /**
   * @brief Used for reading the RAM of the lcd controller, it can either be the data ram storing
   * data to be displayed or the character generator ram, storing custom pattern
   * The reading operation may result in the cursor moving
   * @param returnData the buffer to store received data
   * @param totalDataRead how many bytes to read from the RAM
   * @param startingRamAddr starting address of the RAM, depending on the type of memory selected
   * the method will append the correct bits to indicate the correct memory region, for example, for
   * DDRAM, bit 7 is set
   * @param isDataRam true if reading from data RAM(DDRAM), false if from character generator
   * RAM(CGRAM)
   */
  void ramDataRead(uint8_t *       returnData,
                   const uint32_t &totalDataRead,
                   const uint8_t & startingRamAddr,
                   const bool &    isDataRam);

 public:
  /**
   * @brief Construct a new Lcd Driver object, doesn't initiate any hardware, just compute data to
   * be ready for future operations
   * @param lcdConfig struct storing settings for lcd driver such as pin allocations
   */
  LcdDriver(const LcdConfig &lcdConfig);

  /**
   * @brief Initialize the lcd driver, by turning on all gpio clocks and set the gpio mode to be
   * ready to drive the lcd
   */
  void init(void);

  /**
   * @brief Start communicating with the lcd and write settings to the lcd controller, this might
   * take more than other operations since it will wait the recommended amount of time in the manual
   * for the lcd to warm up
   */
  void enable(void);

  /**
   * @brief Erase the display and add new text to it starting at position (0,0), this method will be
   * the one used the most as it offers the most straightforward interface to writing to the LCD
   * @param dataToWrite character array reprenting string to print, limited at 32
   */
  void displayWrite(const char *dataToWrite);

  /**
   * @brief Append text to existing text onscreen, this method will also be used the most if there
   * is no need to modify the deeper level API
   * Note that during other operation, the cursor might
   * have been moved, if that happens, it's best to use the cursorPositionChange to get back to the
   * desired position and continue printing text
   * @param dataToAppend character array reprenting string to print, limited at 32
   */
  void displayAppend(const char *dataToAppend);

  /**
   * @brief Add new custom character pattern
   * The new pattern will be stored in the custom generator RAM of the lcd controller, the 1602 can
   * store 8 5x8 or 4 5x10 pattern
   * @param charPattern array storing the byte patterns, generate them using the link in the README
   * @param customCharSlot what slot to store the new pattern at, there should be 8 slot if 5x8 font
   * is used
   */
  void newCustomCharAdd(const uint8_t   charPattern[CUSTOM_CHAR_PATTERN_LEN],
                        const uint32_t &customCharSlot);

  /**
   * @brief Change lcd settings like on/off display, cursor, or blinking mode
   *  This method writes to the lcd controller register to set the settings
   * @param displayOn display on if true, off otherwise
   * @param cursorOn cursor on if true, off otherwise
   * @param cursorBlinkOn cursor blinking on if true, off otherwise
   */
  void lcdSettingSwitch(const bool &displayOn, const bool &cursorOn, const bool &cursorBlinkOn);

  /**
   * @brief reset the LCD and erase all RAM, also reset cursor to (0,0)
   */
  void lcdReset(void);
  /**
   * @brief Change cursor position on an x-y scale
   * This method also changes the data ram position with the cursor so future texts will be printed
   * at the cursor
   * @param cursorX x coordinate to set, max at 15 for 16x2 lcd
   * @param cursorY y coordinate to set, max at 1 for 16x2 lcd
   */
  void cursorPositionChange(const uint8_t &cursorX, const uint8_t &cursorY);

  /**
   * @brief Turn on or off the back light LED
   * The TivaC itself probably doesn't have the current to turn on or off the backLight alone so
   * probably use a relay or a transistor
   *
   * @param isBackLedOn turn on LED if true, off otherwise
   */
  void backLedSwitch(const bool &isBackLedOn);
};
}  // namespace lcddriver
#endif