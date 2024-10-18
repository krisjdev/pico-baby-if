# pico-baby-if

A Pico module designed to interface with an FPGA implementation of the Manchester Baby. 

## FPGA-side
The FPGA needs to run RTL that can be found at https://github.com/diy-ic/tt-manchester-baby/.

## Pico-side

The Pico (or microcontroller) acts as the RAM module for the Manchester Baby, allowing it to read and write values when given the appropriate signals. 

### Pinout
Pin assignments can be found in ``babyif/pindefs.h``, and can be edited to better suit your application. Please note that the data pins must be contiguous and cannot overlap.
