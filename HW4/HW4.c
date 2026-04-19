#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/adc.h"
#include "ssd1306.h"
#include "font.h"

#define I2C_PORT i2c0
#define I2C_SDA 4
#define I2C_SCL 5
#define HEARTBEAT_LED PICO_DEFAULT_LED_PIN

void drawChar(uint16_t x, uint16_t y, char letter);
void drawMessage(uint16_t x, uint16_t y, char*m);

int main()
{
    stdio_init_all();

    // Initialize the heartbeat LED
    gpio_init(HEARTBEAT_LED);
    gpio_set_dir(HEARTBEAT_LED, GPIO_OUT);
    
    // Initialize the I2C interface
    i2c_init(I2C_PORT, 1700*1000);

    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    adc_init();
    adc_gpio_init(26);
    adc_select_input(0); 

    ssd1306_setup();
    ssd1306_clear();
    ssd1306_update();

    while (true) {
        /*
        gpio_put(HEARTBEAT_LED, 1);
        ssd1306_drawPixel(10, 20, 1);
        ssd1306_update();
        sleep_ms(500);
        gpio_put(HEARTBEAT_LED, 0);
        ssd1306_drawPixel(10, 20, 0);
        ssd1306_update();
        sleep_ms(500);
        */

        gpio_put(HEARTBEAT_LED, 1);
       
        absolute_time_t t1, t2;
        t1 = get_absolute_time();

        //read ADC and convert to voltage
        uint16_t adc_raw = adc_read(); // Replace with actual ADC reading
        float voltage = adc_raw * 3.3f / 4095.0f; // Convert to voltage

        ssd1306_clear();
        char message[30];
        sprintf(message, "ADC0 raw=%4u", adc_raw);
        drawMessage(0, 0, message);
        sprintf(message, "ADC0 volts=%1.3f", voltage);
        drawMessage(0, 8, message);
        sprintf(message, "t=%u us", (int)to_us_since_boot(t1));
        drawMessage(0, 16, message);
    
        ssd1306_update();
        t2 = get_absolute_time();
        uint64_t ta;
        ta = to_us_since_boot(t2) - to_us_since_boot(t1);
        char speed[30];
        sprintf(speed,"FPS = %6.3f  ",1.0/(ta/1000000.0));
        drawMessage(0, 24, speed);
        ssd1306_update();
        gpio_put(HEARTBEAT_LED, 0);
        sleep_ms(1000);
    }
}

void drawMessage(uint16_t x, uint16_t y, char*m) {
    int i = 0;
    while (m[i] != '\0') {
        drawChar(x + 6 * i, y, m[i]);
        i++;
    }
}

void drawChar(uint16_t x, uint16_t y, char letter) {
    // Clamp unsupported characters to space
    if (letter < 0x20 || letter > 0x7F) {
        letter = ' ';
    }

    for (int i=0; i<5; i++) {
        char colm = ASCII[letter - 0x20][i];
        for (int j=0; j<8; j++) {
            int bit = (colm >> j) & 0x1;
            ssd1306_drawPixel(x+i, y+j, bit);
        }
    }
}
