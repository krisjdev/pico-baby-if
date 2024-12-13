# pico-baby-if

A Pico module designed to interface with an FPGA implementation of the Manchester Baby. 

## FPGA-side
The FPGA needs to run RTL that can be found at https://github.com/diy-ic/tt-manchester-baby/.

## Pico-side

The Pico (or microcontroller) acts as the RAM module for the Manchester Baby, allowing it to read and write values when given the appropriate signals. 

### Pinout
Pin assignments can be found in ``babyif/pindefs.h``, and can be edited to better suit your application. Please note that the data pins must be contiguous and cannot overlap.

The data base pins correspond to the 0th bit of the 8-bit value, so for example if ``GPIO_IN_DATA_BASE_PIN = 2`` then it should be connected to ``ui_in[0]`` - GPIO pin 9 would be ``ui_in[7]``.