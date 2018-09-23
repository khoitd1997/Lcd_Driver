/**
 * @brief header file holding lcd controller specific information like its timing parameter,
 * commands
 *
 * @file lcd_include.hpp
 * @author Khoi Trinh
 * @date 2018-09-23
 */

#ifndef _LCD_INCLUDE_HPP
#define _LCD_INCLUDE_HPP

#define MAX_LCD_X 15  //!< Max cursor x coodirnate, limited by the horizontal length of the LCD
#define MAX_LCD_Y 1   //!< Max cursor y coordinate, limited by the vertical len of LCD

/* Timing Variable */

#define COM_TIME_SCALER \
  7000  //!< will be multiplied with timing parameters to prolong the duration, adjust as needed,
        //!< too low and there might be problem as the lcd controller may not catch up

// waiting phase time
#define LCD_WARM_UP_TIME_NANOSEC 49000000    //!< nanosec to wait for the LCD when it first wakes up
#define LCD_FIRST_INIT_TIME_NANOSEC 4500000  //!< time to wait after first lcd contact
#define LCD_SECOND_INIT_TIME_NANOSEC 150000  //!< time to wait after second lcd contact

// data cycle time
#define LCD_PULSE_WIDTH_NANOSEC 200 * COM_TIME_SCALER  //!< duration that the EN pin is stable high
#define LCD_MIN_CYCLE_TIME_NANOSEC \
  410 * COM_TIME_SCALER  //!< Min time in nanosec between two rising edge of EN pin

// setup and hold time
// for writing
#define LCD_DATA_SETUP_TIME_NANOSEC \
  45 * COM_TIME_SCALER  //!< time to hold data stable pre write transaction
#define LCD_DATA_HOLD_TIME_NANOSEC \
  15 * COM_TIME_SCALER  //!< time to hold data stable during transaction for write

// for address based
#define LCD_ADDR_SETUP_TIME_NANOSEC \
  35 * COM_TIME_SCALER  //!< time to hold the RS, R/W line stable b4 transaction to properly indcate
                        //!< target

#define LCD_ADDR_HOLD_TIME_NANOSEC \
  15 * COM_TIME_SCALER  //!< time to hold the RS, R/W line stable during transaction to properly
                        //!< indcate target

#define TIVA_MAX_RISE_TIME 13  //!< TIVA C max rise time for square wave if using 8mA drive strength
#define TIVA_MAX_FALSE_TIME \
  14  //!< TIVA C max fall time for square wave if using 8mA drive strength

/**
 * @brief wait time after a write during a transaction, combines many other timing section
 */
#define LCD_DATA_WRITE_WAIT_NANOSEC \
  TIVA_MAX_RISE_TIME + LCD_PULSE_WIDTH_NANOSEC - (LCD_DATA_SETUP_TIME_NANOSEC)

#define LCD_DATA_READ_DELAY_NANOSEC \
  800  //!< nominal amount of time that the data output by the lcd controller will be available

#define LCD_STARTUP_COMMAND 0b110000  //!< command to be written during the lcd wakeup
#define LCD_BEGIN_COMMAND \
  0b100000  //!< command to be written to initiate the first configuration transaction for the lcd
#define LCD_CLEAR_COMMAND 0b1         //!< command to clear the lcd ram and move cursor to (0,0)
#define LCD_RETURN_HOME_COMMAND 0b10  //!< command to set cursor back at (0,0) without clearing data
#define LCD_JUMP_LINE_COMMAND 0xc0    //!< command to jump to the beginning of the next line on lcd

#define LCD_BUSY_BIT 7  //!< the busy bit in the data returned from reading the lcd program data
#define LCD_ADDR_COUNTER_MASK \
  0x7f  //!< bit mask for address counter in data received from the lcd controller program memory

#define LCD_MEMUSED_PER_x8_CHAR 8  //!< how many bytes does one custom char pattern use

#endif