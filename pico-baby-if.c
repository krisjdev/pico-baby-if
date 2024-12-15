#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include <malloc.h>

#include "babyif.h"
#include "pindefs.h"
#include "program.c"

int inline update_tick(int current_tick) {
    return (current_tick + 1) % 8;
}

void dump_memory_contents() {
    printf("[dump_memory_contents] memory dump:\n");

        for (int i = 0; i < PROGRAM_SIZE; i++) {
            if (i % 4 == 0) {
                printf("%10x|\t%#10x ", i, program[i]);
            } else if (i % 4 == 3) {
                printf("%#10x\t|\n", program[i]);
            } else {
                printf("%#10x ", program[i]); 
            }
    }
}

int main()
{
start:
    stdio_init_all();

    babyif_init_gpio();
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, true);

    // inital state
    uint8_t tick = 1;
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

    printf("[main] ready\n");

    while(true) {
        gpio_put(PICO_DEFAULT_LED_PIN, true);

        babyif_write_data(data_tx);
        babyif_pulse_clock(1);
        tick = update_tick(tick);

        int rw_intent = gpio_get(GPIO_IN_RW_INTENT);

        if (gpio_get(GPIO_IN_STOP_LAMP)) {
            printf("[main] stop lamp is high\n");
            break;
        }

        // show data to baby
        _pulse_control_line(WRITE__PTP_A);

        packet = babyif_read_data();

        // printf("[main] packet.addr = %#x, packet.data = %#x\n", packet.address, packet.data);

        // TODO: might be better to provide helper functions to access memory?
        // accessing out of bounds memory
        if (packet.address > PROGRAM_SIZE) {
            printf("[main] fatal: attempted to access out of bounds memory: \n\tpacket.address=%#10x\n\tPROGRAM_SIZE=%i\n", packet.address, PROGRAM_SIZE);
            sleep_ms(200);
            goto start;
        }
        if (rw_intent == BABY_READ_INTENT) {
            data_tx = program[packet.address];
            printf("[main] read: progam[%#x] = %#x\n", packet.address, data_tx);
        } else if (rw_intent == BABY_WRITE_INTENT) {
            program[packet.address] = packet.data;
            printf("[main] write: progam[%#x] = %#x\n", packet.address, packet.data);
        } else {
            printf("[main] unable to determine read/write intent\n");
            return -1;
        }

        gpio_put(PICO_DEFAULT_LED_PIN, false);
    }

    printf("[main] broken out of program loop\n");
    dump_memory_contents();
    return -1;
}