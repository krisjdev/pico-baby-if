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

void draw_crt() {
    // potential characters to use for rendering: ■ █ ● • · ▪

    for (int i = 0; i < PROGRAM_SIZE; i++) {
        // printf("%#10x | ", program[i]);
        printf("%#4x |", i);

        // TODO: replace with byte hashmap instead of having to compute it for each bit
        // or replace with some other performant variant
        for (int j = 0; j < 32; j++) {
            if (program[i] & (0x1 << j)) {
                // print 1
                printf("■ ");
            } else {
                // print 0
                printf("· ");
            }
        }
        printf("| %#10x\n", program[i]);
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
    uint8_t tick = 0;
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
    
    // https://stackoverflow.com/questions/66927511/what-does-e-do-what-does-e11h-e2j-do
    // clear terminal
    printf("\e[1;1H\e[2J"); 

    while(true) {
        printf("\e[1;1H"); // move cursor to top of terminal
        draw_crt();

        gpio_put(PICO_DEFAULT_LED_PIN, true);

        babyif_write_data(data_tx);
        babyif_pulse_clock(1);

        int rw_intent = gpio_get(GPIO_IN_RW_INTENT);

        if (gpio_get(GPIO_IN_STOP_LAMP)) {
            printf("\n\n\n\n"); // spam newlines to skip over other debug info
            printf("[main] stop lamp is high\n");
            break;
        }

        // show data to baby
        _pulse_control_line(WRITE__PTP_A);

        packet = babyif_read_data();


        // TODO: might be better to provide helper functions to access memory?
        // if accessing out of bounds memory
        if (packet.address > PROGRAM_SIZE) {
            printf("[main] fatal: attempted to access out of bounds memory: \n\tpacket.address=%#10x\n\tPROGRAM_SIZE=%i\n", packet.address, PROGRAM_SIZE);
            printf("[main] restarting soon...\n");
            sleep_ms(200);
            goto start;
        }

        printf("\ntick: %d\n", tick);
        printf("PC: %#10x, IR: %#10x, ACC: %#10x\n", packet.pc, packet.ir, packet.acc);
        tick = update_tick(tick);


        if (rw_intent == BABY_READ_INTENT) {
            data_tx = program[packet.address];
            printf("read : program[%#10x] = %#10x\n", packet.address, data_tx);
        } else if (rw_intent == BABY_WRITE_INTENT) {
            program[packet.address] = packet.data;
            printf("write: program[%#10x] = %#10x\n", packet.address, packet.data);
        } else {
            printf("\n[main] unable to determine read/write intent\n");
            return -1;
        }

        gpio_put(PICO_DEFAULT_LED_PIN, false);
    }

    printf("[main] broken out of program loop\n");
    dump_memory_contents();
    return -1;
}