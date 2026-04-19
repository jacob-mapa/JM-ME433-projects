#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "ssd1306.h"
#include "font.h"

#define I2C_PORT i2c0
#define I2C_SDA 4
#define I2C_SCL 5
#define HEARTBEAT_LED PICO_DEFAULT_LED_PIN

//void drawChar(uint16_t x, uint16_t y, char letter);
//void drawMessage(uint16_t x, uint16_t y, char*m);

int main()
{
    stdio_init_all();

    gpio_init(I2C_PORT, 1700*1000);

    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    ssd1306_setup();
    ssd1306_clear();
    ssd1306_update();

    while (true) {
        ssd1306_drawPixel(10, 20, 1);
        ssd1306_update();
        sleep_ms(1000);
        ssd1306_drawPixel(10, 20, 0);
        ssd1306_update();
        sleep_ms(1000);
    }
}
