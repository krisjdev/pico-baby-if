#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include <malloc.h>
#include <stdlib.h>

#include "babyif.h"
#include "pindefs.h"
#include "program.c"

int main()
{
start:
    stdio_init_all();

    babyif_init_gpio();
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, true);

    // inital state
    uint32_t total_ticks = 0;
    read_packet_t packet;
    uint32_t data_tx = program[0];
    gpio_put(PICO_DEFAULT_LED_PIN, false);

    // set reset lines and clock twice
    gpio_put(GPIO_OUT_RESET_N, false);
    gpio_put(GPIO_OUT_PTP_RESET_N, false);
    babyif_pulse_clock(2);

    // exit reset state
    gpio_put(GPIO_OUT_RESET_N, true);
    gpio_put(GPIO_OUT_PTP_RESET_N, true);

    // present data to pins (exit reset state)
    // this has to be done to avoid reading a 0 on the first go
    _pulse_control_line(READ__PTP_B);

    printf("\e[1;1H\e[2J"); 
    printf("[test] ready\n");

    // fetch data, first two entries should match magic values
    packet = babyif_read_data();

    if (packet.address != 0xDEADBEEF) {
        printf("[test - read] first packet magic value was incorrect - %#10x != 0xDEADBEEF\n", packet.address);
    } else {
        printf("[test - read] first packet magic value matched\n");
    }

    if (packet.data != 0xCAFEB0BA) {
        printf("[test - read] second packet magic value was incorrect - %#10x != 0xCAFEB0BA\n", packet.data);
    } else {
        printf("[test - read] second packet magic value matched\n");
    }



    printf("\n");



    printf("[test - write] current data is: %#10x\n", packet.pc);
    uint32_t magic_value = 0xBAADF00D;
    printf("[test - write] writing magic value (%#10x)\n", magic_value);
    babyif_write_data(magic_value);
    _pulse_control_line(WRITE__PTP_A);

    packet = babyif_read_data();

    if (packet.pc != magic_value) {
        printf("[test - write] magic value was not written properly - %#10x != %#10x\n", packet.pc, magic_value);
    } else {
        printf("[test - write] magic value was written properly and matches");
    }

    sleep_ms(1LL << 32 - 1);
    return -1;
}