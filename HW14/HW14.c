#include <stdio.h>
#include "pico/stdlib.h"

#define HX711_DT 14
#define HX711_SCK 15

void hx711_init() {
    gpio_init(HX711_DT);
    gpio_set_dir(HX711_DT, GPIO_IN);

    gpio_init(HX711_SCK);
    gpio_set_dir(HX711_SCK, GPIO_OUT);
    gpio_put(HX711_SCK, 0);
}

int32_t hx711_read(void) {
    int32_t raw = 0;

    // Wait for the HX711 to be ready
    while (gpio_get(HX711_DT)) {
        tight_loop_contents();
    }

    // Read 24 bits of data
    for (int i = 0; i < 24; i++) {
        gpio_put(HX711_SCK, 1);
        sleep_us(1); // Short delay for timing
        raw <<= 1; // Shift left to make room for the next bit
        if (gpio_get(HX711_DT)) {
            raw++; // Set the least significant bit if DT is high
        }
        gpio_put(HX711_SCK, 0);
        sleep_us(1); // Short delay for timing
    }

    // Set the gain for the next reading (128 gain)
    gpio_put(HX711_SCK, 1);
    sleep_us(1); // Short delay for timing
    gpio_put(HX711_SCK, 0);
    sleep_us(1); // Short delay for timing

    if (raw & 0x800000) { // If the sign bit is set
        raw |= 0xFF000000; // Sign extend to 32 bits
    }

    return (int32_t) raw;
}

int main(void)
{
    stdio_init_all();
    sleep_ms(2000); // Wait for the USB connection to be established
    hx711_init();

    printf("HX711 test starting...\n");

    while (true) {
        int32_t value = hx711_read();
        printf("Raw value: %d\n", value);
        sleep_ms(100);
    }
}
