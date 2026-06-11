#include <stdio.h>
#include "pico/stdlib.h"

#define BUTTON_PIN 15

int main()
{
    stdio_init_all();
    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN);

    while (true) {
        int raw = gpio_get(BUTTON_PIN);
        int pressed = !raw; // Active low button
        printf("%d\n", pressed);
        sleep_ms(50);
    }
}
