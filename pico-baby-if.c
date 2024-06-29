#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include <malloc.h>

#include <babyif.h>

// #include <clocks.pio.h>


int main()
{
    stdio_init_all();

    babyif *bif = babyif_constructor();
    babyif_init(bif, pio0, pio1, 1);
    
    babyif_init_pio_clock(bif);
    babyif_enable_pio_clock(bif);

    bool prev_clk_irq_0_val = false;
    bool prev_baby_clk_gpio_val = false;
    bool clk_irq_0 = false;
    bool clk_irq_1 = false;
    bool baby_clk_gpio_val = false;


    while (true) {

        clk_irq_0 = babyif_get_generated_clock_irq(bif);
        clk_irq_1 = babyif_get_baby_clock_irq(bif);
        baby_clk_gpio_val = gpio_get(GPIO_IN_BABY_CLOCK_PIN);

        printf("---START\n");
        printf("irq0: %d , irq1: %d\n", clk_irq_0, clk_irq_1);

            // 1      ->    0
        if (baby_clk_gpio_val > prev_baby_clk_gpio_val) {
            printf("\t\tbaby clock has gone high\n");

            //     0              1
        } else if (baby_clk_gpio_val < prev_baby_clk_gpio_val) {
            printf("\t\tbaby clock has gone low\n");
        } else if (baby_clk_gpio_val == prev_baby_clk_gpio_val) {
            if (baby_clk_gpio_val == 1) {
                printf("\t\tbaby clock has remained high\n");
            } else {
                printf("\t\tbaby clock has remained low\n");
            }
        }


        // cannot use irq to check since it gets cleared
        // and this loop runs much faster than the pio
        // (or can do, depending on the freq settings)
        // prev_baby_clk_gpio_val = gpio_get(bif->baby_clock_pin);
        prev_baby_clk_gpio_val = gpio_get(GPIO_IN_BABY_CLOCK_PIN);


        if (clk_irq_0) {
            printf("\tclearing clk_irq_0\n");
            sleep_ms(250);
            babyif_clear_generated_clock_irq(bif);
        }

        if (clk_irq_1) {
            printf("\tclearing clk_irq_1\n");
            sleep_ms(500);
            babyif_clear_baby_clock_irq(bif);
        }

        printf("---END\n");
        sleep_ms(500);
    }
}
