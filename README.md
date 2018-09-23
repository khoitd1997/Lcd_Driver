# Lcd Driver

Driver using the TivaC Dev board for the 1602A LCD controller. This controller uses a parallel bus that can do 4-bit or 8-bit, there was no standard protocol to talk with it aside from the things noted in the datasheet so it's just pretty big banging until it works

Features:

- Write data to display, support newline, standard alphanumeric character
- Flexible cursor movement
- Display supports autoscroll
- Reading data from both data ram and character generator ram as well as lcd status
- Check display status
- Adding new custom characters to character generator ROM, the `{custom_pattern_num} format is used when displaying custom char, example below
- Control backLED(provided that you have a relay hooked up to it)

## Notes about Usability

The project was written mainly for my personal usage so there was more hardcoding that I would like for a driver but I have also attempted to make it flexible where possible, you are welcome to add more abstractions to make it look nicer

## Documentations

There is doxygen docs of the LcdDriver class here:

[]()

Check TivaWare documents(included with the full TivaWare) for other stuffs

## References and Helpful Resources

[HD44780 datasheet, similar but not the same](https://www.sparkfun.com/datasheets/LCD/HD44780.pdf)

[The 1602 datasheet](https://www.openhacks.com/uploadsproductos/eone-1602a1.pdf)

[Custom character generator](https://omerk.github.io/lcdchargen/)(Necessary if you want to create your own character)

## Building and the project

This is a code composer studio(ccs) project so simply cloning and importing it and then set the SW_ROOT inside the project properties to the TivaDep directory would be enough, for example: my SW_ROOT variable is /path/to/this_repo/Tivaware_Dep, for version 8.2.0.00007, the project builds successfully

Building the project using something other than ccs is not guaranteed because TivaWare doesn't play nicely with other IDE from my experience(unless you go through and fix every single include errors)

If you look at the .gitignore of the project, you would see only a fraction of ignore files that usually go into C/C++ and Eclipse project, it was intentional since ccs won't build if I ignore other stuffs, so to play it safe, I only ignore files that are guaranteed to be unimportant

Another note is that if you haven't, you should increase the stacksize allowed by ccs to make sure no weird errors happen

## Project structure

- **Tivaware_Dep/**: This is the necessary stuffs pulled from TivaWare, defining pins used for the LCD needs macro from this folder
- **tiva_utils/**: general utils stuffs like bit manipulations macros
- **general_timer/**: this is a utility class used for timing various things, it uses wide timer 0 of the TivaC for timing purposes, the accuracy is probably in the range of 500 uS
- **src/**: this is where the lcd driver code resides
    - lcd_driver.cpp: the main functions of the LcdDriver class is defined here
    - lcd_driver.hpp: header file declaring the LcdDriver class
    - lcd_utils.cpp: defining utils functions of the LcdDriver class
    - main.cpp: serve as an example for how to use the LcdDriver class

## Adding and Using Custom Character

While most of the projects are documented in doxygen, adding custom character is a little special since it requires external help, you should go to https://omerk.github.io/lcdchargen/, create your pattern then grab the array from there

Then in your code:

```cpp
// creating and preparing the driver
auto lcdDriver = LcdDriver(lcdConfig);
lcdDriver.init();
lcdDriver.enable();

// paste the array you got from the website above here
uint8_t charPattern2[] = {0b10000, 0b01000, 0b01011, 0b01110, 0b01010, 0b00010, 0b00010, 0b00010};
lcdDriver.newCustomCharAdd(charPattern2, 2); // uses slot 2 for the pattern, you are set to use after this

// using the custom pattern
lcdDriver.displayWrite("Pattern: `2"); // `2 will be replaced with the pattern you just added on the LCD

// the pattern should be printed on the lcd after this
```