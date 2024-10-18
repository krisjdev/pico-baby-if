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
    stdio_init_all();

    babyif_init_gpio();
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, true);

    // inital state
    uint8_t tick = 0;
    read_packet packet;
    uint32_t data_tx = program[0];

    // set reset lines and clock twice
    gpio_put(GPIO_OUT_RESET_N, false);
    gpio_put(GPIO_OUT_PTP_RESET_N, false);
    babyif_pulse_clock(2);

    // exit reset state
    gpio_put(GPIO_OUT_RESET_N, true);
    gpio_put(GPIO_OUT_PTP_RESET_N, true);


    while(true) {
        gpio_put(PICO_DEFAULT_LED_PIN, true);
        printf("[main] (estimated) tick: %d\n", tick);

        babyif_write_data(data_tx);
        babyif_pulse_clock(1);
        tick = update_tick(tick);

        int rw_intent = gpio_get(GPIO_IN_RW_INTENT);

        if (gpio_get(GPIO_IN_STOP_LAMP)) {
            printf("[main] stop lamp is high\n");
            break;
        }

        _pulse_control_line(WRITE__PTP_A);

        packet = babyif_read_data();

        // TODO: might be better to provide helper functions to access memory?
        // accessing out of bounds memory
        if (packet.address > PROGRAM_SIZE) {
            printf("[main] fatal: manchester baby attempted to access out of bounds memory: packet.address=%#10x, PROGRAM_SIZE=%i", packet.address, PROGRAM_SIZE);
            return -1;
        }

        if (rw_intent == BABY_READ_INTENT) {
            data_tx = program[packet.address];
            printf("[main] read: progam[%#x] = %#x\n", packet.address, data_tx);
        } else if (rw_intent == BABY_WRITE_INTENT) {
            program[packet.address] = packet.data;
            printf("[main] write: progam[%#x] = %#x\n", packet.address, packet.data);
        }

        sleep_ms(800);
        gpio_put(PICO_DEFAULT_LED_PIN, false);
    }

    printf("[main] broken out of program loop\n");
    dump_memory_contents();
    sleep_ms(1LL << 32 - 1);
}