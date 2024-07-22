#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include <malloc.h>

#include "babyif.h"
#include "pindefs.h"


int main()
{
    stdio_init_all();

    babyif *bif = babyif_constructor();
    babyif_init(bif, pio0, pio1, 1);
    
    babyif_init_pio_clock(bif);
    babyif_enable_pio_clock(bif);

    babyif_init_pio_data(bif);
    babyif_enable_pio_data(bif);

    bool prev_clk_irq_0_val = false;
    bool prev_baby_clk_gpio_val = false;
    bool clk_irq_0 = false;
    bool clk_irq_1 = false;
    bool baby_clk_gpio_val = false;
    uint32_t val = 0;

    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

    while (true) {

        clk_irq_0 = babyif_get_generated_clock_irq(bif);
        clk_irq_1 = babyif_get_baby_clock_irq(bif);
        baby_clk_gpio_val = gpio_get(GPIO_IN_BABY_CLOCK_PIN);


        printf("---START\n");
        printf("[clk] irq0: %d , irq1: %d\n", clk_irq_0, clk_irq_1);

        printf("[data] irq_read: %d, irq_write: %d\n", babyif_get_received_data_irq(bif), babyif_get_sent_data_irq(bif));


        // printf("write sm pc: %d\n", pio_sm_get_pc(bif->pio_data, 1));

        // 00000000 00000000 00000000 00000000 
        // 10101010_11110000_10101010_11110000 -> AAF0AAF0

        // test if sending correct data by connecting to receiving pio
        // babyif_put_data_word(bif, 0b00001111000010100000111100001010);
        babyif_put_data_word(bif, 0x1234ABCD);
        babyif_clear_sent_data_irq(bif);
        
        printf("rx word: %#10x", babyif_get_data_word(bif));

        

            // 1      ->    0
        if (baby_clk_gpio_val > prev_baby_clk_gpio_val) {
            printf("\t\tbaby clock has gone high\n");
            gpio_put(PICO_DEFAULT_LED_PIN, 1);

            //     0              1
        } else if (baby_clk_gpio_val < prev_baby_clk_gpio_val) {
            printf("\t\tbaby clock has gone low\n");
            gpio_put(PICO_DEFAULT_LED_PIN, 0);

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
