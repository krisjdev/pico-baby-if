#include <babyif.h>

#include <clocks.pio.h>
#include <data_io.pio.h>

#include "pindefs.h"

babyif* babyif_constructor() {

    gpio_init(GPIO_OUT_RESET_N);
    gpio_init(GPIO_OUT_ALLOW_EXEC);
    gpio_init(GPIO_IN_RW_INTENT);
    gpio_init(GPIO_IN_STOP_LAMP);

    gpio_set_dir(GPIO_OUT_RESET_N, GPIO_OUT);
    gpio_set_dir(GPIO_OUT_ALLOW_EXEC, GPIO_OUT);
    gpio_set_dir(GPIO_IN_RW_INTENT, GPIO_IN);
    gpio_set_dir(GPIO_IN_STOP_LAMP, GPIO_IN);

    gpio_put(GPIO_OUT_RESET_N, true);

    // see https://github.com/krisjdev/pico-baby-if/issues/1
    // set data pins to input
    for (int i = 0; i < 8; i++){
        gpio_init(GPIO_IN_DATA_BASE_PIN+i);
        gpio_set_dir(GPIO_IN_DATA_BASE_PIN+i, GPIO_IN);
    }

    // set pulse pin to output
    gpio_init(GPIO_OUT_PTP_B_PULSE);
    gpio_set_dir(GPIO_OUT_PTP_B_PULSE, GPIO_OUT);

    return (babyif*) malloc(sizeof(babyif));
}

void babyif_destructor(babyif* babyif) {
    free(babyif);
}

void babyif_init(babyif* babyif, PIO pio_clk, PIO pio_data, uint clk_freq) {
    babyif->pio_clk = pio_clk;
    babyif->pio_data = pio_data;
    babyif->clk_freq = clk_freq;
}

void babyif_init_pio_clock(babyif* babyif) {
    uint prog_offset = pio_add_program(babyif->pio_clk, &clocks_program);

    #ifdef DEBUG
        printf("[babyif_init_pio_clock] loaded clock program at %d\n", prog_offset);
    #endif

    clocks_program_init(babyif->pio_clk, 0, prog_offset, GPIO_OUT_GENERATED_CLOCK_PIN, GPIO_IN_BABY_CLOCK_PIN);

    // calculation taken from raspbery pi's blinking pio example
    pio_sm_put(babyif->pio_clk, 0, (125000000 / (2 * babyif->clk_freq)) - 3);
}

void babyif_enable_pio_clock(babyif* babyif) {
    pio_sm_set_enabled(babyif->pio_clk, 0, true);
}

void babyif_disable_pio_clock(babyif* babyif) {
    pio_sm_set_enabled(babyif->pio_clk, 0, false);
}

void babyif_restart_pio_clock(babyif* babyif) {
    pio_sm_restart(babyif->pio_clk, 0);
}

bool babyif_get_generated_clock_irq(babyif* babyif) {
    return pio_interrupt_get(babyif->pio_clk, GENERATED_CLOCK_IRQ_FLAG);
}

void babyif_clear_generated_clock_irq(babyif* babyif) {
    pio_interrupt_clear(babyif->pio_clk, GENERATED_CLOCK_IRQ_FLAG);
}

bool babyif_get_baby_clock_irq(babyif* babyif) {
    return pio_interrupt_get(babyif->pio_clk, BABY_CLOCK_IRQ_FLAG);
}

void babyif_clear_baby_clock_irq(babyif* babyif) {
    pio_interrupt_clear(babyif->pio_clk, BABY_CLOCK_IRQ_FLAG);
}



void babyif_init_pio_data(babyif* babyif) {
    uint prog_offset = pio_add_program(babyif->pio_data, &data_io_program);

    #ifdef DEBUG
        printf("[babyif_init_pio_data] loaded data_io program at %d\n", prog_offset);
    #endif

    #ifndef DO_NOT_USE_PIO_DATA_RECEIVER_SM
        // init reciever state machine
        data_io_program_init(babyif->pio_data, RECEIVER_STATE_MACHINE, prog_offset, GPIO_IN_DATA_BASE_PIN, GPIO_OUT_PTP_B_PULSE, false);
        pio_sm_put(babyif->pio_data, RECEIVER_STATE_MACHINE, CONFIG_READ);
    #endif

    // init sender state machine
    data_io_program_init(babyif->pio_data, SENDER_STATE_MACHINE, prog_offset, GPIO_OUT_DATA_BASE_PIN, GPIO_OUT_PTP_A_PULSE, true);
    pio_sm_put(babyif->pio_data, SENDER_STATE_MACHINE, CONFIG_WRITE);  
    
    #ifdef DEBUG
        printf("[babyif_init_pio_data] receiver sm txf: %d\n", babyif->pio_data->txf[RECEIVER_STATE_MACHINE]);
        printf("[babyif_init_pio_data] sender sm txf: %d\n", babyif->pio_data->txf[SENDER_STATE_MACHINE]);
    #endif
}

