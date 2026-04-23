#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "ssd1306.h"

// I2C defines
// This example will use I2C0 on GPIO8 (SDA) and GPIO9 (SCL) running at 400KHz.
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define I2C_PORT i2c0
#define I2C_SDA 4
#define I2C_SCL 5

#define MPU6050_ADDR 0x68

// config registers
#define CONFIG 0x1A
#define GYRO_CONFIG 0x1B
#define ACCEL_CONFIG 0x1C
#define PWR_MGMT_1 0x6B
#define PWR_MGMT_2 0x6C

// sensor data registers:
#define ACCEL_XOUT_H 0x3B
#define ACCEL_XOUT_L 0x3C
#define ACCEL_YOUT_H 0x3D
#define ACCEL_YOUT_L 0x3E
#define ACCEL_ZOUT_H 0x3F
#define ACCEL_ZOUT_L 0x40
#define TEMP_OUT_H   0x41
#define TEMP_OUT_L   0x42
#define GYRO_XOUT_H  0x43
#define GYRO_XOUT_L  0x44
#define GYRO_YOUT_H  0x45
#define GYRO_YOUT_L  0x46
#define GYRO_ZOUT_H  0x47
#define GYRO_ZOUT_L  0x48
#define WHO_AM_I     0x75

#define OLED_WIDTH 128
#define OLED_HEIGHT 32

typedef struct{
    int16_t ax_raw;
    int16_t ay_raw;
    int16_t az_raw;
    int16_t temp_raw;
    int16_t gx_raw;
    int16_t gy_raw;
    int16_t gz_raw;

    float ax_g;
    float ay_g;
    float az_g;
    float temp_c;
    float gx_dps;
    float gy_dps;
    float gz_dps;
} imu_data_t;

static void init_i2c(void):
static void mpu_write_reg(uint8_t reg, uint8_t value);
static uint8_t mpu_read_reg(uint8_t reg);
static void mpu_read_burst(uint8_t start_reg, uint8_t *buf, size_t len);
static int16_t combine_bytes(uint8_t high, uint8_t low);
static void mpu6050_check_whoami(void);
static void mpu6050_init(void);
static void mpu6050_read_all(imu_data_t *imu);

static void draw_pixel_safe(int x, int y, unsigned char color);
static void draw_line(int x0, int y0, int x1, int y1, unsigned char color);
static void draw_crosshair(int cx, int cy);
static void draw_tilt_vector(const imu_data_t *imu);

int main()
{
    stdio_init_all();
    sleep_ms(2000);

    // I2C Initialisation. Using it at 400Khz.
    i2c_init(I2C_PORT, 400*1000);
    
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    // For more examples of I2C use see https://github.com/raspberrypi/pico-examples/tree/master/i2c

    while (true) {
        printf("Hello, world!\n");
        sleep_ms(1000);
    }
}
