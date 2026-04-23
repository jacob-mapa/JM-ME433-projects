#include <stdio.h>
#include "pico/stdlib.h"

// add the hardware spi

int main()
{
    stdio_init_all();

    spi_init(spi_default, 1000 * 1000); // the baud, or bits per second
    gpio_set_function(PICO_DEFAULT_SPI_RX_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_SCK_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_TX_PIN, GPIO_FUNC_SPI);

    cs_select(PIN_CS);
    spi_write_blocking(SPI_PORT, data, len); // where data is a uint8_t array with length len
    cs_deselect(PIN_CS);

    float v[100];
    for (i=0;i<100;i++) {
        v[i] = sine(i)

    while (true) {
        float t = 0;
        t += 0.01; // increment time
        float voltage = (sine(2*pi*2*t) + 1.0) / 2.0 * 3.3; // convert sine wave to voltage range
        writeDAC(channel, voltage);
        sleep_ms(10);
    }
}

void writeDAC(int channel, float v){
    uint8_t data[2]:

    data[0] = 0b01110000;

    data[0] = data[0] | ((channel&0b1)<<7); 
    
    uint16_t myV = v/3.3 * 1023;
    
    data[0] = data[0] | ((myV>>6)&0b00001111); // set the channel bit

    data[1] = (myV<<2)&0xFF; // set the second byte to 0

    cs_select(PIN_CS);
    spi_write_blocking(SPI_PORT, data, 2); // where data is a uint8_t array with length len
    cs_deselect(PIN_CS);
}
