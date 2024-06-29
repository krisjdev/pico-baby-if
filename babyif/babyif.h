#pragma once
#include <stdio.h>
#include <malloc.h>

#include <hardware/pio.h>
#include "pico/stdlib.h"

#define GENERATED_CLOCK_IRQ_FLAG 0
#define BABY_CLOCK_IRQ_FLAG 1

#define GPIO_OUT_GENERATED_CLOCK_PIN 15
#define GPIO_IN_BABY_CLOCK_PIN 14

typedef struct babyif_struct {
    PIO pio_clk;
    PIO pio_data;
    uint clk_freq;
    uint generated_clock_pin;
    uint baby_clock_pin;
} babyif;

// typedef babyif* babyif_ptr;

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