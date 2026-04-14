#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

#define MCP23008_ADDR 0x20

#define IODIR 0x00
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

    while (1) {
        gpio_put(HEARTBEAT_LED, 1);

        mcp_write(MCP23008_ADDR, OLAT, (1 << 7));
        sleep_ms(200);

        gpio_put(HEARTBEAT_LED, 0);

        mcp_write(MCP23008_ADDR, OLAT, 0x00);
        sleep_ms(200);
    }
}
