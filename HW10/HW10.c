#include <stdio.h>
#include "pico/stdlib.h"

#define BUTTON_PIN 15

int main()
{
    stdio_init_all();

    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN);

    sleep_ms(2000); // Allow time for the USB connection to be established

    while (true) {
        int pressed = !gpio_get(BUTTON_PIN); // Active low button
        printf("BTN %d\n", pressed);
        sleep_ms(20); // Poll every 20 ms
    }
}
