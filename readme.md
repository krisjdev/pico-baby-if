# pico-baby-if

A Pico module designed to interface with an FPGA implementation of the Manchester Baby. 

## FPGA-side
The FPGA needs to run HDL that can be found at https://github.com/diy-ic/tt07-manchester-baby/.


### Pinout

| Pin(s) | Direction | Usage/Notes |
| :- | :- | :- |
| ui_in[7:0] | Input | Parallel data input to PTP_A on Manchester Baby |
| uo_out[7:0] | Output | Parallel data output from PTP_B on Manchester Baby |
| uio_*[7] | Output | RAM read/write intent |
| uio_*[6] | Output | Stop Lamp |
| uio_*[5] | Output | Internal Manchester Baby clock output |
| uio_*[4] | Input | Allow execution |
| uio_*[3] | Input | Baby reset_n |
| uio_*[2] | Input | PTP reset_n |
| uio_*[1] | Input | PTP_B control |
| uio_*[0] | Input | PTP_A control |


## Pico-side
This module makes use of both of Pico's PIO units - one to control a clock signal going to the FPGA, and another to control the data I/O.

> [!NOTE]
> Currently the data input is not handled by PIO, see https://github.com/krisjdev/pico-baby-if/issues/1.

### Pinout
The pin assignments can be seen in ``babyif/pindefs.h``, and can be edited to better suit your application. The default values are also available below:

| Pin(s) | Direction | Usage/Notes |
| :- | :- | :- |
| GP 2-9 | Input | 8-bit data input -- only the base can be modified, these will always have to be 8 contiguous pins|
| GP 10-17 | Output | 8-bit data output -- only the base can be modified, these will always have to be 8 contiguous pins |
| GP 18 | Output | PTP_A control signal |
| GP 19 | Output | PTP_B control signal |
| GP 20 | Input | Clock signal from the Baby |
| GP 21 | Output | PIO generated clock signal |
| GP 22 | Input | RAM read/write intent signal |
| GP 26 | Output | Reset signal |
| GP 27 | Output | Allow Baby to execute |
| GP 28 | Input | Stop lamp from the Baby |