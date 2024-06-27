#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include <babyif.h>

#include <clocks.pio.h>

int main()
{
    stdio_init_all();

    PIO pio_clocks = pio0;
    uint pio_clocks_offset = pio_add_program(pio_clocks, &clocks_program);
    uint clock_freq = 1;
    printf("loaded clock program at %d\n", pio_clocks_offset);
    clocks_program_init(pio_clocks, 0, pio_clocks_offset);
    pio_sm_put(pio_clocks, 0, (125000000 / (2 * clock_freq)) - 3);
    pio_sm_set_enabled(pio_clocks, 0, true);

    

    while (true) {
        // printf("Hello, world!\n");
        // sleep_ms(1000);

        // hang until interrupt
        // while (!pio_interrupt_get(pio_clocks, 0));

        // printf("interrupt 0 was set on pio? %s\n", pio_interrupt_get(pio_clocks, 0) ? "true" : "false");

        // if (pio_interrupt_get(pio_clocks, 0)) {
        //     printf("sleeping then clearing interrupt\n");
        //     sleep_ms(250);
        //     pio_interrupt_clear(pio_clocks, 0);
        // }

        // sleep_ms(750);

        uint clk_irq_0 = pio_interrupt_get(pio_clocks, 0);
        uint clk_irq_1 = pio_interrupt_get(pio_clocks, 1);

        // hang until irq fires
        // while (!clk_irq_0 || !clk_irq_1);

        printf("irq0: %d , irq1: %d\n", clk_irq_0, clk_irq_1);

        if (clk_irq_0) {
            printf("\tclearing clk_irq_0\n");
            sleep_ms(250);
            pio_interrupt_clear(pio_clocks, 0);
        }

        if (clk_irq_1) {
            printf("\tclearing clk_irq_1\n");
            sleep_ms(250);
            pio_interrupt_clear(pio_clocks, 1);
        }

        sleep_ms(500);

    }
}
