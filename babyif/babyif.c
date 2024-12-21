#include <stdio.h>

#include "babyif.h"
#include "pindefs.h"

#define CLOCK_PERIOD_MS 2

void babyif_init_gpio() {
    gpio_init(GPIO_OUT_PTP_A_PULSE);
    gpio_init(GPIO_OUT_PTP_B_PULSE);
    gpio_init(GPIO_OUT_PTP_RESET_N);
    gpio_init(GPIO_OUT_CLOCK);
    gpio_init(GPIO_IN_RW_INTENT);
    gpio_init(GPIO_OUT_RESET_N);
    gpio_init(GPIO_IN_STOP_LAMP);

    gpio_set_dir(GPIO_OUT_PTP_A_PULSE, GPIO_OUT);
    gpio_set_dir(GPIO_OUT_PTP_B_PULSE, GPIO_OUT);
    gpio_set_dir(GPIO_OUT_PTP_RESET_N, GPIO_OUT);
    gpio_set_dir(GPIO_OUT_CLOCK, GPIO_OUT);
    gpio_set_dir(GPIO_OUT_RESET_N, GPIO_OUT);

    gpio_set_dir(GPIO_IN_RW_INTENT, GPIO_IN);
    gpio_set_dir(GPIO_IN_STOP_LAMP, GPIO_IN);

    // drive outputs low immediately
    gpio_put(GPIO_OUT_PTP_A_PULSE, false);
    gpio_put(GPIO_OUT_PTP_B_PULSE, false);
    gpio_put(GPIO_OUT_PTP_RESET_N, false);
    gpio_put(GPIO_OUT_CLOCK, false);
    gpio_put(GPIO_OUT_RESET_N, false);

    for (int i = 0; i < 8; i++){
        gpio_init(GPIO_IN_DATA_BASE_PIN+i);
        gpio_init(GPIO_OUT_DATA_BASE_PIN+i);

        gpio_set_dir(GPIO_IN_DATA_BASE_PIN+i, GPIO_IN);
        gpio_set_dir(GPIO_OUT_DATA_BASE_PIN+i, GPIO_OUT);

        gpio_put(GPIO_OUT_DATA_BASE_PIN+i, false);
    }

    #ifdef BIF_DEBUG
        printf("[babyif_init_gpio] finished setting up gpio pins\n");
    #endif
}


void babyif_pulse_clock(uint32_t cycles) {
    
    if (cycles == 0) return;

    for (int i = 0; i < cycles; i++) {
        gpio_put(GPIO_OUT_CLOCK, true);
        sleep_ms(CLOCK_PERIOD_MS/2);
        gpio_put(GPIO_OUT_CLOCK, false);
        sleep_ms(CLOCK_PERIOD_MS/2);

        #ifdef BIF_DEBUG
            printf("[babyif_pulse_clock] finished pulsing clock for %ims\n", CLOCK_PERIOD_MS);
        #endif
    }
}

void _pulse_control_line(control_line_t line) {

    gpio_put(line, true);
    sleep_ms(2);
    gpio_put(line, false);

    #ifdef BIF_DEBUG
        printf("[_pulse_control_line] pulsed pin %d\n", line);
    #endif
};

uint8_t _read_pins(int base) {
    uint8_t value = 0;

    for (int i = 0; i < 8; i++) {
        value |= gpio_get(GPIO_IN_DATA_BASE_PIN+i) << i;
    }

    return value;
}


#define INPUT_MASK(x) (x & 0x3fc)
#define OUTPUT_MASK(x) (x & 0x3fc00)
#define NORMALISE_INPUT_MASK(x) (INPUT_MASK(x) >> 2)
#define NORMALISE_OUTPUT_MASK(x) (OUTPUT_MASK(x) >> 10)

uint32_t _read_32b_word() {
    uint32_t value = 0;
    for (int i = 0; i < 4; i++) {
        //                  fill topmost byte first vvvvvvvv
        value += _read_pins(GPIO_IN_DATA_BASE_PIN) << 8 * (3-i);
        _pulse_control_line(READ__PTP_B);
    }

    return value;
}

