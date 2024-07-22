#pragma once
#include <stdio.h>
#include <malloc.h>

#include <hardware/pio.h>
#include "pico/stdlib.h"

#define GENERATED_CLOCK_IRQ_FLAG 0
#define BABY_CLOCK_IRQ_FLAG 1

#define SENT_DATA_IRQ_FLAG 1
#define RECEIVED_DATA_IRQ_FLAG 0

#define CONFIG_WRITE 0
#define CONFIG_READ 1

#define SENDER_STATE_MACHINE 1
#define RECEIVER_STATE_MACHINE 0
 

typedef struct babyif_struct {
    PIO pio_clk;
    PIO pio_data;
    uint clk_freq;
    uint generated_clock_pin;
    uint baby_clock_pin;
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


// misc
bool babyif_get_stop_lamp();
bool babyif_get_ram_read_write_intent();

void babyif_set_exec_signal();
void babyif_clear_exec_signal();
bool babyif_get_exec_signal();

void babyif_set_reset_signal();
void babyif_clear_reset_signal();
bool babyif_get_reset_signal();