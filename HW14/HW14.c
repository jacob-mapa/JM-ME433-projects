#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "pico/stdlib.h"

#define HX711_DT 14
#define HX711_SCK 15
#define MAX_SAMPLES 5000

#define IIR_ALPHA 0.10f

static int32_t raw_data[MAX_SAMPLES];
static float filtered_data[MAX_SAMPLES];
static uint32_t time_data[MAX_SAMPLES];

void hx711_init(void) {
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

int get_sample_count_from_usb(void) {
    char buffer[32];
    int index = 0;
    int ch;

    printf("Enter the number of samples to collect, then press Enter:\n");

    while (true) {
        ch = getchar_timeout_us(0);

        if (ch != PICO_ERROR_TIMEOUT) {
            if (ch == '\r' || ch == '\n') { // Check for Enter key
                buffer[index] = '\0'; // Null-terminate the string
                break; // Convert to integer and return
            } 
            if (index < sizeof(buffer) - 1) { // Ensure we don't overflow the buffer
                buffer[index++] = (char) ch; // Store the character
                index++;
            }
        }
    }

    int n = 0;

    scanf("%d", &n); // Read the number of samples from USB

    if (n < 1) {
        n = 1;
    }

    if (n > MAX_SAMPLES) {
        n = MAX_SAMPLES;
    }
    return n;
}

int main(void)
{
    stdio_init_all();
    sleep_ms(2000); // Wait for the USB connection to be established
    hx711_init();

    printf("HX711 data collection starting...\n");
    printf("Using DT = GPIO%d, SCK = GPIO%d\n", HX711_DT, HX711_SCK);

    while (true) {
        int n_samples = get_sample_count_from_usb();
        printf("Collecting %d samples...\n", n_samples);
        uint32_t start_time = to_ms_since_boot(get_absolute_time());

        for (int i = 0; i < n_samples; i++) {
            int32_t raw = hx711_read();
            uint32_t now = to_ms_since_boot(get_absolute_time() - start_time);
            raw_data[i] = raw;
            time_data[i] = now;

            if (i == 0) {
                filtered_data[i] = (float) raw; // Initialize the filter with the first sample
            } else {
                filtered_data[i] = IIR_ALPHA * (float) raw + (1.0f - IIR_ALPHA) * filtered_data[i - 1];
            }
        }
        printf("time_ms,raw,filtered\n");
        for (int i = 0; i < n_samples; i++) {
            printf("%lu,%d,%.2f\n", time_data[i], raw_data[i], filtered_data[i]);
        }
        printf("Data collection complete. Enter the number of samples to collect, then press Enter:\n");
    }
}
