#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

#define MCP23008_ADDR 0x20

#define IODIR 0x00
#define GPIO 0x09
#define OLAT 0x0A

// I2C defines
// This example will use I2C0 on GPIO8 (SDA) and GPIO9 (SCL) running at 400KHz.
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define I2C_PORT i2c0
#define I2C_SDA 4
#define I2C_SCL 5

#define HEARTBEAT_LED PICO_DEFAULT_LED_PIN

void mcp_write(unsigned char ADDR, unsigned char reg, unsigned char value) {
    unsigned char buf[2];
    buf[0] = reg;
    buf[1] = value;
    i2c_write_blocking(I2C_PORT, ADDR, buf, 2, false);
}

unsigned char mcp_read(unsigned char ADDR, unsigned char reg) {
    unsigned char value;
    i2c_write_blocking(I2C_PORT, ADDR, &reg, 1, true);
    i2c_read_blocking(I2C_PORT, ADDR, &value, 1, false);
    return value;
}

int main()
{
    stdio_init_all();

    gpio_init(HEARTBEAT_LED);
    gpio_set_dir(HEARTBEAT_LED, GPIO_OUT);
    gpio_put(HEARTBEAT_LED, 0);

    // I2C Initialisation. Using it at 100Khz.
    i2c_init(I2C_PORT, 100*1000);
    
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    // For more examples of I2C use see https://github.com/raspberrypi/pico-examples/tree/master/i2c

    sleep_ms(500); // Sleep for a bit to let the I2C device boot up

    mcp_write(MCP23008_ADDR, IODIR, 0x7F);

    mcp_write(MCP23008_ADDR, OLAT, 0x00);

    unsigned char heartbeat_state = 0;
    unsigned char expander_output = 0x00;

    while (1) {
        heartbeat_state = !heartbeat_state;
        gpio_put(HEARTBEAT_LED, heartbeat_state);

        unsigned char gpio_val = mcp_read(MCP23008_ADDR, GPIO);

        unsigned char gp0_state = gpio_val & 0x01;

        if (gp0_state == 0) {
            expander_output = (1 << 7);
        } else {
            expander_output &= ~(1 << 7);       
        }

        mcp_write(MCP23008_ADDR, OLAT, expander_output);
        sleep_ms(200);
    }
}
