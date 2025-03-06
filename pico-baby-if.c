#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include <malloc.h>
#include <stdlib.h>

#include "babyif.h"
#include "pindefs.h"
#include "program.c"

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

void get_cursor_position(int* positions) {

    printf("\e[6n"); // get current cursor position
    // \e[000;000R - https://gist.github.com/fnky/458719343aabd01cfb17a3a4f7296797#cursor-controls
    // assume that the x and y pos fits within 3 digits for each axis, with the last element being a null terminator
    char xpos_str[4] = {'\0'};
    char ypos_str[4] = {'\0'};
    bool write_xpos_str = true;
    int pos_str_index = 0;

    while (true) {
        char read_byte = getchar();
        
        if (read_byte == '\e' || read_byte == '[') continue;        
        if (read_byte == 'R') break;
        
        if (read_byte == ';') {
            write_xpos_str = false;
            pos_str_index = 0;
            continue;
        }

        if (write_xpos_str) {
            xpos_str[pos_str_index] = read_byte;
        } else {
            ypos_str[pos_str_index] = read_byte;
        }

        pos_str_index++;
    }

    positions[0] = atoi(xpos_str);
    positions[1] = atoi(ypos_str);
}


// potential characters to use for rendering: ■ █ ● • · ▪
const char *glyphs[16] = {
    "· · · ·", "■ · · ·", "· ■ · ·", "■ ■ · ·",     // 0000, 0001, 0010, 0011
    "· · ■ ·", "■ · ■ ·", "· ■ ■ ·", "■ ■ ■ ·",     // 0100, 0101, 0110, 0111
    "· · · ■", "■ · · ■", "· ■ · ■", "■ ■ · ■",     // 1000, 1001, 1010, 1011
    "· · ■ ■", "■ · ■ ■", "· ■ ■ ■", "■ ■ ■ ■",     // 1100, 1101, 1110, 1111
};


void draw_crt() {

    for (int i = 0; i < PROGRAM_SIZE; i++) {
        // printf("%#10x | ", program[i]);
        printf("%#4x | ", i);

        // https://gist.github.com/fnky/458719343aabd01cfb17a3a4f7296797
        printf("\e[32m"); // set colour to green

        for (int j = 0; j < 8; j++) {
            // shift mask for 4 bits then reshift whole number back into 8 bits max
            // otherwise some values would overflow and would not access the glyphs properly
            int glyph_index = (program[i] & (0xF << (j*4))) >> j * 4;
            printf(glyphs[glyph_index]);
            printf(" ");
        }

        printf("\e[0m"); // reset colour
        printf("| %#10x\n", program[i]);
    }
    
}

void update_crt_line(uint32_t pos, uint32_t value) {

    int cursor_xy[2] = {0};
    get_cursor_position(cursor_xy);

    // move cursor to start of crt line for given address
    printf("\e[%i;8H", pos);

    printf("\e[32m");

    // TODO: move this into a seperate function?
    // same block as the one in draw_crt()
    for (int j = 0; j < 8; j++) {
        int glyph_index = (value & (0xF << (j*4))) >> j * 4;
        printf(glyphs[glyph_index]);
        printf(" ");
    }

    printf("\e[0m"); // reset colour
    printf("| %#10x\n", value);

    printf("\e[%i;%iH", cursor_xy[0], cursor_xy[1]);
}

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

    printf("[main] ready\n");
    
    // https://stackoverflow.com/questions/66927511/what-does-e-do-what-does-e11h-e2j-do
    // clear terminal
    printf("\e[1;1H\e[2J"); 
    draw_crt();

    while(true) {
        printf("\e[33;1H"); // move cursor to bottom of crt
        sleep_ms(100);

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

        uint8_t tick = total_ticks % 8;
        printf("\ntick: %d (total: %d)\n", tick, total_ticks);
        printf("PC: %#10x, IR: %#10x, ACC: %#10x\n", packet.pc, packet.ir, packet.acc);
        total_ticks++;


        if (rw_intent == BABY_READ_INTENT) {
            data_tx = program[packet.address];
            printf("read : program[%#10x] = %#10x\n", packet.address, data_tx);
        } else if (rw_intent == BABY_WRITE_INTENT) {
            program[packet.address] = packet.data;
            update_crt_line(packet.address, packet.data);
            printf("write: program[%#10x] = %#10x\n", packet.address, packet.data);
        }

        gpio_put(PICO_DEFAULT_LED_PIN, false);
    }

    printf("[main] broken out of program loop\n");
    dump_memory_contents();
    sleep_ms(1LL << 32 - 1);
    return -1;
}