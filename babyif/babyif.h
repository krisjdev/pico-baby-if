#pragma once
#include "pico/stdlib.h"

typedef struct {
    uint32_t address;
    uint32_t data;
} read_packet;

void babyif_init_gpio();
void babyif_pulse_clock(uint32_t period);
read_packet babyif_read_data();
void babytif_write_data(uint32_t data);

void _pulse_control_line(bool pulse_read_line);


#ifndef DO_NOT_USE_BIF_SM
#error "Not implemented"
#define GENERATED_CLOCK_IRQ_FLAG 0
#define BABY_CLOCK_IRQ_FLAG 1

#define SENT_DATA_IRQ_FLAG 1
#define RECEIVED_DATA_IRQ_FLAG 0

#define CONFIG_WRITE 0
#define CONFIG_READ 1

#define SENDER_STATE_MACHINE 1
#define RECEIVER_STATE_MACHINE 0

// 2^5 -- baby has 5 address bits
// const uint32_t MEMORY_MAXIMUM_SIZE = 1 << 5;

typedef struct babyif_struct {
    PIO pio_clk;
    PIO pio_data;
    uint clk_freq;
    uint generated_clock_pin;
    uint baby_clock_pin;
    uint32_t *memory;
} babyif;


babyif* babyif_constructor(void);
void babyif_destructor(babyif* babyif);

void babyif_init(babyif* babyif, PIO pio_clk, PIO pio_data, uint clk_freq);

// pio0: clock generator

void babyif_init_pio_clock(babyif* babyif);

void babyif_enable_pio_clock(babyif* babyif);
void babyif_disable_pio_clock(babyif* babyif);
void babyif_restart_pio_clock(babyif* babyif);

bool babyif_get_generated_clock_irq(babyif* babyif);
void babyif_clear_generated_clock_irq(babyif* babyif);

bool babyif_get_baby_clock_irq(babyif* babyif);
void babyif_clear_baby_clock_irq(babyif* babyif);


// pio1: data handler

void babyif_init_pio_data(babyif* babyif);

void babyif_enable_pio_data(babyif* babyif);
void babyif_disable_pio_data(babyif* babyif);
void babyif_restart_pio_data(babyif* babyif);

void babyif_clear_pio_data_fifos(babyif* babyif);

bool babyif_get_sent_data_irq(babyif* babyif);
void babyif_clear_sent_data_irq(babyif* babyif);

bool babyif_get_received_data_irq(babyif* babyif);
void babyif_clear_received_data_irq(babyif* babyif);

void babyif_put_data_word(babyif* babyif, uint32_t word);
uint32_t babyif_get_data_word(babyif* babyif);
uint32_t babyif_get_data_word_gpio(babyif* babyif);

// misc
bool babyif_get_stop_lamp();
bool babyif_get_ram_read_write_intent();

void babyif_set_exec();
void babyif_clear_exec();
bool babyif_get_exec();

void babyif_set_reset();
void babyif_clear_reset();
bool babyif_get_reset();

// memory functions
// void babyif_memory_clear(babyif* babyif); // unnecessary
// void babyif_memory_load(babyif* babyif, int *program_array, int program_size);
uint32_t babyif_memory_read(babyif* babyif, uint32_t address);
void babyif_memory_write(babyif* babyif, uint32_t address, uint32_t data);

void babyif_clock_cycles(babyif* babyif, int clock_cycles);
#endif