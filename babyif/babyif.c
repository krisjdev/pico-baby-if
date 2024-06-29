#include <babyif.h>
#include <clocks.pio.h>

babyif* babyif_constructor() {
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