void babyif_enable_pio_data(babyif* babyif) {
    pio_sm_set_enabled(babyif->pio_data, 0, true);
    pio_sm_set_enabled(babyif->pio_data, 1, true);
}

void babyif_disable_pio_data(babyif* babyif) {
    pio_sm_set_enabled(babyif->pio_data, 0, false);
    pio_sm_set_enabled(babyif->pio_data, 1, false);
}

void babyif_restart_pio_data(babyif* babyif) {
    pio_sm_restart(babyif->pio_data, 0);
    pio_sm_restart(babyif->pio_data, 1);
}

bool babyif_get_sent_data_irq(babyif* babyif) {
    return pio_interrupt_get(babyif->pio_data, SENT_DATA_IRQ_FLAG);
}

void babyif_clear_sent_data_irq(babyif* babyif) {
    pio_interrupt_clear(babyif->pio_data, SENT_DATA_IRQ_FLAG);
}

bool babyif_get_received_data_irq(babyif* babyif) {
    return pio_interrupt_get(babyif->pio_data, RECEIVED_DATA_IRQ_FLAG);
}

void babyif_clear_received_data_irq(babyif* babyif) {
    pio_interrupt_get(babyif->pio_data, RECEIVED_DATA_IRQ_FLAG);
}

void babyif_put_data_word(babyif* babyif, uint32_t word) {

    #ifdef DEBUG
        printf("[babyif_put_data_word] writing to fifo: %#10x\n", word);
    #endif

    pio_sm_put(babyif->pio_data, SENDER_STATE_MACHINE, word);
    pio_interrupt_clear(babyif->pio_data, SENT_DATA_IRQ_FLAG);
}

#ifndef DO_NOT_USE_PIO_DATA_RECEIVER_SM
uint32_t babyif_get_data_word(babyif* babyif) {
    uint32_t data = pio_sm_get_blocking(babyif->pio_data, RECEIVER_STATE_MACHINE);
    #ifdef DEBUG
        printf("[babyif_get_data_word] got data: %#10x\n", data);
    #endif

    return data;
}
#endif


uint32_t babyif_get_data_word_gpio(babyif* babyif) {

    uint32_t temp = 0;
    gpio_put(GPIO_OUT_PTP_B_PULSE, false);

    for (int i = 0; i < 4; i++){
        for (int j = 0; j < 8; j++) {
            // printf("%d ", gpio_get(GPIO_IN_DATA_BASE_PIN+j));
            // shift *then* add, otherwise one bit will be missing
            temp = temp << 1;
            temp += gpio_get(GPIO_IN_DATA_BASE_PIN+j);
        }
        // printf("\t%#10x\n", temp);
        

        gpio_put(GPIO_OUT_PTP_B_PULSE, true);
        gpio_put(GPIO_OUT_PTP_B_PULSE, false);
    }

    #ifdef DEBUG
        printf("[babyif_get_data_word_gpio] got data: %#10x\n", temp);
    #endif

    return temp;
}

bool inline babyif_get_stop_lamp() {
    return gpio_get(GPIO_IN_STOP_LAMP);
}

bool inline babyif_get_ram_read_write_intent() {
    return gpio_get(GPIO_IN_RW_INTENT);
}

void inline babyif_set_exec() {
    gpio_put(GPIO_OUT_ALLOW_EXEC, 1);
}

void inline babyif_clear_exec() {
    gpio_put(GPIO_OUT_ALLOW_EXEC, 0);
}
bool inline babyif_get_exec() {
    return gpio_get(GPIO_OUT_ALLOW_EXEC);
}

void inline babyif_set_reset() {
    gpio_put(GPIO_OUT_RESET_N, 0);
}

void inline babyif_clear_reset() {
    gpio_put(GPIO_OUT_RESET_N, 1);
}
bool inline babyif_get_reset() {
    return gpio_get(GPIO_OUT_RESET_N);
}