read_packet_t babyif_read_data() {
    read_packet_t packet = {0};

    // NOTE: reset modules on each read? shouldn't affect the data...
    // for now we'll assume it has been previously reset and is in a good state

    //                  GPIO PINS
    // 0b 0000 0000 0000 0000 0000 0000 0000 0000
    //                               ^--------^ 0x3FC   -> input mask      
    //                     ^--------^           0x3FC00 -> output mask

    // NOTE: the masking seems to be incorrect? replaced with _read_32b_word() for now
    // TODO: fix masks?
    packet.address = _read_32b_word();
    packet.data = _read_32b_word();
    packet.pc = _read_32b_word();
    packet.ir = _read_32b_word();
    packet.acc = _read_32b_word();

    #ifdef BIF_DEBUG
        printf("[babyif_read_data] returning: address: %#10x, data: %#10x\n", packet.address, packet.data);
    #endif

    return packet;
}


void babyif_write_data(uint32_t data) {
    uint8_t data_byte = 0;

    for (int i = 0; i < 4; i++) {
        data_byte = (data & (0xFF000000 >> 8 * i)) >> 8 * (3-i);

        // TODO: is there a way to use gpio_set_mask() instead?
        for (int j = 0; j < 8; j++) {
            bool bit = data_byte & 0x1 << j;
            gpio_put(GPIO_OUT_DATA_BASE_PIN + j, bit);
        }

        #ifdef BIF_DEBUG
            uint8_t gpio_actual = NORMALISE_OUTPUT_MASK(gpio_get_all());
            printf("[babyif_write_data] data: %#4x, gpio: %#4x, gpio==data? %s\n", data_byte, gpio_actual, gpio_actual == data_byte ? "true" : "false");
        #endif

        // pulse write line
        _pulse_control_line(WRITE__PTP_A);
    }
}

#ifndef DO_NOT_USE_BIF_SM
#error "Not implemented"
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

    babyif->memory = program;
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
        sleep_ms(1);
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
    /* Allow execution */
    gpio_put(GPIO_OUT_ALLOW_EXEC, 1);
}

void inline babyif_clear_exec() {
    /* Disallow execution */
    gpio_put(GPIO_OUT_ALLOW_EXEC, 0);
}
bool inline babyif_get_exec() {
    /* Get exec pin state */
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

uint32_t babyif_memory_read(babyif* babyif, uint32_t address) {

    if (address >= PROGRAM_SIZE-1) {
        printf("[babyif_memory_read] attempted to access address out of bounds: %#10x but PROGRAM_SIZE=%#10x", address, PROGRAM_SIZE);
        return NULL;
    }

    #ifdef DEBUG
        printf("[babyif_memory_read] accessing memory %#10x, returning %#10x", address, babyif->memory[address]);
    #endif

    return babyif->memory[address];
    
}

void babyif_memory_write(babyif* babyif, uint32_t address, uint32_t data) {

    if (address >= PROGRAM_SIZE-1) {
        printf("[babyif_memory_write] attempted to access address out of bounds: %#10x but PROGRAM_SIZE=%#10x", address, PROGRAM_SIZE);
        return NULL;
    }

    #ifdef DEBUG
        printf("[babyif_memory_write] writing %#10x to %#10x", data, address);
    #endif

    babyif->memory[address] = data;

}

void babyif_clock_cycles(babyif* babyif, int clock_cycles) {

    for (int i = 0; i < clock_cycles; i++) {
        # ifdef DEBUG
            printf("[babyif_clock_cycles] clock cycle %d", i);
        #endif

        while (!babyif_get_generated_clock_irq(babyif)) {};
        babyif_clear_generated_clock_irq(babyif);

        #ifdef DEBUG
            printf(" done\n");
        #endif
    }
}
#endif