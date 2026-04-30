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

static void init_i2c(void);
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

    init_i2c();

    ssd1306_setup();

    mpu6050_check_whoami();
    mpu6050_init();

    imu_data_t imu;

    while (1) {
        mpu6050_read_all(&imu);
        printf("AX=%6d AY=%6d AZ=%6d | GX=%6d GY=%6d GZ=%6d | T=%6d || "
               "AX=% .3f g AY=% .3f g AZ=% .3f g | "
               "GX=% .2f dps GY=% .2f dps GZ=%.2f dps | "
               "Temp=%.2f C\n",
               imu.ax_raw, imu.ay_raw, imu.az_raw, 
               imu.gx_raw, imu.gy_raw, imu.gz_raw, 
               imu.temp_raw,
               imu.ax_g, imu.ay_g, imu.az_g, 
               imu.gx_dps, imu.gy_dps, imu.gz_dps, 
               imu.temp_c);

        draw_tilt_vector(&imu);

        sleep_ms(10);
    }

    return 0;
}

static void init_i2c(void) {
    i2c_init(I2C_PORT, 100000); // 100 KHz
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
}

static void mpu_write_reg(uint8_t reg, uint8_t value) {
    uint8_t buf[2] = {reg, value};
    i2c_write_blocking(I2C_PORT, MPU6050_ADDR, buf, 2, false);
}

static uint8_t mpu_read_reg(uint8_t reg) {
    uint8_t value;
    i2c_write_blocking(I2C_PORT, MPU6050_ADDR, &reg, 1, true);
    i2c_read_blocking(I2C_PORT, MPU6050_ADDR, &value, 1, false);
    return value;
}

static void mpu_read_burst(uint8_t start_reg, uint8_t *buf, size_t len) {
    i2c_write_blocking(I2C_PORT, MPU6050_ADDR, &start_reg, 1, true);
    i2c_read_blocking(I2C_PORT, MPU6050_ADDR, buf, len, false);
}

static int16_t combine_bytes(uint8_t high, uint8_t low) {
    return (int16_t)((high << 8) | low);
}

static void mpu6050_check_whoami(void) {
    uint8_t who = mpu_read_reg(WHO_AM_I);
    printf("WHO_AM_I = 0x%02X\n", who);

    if (!(who == 0x68 || who == 0x98)) {
        printf("MPU6050 WHO_AM_I mismatch: expected 0x%02X, got 0x%02X\n", MPU6050_ADDR, who);
        while (1) { 
            sleep_ms(250); 
        }
    }
}

static void mpu6050_init(void) {
    mpu_write_reg(PWR_MGMT_1, 0x00); // Wake up sensor
    sleep_ms(100);
    mpu_write_reg(CONFIG, 0x00); // DLPF_CFG = 3 (44Hz accel, 42Hz gyro)
    mpu_write_reg(GYRO_CONFIG, 0x18); // ±250 dps
    mpu_write_reg(ACCEL_CONFIG, 0x00); // ±2 g
}

static void mpu6050_read_all(imu_data_t *imu) {
    uint8_t buf[14];
    mpu_read_burst(ACCEL_XOUT_H, buf, 14);

    imu->ax_raw = combine_bytes(buf[0], buf[1]);
    imu->ay_raw = combine_bytes(buf[2], buf[3]);
    imu->az_raw = combine_bytes(buf[4], buf[5]);
    imu->temp_raw = combine_bytes(buf[6], buf[7]);
    imu->gx_raw = combine_bytes(buf[8], buf[9]);
    imu->gy_raw = combine_bytes(buf[10], buf[11]);
    imu->gz_raw = combine_bytes(buf[12], buf[13]);

    imu->ax_g = imu->ax_raw * 0.000061f;
    imu->ay_g = imu->ay_raw * 0.000061f;
    imu->az_g = imu->az_raw * 0.000061f;

    imu->gx_dps = imu->gx_raw * 0.007630f;
    imu->gy_dps = imu->gy_raw * 0.007630f;
    imu->gz_dps = imu->gz_raw * 0.007630f;

    imu->temp_c = (imu->temp_raw / 340.0f) + 36.53f;
}

static void draw_pixel_safe(int x, int y, unsigned char color) {
    if (x < 0 || x >= OLED_WIDTH || y < 0 || y >= OLED_HEIGHT) {
        return;
    }
    ssd1306_drawPixel((unsigned char)x, (unsigned char)y, color);
}

static void draw_line(int x0, int y0, int x1, int y1, unsigned char color) {
    int dx = abs(x1 - x0);
    int sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0);
    int sy = y0 < y1 ? 1 : -1; 
    int err = dx + dy;

    while (1) {
        draw_pixel_safe(x0, y0, color);
        if (x0 == x1 && y0 == y1) {
            break;
        }
        int e2 = 2 * err;
        if (e2 >= dy) {
            err += dy; 
            x0 += sx;
        }
        if (e2 <= dx) {
            err += dx; 
            y0 += sy;
        }
    }
}

static void draw_crosshair(int cx, int cy) {
    draw_line(cx - 3, cy, cx + 3, cy, 1);
    draw_line(cx, cy - 3, cx, cy + 3, 1);
}

static void draw_tilt_vector(const imu_data_t *imu) {
    int cx = OLED_WIDTH / 2;
    int cy = OLED_HEIGHT / 2;

    // Scale the tilt vector for better visibility
    const float scale = 20.0f;
    int x1 = cx + (int)(imu->ax_g * scale);
    int y1 = cy - (int)(imu->ay_g * scale);


    ssd1306_clear();
    draw_crosshair(cx, cy);
    draw_line(cx, cy, x1, y1, 1);
    ssd1306_update();
}


