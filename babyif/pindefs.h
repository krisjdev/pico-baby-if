#pragma once

// Pico-Baby pin defintion file


// the sender controls ptp a
// the reader controls ptp b
#define GPIO_OUT_PTP_A_PULSE 18     // TX data to baby
#define GPIO_OUT_PTP_B_PULSE 19     // RX data from baby
#define GPIO_OUT_CLOCK 21
#define GPIO_IN_RW_INTENT 22
#define GPIO_OUT_RESET_N 26
#define GPIO_IN_STOP_LAMP 28


// these pins must be seperated by 8
// they cannot overlap
// https://pico.pinout.xyz/
#define GPIO_IN_DATA_BASE_PIN 2     // GP2 to GP9 will be used for data input, paired with PTP_B
#define GPIO_OUT_DATA_BASE_PIN 10   // GP10 to GP17 will be used for data output, paired with PTP_A