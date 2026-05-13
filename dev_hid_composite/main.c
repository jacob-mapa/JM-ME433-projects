/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "bsp/board_api.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "tusb.h"

#include "usb_descriptors.h"

#define MPU_I2C_PORT i2c0
#define MPU_I2C_SDA_PIN 4
#define MPU_I2C_SCL_PIN 5

#define MODE_BUTTON_PIN 15
#define LED_PIN 19

#define MPU_ADDR 0x68

#define MPU_REG_SMPLRT_DIV 0x19
#define MPU_REG_CONFIG 0x1A
#define MPU_REG_ACCEL_CONFIG 0x1C
#define MPU_REG_PWR_MGMT_1 0x6B
#define MPU_REG_ACCEL_XOUT_H 0x3B

//--------------------------------------------------------------------+
// MACRO CONSTANT TYPEDEF PROTYPES
//--------------------------------------------------------------------+

/* Blink pattern
 * - 250 ms  : device not mounted
 * - 1000 ms : device mounted
 * - 2500 ms : device is suspended
 */
enum  {
  BLINK_NOT_MOUNTED = 250,
  BLINK_MOUNTED = 1000,
  BLINK_SUSPENDED = 2500,
};

static uint32_t blink_interval_ms = BLINK_NOT_MOUNTED;

static bool remote_working_mode = false;

void led_blinking_task(void);
void hid_task(void);

static void initGPIO(void);
static void mpu_init(void);
static void mpu_write_reg(uint8_t reg, uint8_t value);
static void mpu_read_accel(int16_t* ax, int16_t* ay, int16_t* az);

static bool button_pressed_event(void);
//static int8_t accel_to_mouse_delta(int16_t accel_raw);
//static void send_hid_report(uint8_t report_id, uint32_t btn);

/*------------- MAIN -------------*/
int main(void)
{
  board_init();
  initGPIO();
  mpu_init();

  // init device stack on configured roothub port
  tud_init(BOARD_TUD_RHPORT);

  if (board_init_after_tusb) {
    board_init_after_tusb();
  }

  while (1)
  {
    tud_task(); // tinyusb device task
    led_blinking_task();

    hid_task();
  }
}

static void initGPIO(void)
{
  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);
  gpio_put(LED_PIN, 0);

  gpio_init(MODE_BUTTON_PIN);
  gpio_set_dir(MODE_BUTTON_PIN, GPIO_IN);
  gpio_pull_up(MODE_BUTTON_PIN);
}

static void mpu_init(void)
{
  i2c_init(MPU_I2C_PORT, 400 * 1000);
  gpio_set_function(MPU_I2C_SDA_PIN, GPIO_FUNC_I2C);
  gpio_set_function(MPU_I2C_SCL_PIN, GPIO_FUNC_I2C);
  gpio_pull_up(MPU_I2C_SDA_PIN);
  gpio_pull_up(MPU_I2C_SCL_PIN);

  sleep_ms(100);

  // wake up MPU
  mpu_write_reg(MPU_REG_PWR_MGMT_1, 0x00);
  sleep_ms(100);

  mpu_write_reg(MPU_REG_CONFIG, 0x03); // DLPF_CFG = 3, bandwidth = 44Hz

  mpu_write_reg(MPU_REG_SMPLRT_DIV, 0x09);

  mpu_write_reg(MPU_REG_ACCEL_CONFIG, 0x00);
}

static void mpu_write_reg(uint8_t reg, uint8_t value)
{
  uint8_t data[2];

  data[0] = reg;
  data[1] = value;

  i2c_write_blocking(MPU_I2C_PORT, MPU_ADDR, data, 2, false);
}
  
static void mpu_read_accel(int16_t* ax, int16_t* ay, int16_t* az)
{
  uint8_t reg = MPU_REG_ACCEL_XOUT_H;
  uint8_t data[6];

  int result;

  result = i2c_read_blocking(MPU_I2C_PORT, MPU_ADDR, &reg, 1, true);

  if (result < 0) {
    *ax = 0;
    *ay = 0;
    *az = 0;
    return;
  }

  result = i2c_read_blocking(MPU_I2C_PORT, MPU_ADDR, data, 6, false);

  if (result < 0) {
    *ax = 0;
    *ay = 0;
    *az = 0;
    return;
  }

  *ax = (int16_t)((data[0] << 8) | data[1]);
  *ay = (int16_t)((data[2] << 8) | data[3]);
  *az = (int16_t)((data[4] << 8) | data[5]);
}

// Button debounce
static bool button_pressed_event(void){
  static bool previous_raw_state = true;
  static bool stable_state = true;
  static bool press_already_counted = false;
  static uint32_t last_change_time = 0;

  bool current_raw_state = gpio_get(MODE_BUTTON_PIN);
  uint32_t current_ms = board_millis();

  if (current_raw_state != previous_raw_state) {
    previous_raw_state = current_raw_state;
    last_change_time = current_ms;
  }

  if ((current_ms - last_change_time) > 25){
    stable_state = current_raw_state;
  }

  //new press detected
  if ((stable_state == false) && (press_already_counted == false)){
    press_already_counted = true;
    return true;
  }

  //button released, reset press count
  if (stable_state == true){
    press_already_counted = false;
  }
  return false;
}

//**static int8_t accel_to_mouse_delta(int16_t accel_raw);**//

//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

// Invoked when device is mounted
void tud_mount_cb(void)
{
  blink_interval_ms = BLINK_MOUNTED;
}

// Invoked when device is unmounted
void tud_umount_cb(void)
{
  blink_interval_ms = BLINK_NOT_MOUNTED;
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en)
{
  (void) remote_wakeup_en;
  blink_interval_ms = BLINK_SUSPENDED;
}

// Invoked when usb bus is resumed
void tud_resume_cb(void)
{
  blink_interval_ms = tud_mounted() ? BLINK_MOUNTED : BLINK_NOT_MOUNTED;
}
//--------------------------------------------------------------------+
// USB HID
//--------------------------------------------------------------------+

static void send_hid_report(uint8_t report_id, uint32_t btn)
{
  // skip if hid is not ready yet
  if ( !tud_hid_ready() ) return;

  switch(report_id)
  {
    case REPORT_ID_KEYBOARD:
    {
      // use to avoid send multiple consecutive zero report for keyboard
      static bool has_keyboard_key = false;

      if ( btn )
      {
        uint8_t keycode[6] = { 0 };
        keycode[0] = HID_KEY_A;

        tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, keycode);
        has_keyboard_key = true;
      }else
      {
        // send empty key report if previously has key pressed
        if (has_keyboard_key) tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, NULL);
        has_keyboard_key = false;
      }
    }
    break;

    case REPORT_ID_MOUSE:
    {
      int8_t deltax = 5;
      int8_t deltay = 5;

      static int time = 0;
      static int dir = 0;

      if (dir == 0){
        deltax = 5;
        deltay = 0;
      }

      if (dir == 1){
        deltax = 0;
        deltay = 5;
      }

      if (dir == 2){
        deltax = -5;
        deltay = 0;
      }

      if (dir == 3){
        deltax = 0;
        deltay = -5;
      }

      time++;
      if (time == 50){
        dir = dir+1;
        time = 0;
      }
      if (dir == 4){
        dir = 0;
      }

      // no button, right + down, no scroll, no pan
      tud_hid_mouse_report(REPORT_ID_MOUSE, 0x00, deltax, deltay, 0, 0);
    }
    break;

    case REPORT_ID_CONSUMER_CONTROL:
    {
      // use to avoid send multiple consecutive zero report
      static bool has_consumer_key = false;

      if ( btn )
      {
        // volume down
        uint16_t volume_down = HID_USAGE_CONSUMER_VOLUME_DECREMENT;
        tud_hid_report(REPORT_ID_CONSUMER_CONTROL, &volume_down, 2);
        has_consumer_key = true;
      }else
      {
        // send empty key report (release key) if previously has key pressed
        uint16_t empty_key = 0;
        if (has_consumer_key) tud_hid_report(REPORT_ID_CONSUMER_CONTROL, &empty_key, 2);
        has_consumer_key = false;
      }
    }
    break;

    case REPORT_ID_GAMEPAD:
    {
      // use to avoid send multiple consecutive zero report for keyboard
      static bool has_gamepad_key = false;

      hid_gamepad_report_t report =
      {
        .x   = 0, .y = 0, .z = 0, .rz = 0, .rx = 0, .ry = 0,
        .hat = 0, .buttons = 0
      };

      if ( btn )
      {
        report.hat = GAMEPAD_HAT_UP;
        report.buttons = GAMEPAD_BUTTON_A;
        tud_hid_report(REPORT_ID_GAMEPAD, &report, sizeof(report));

        has_gamepad_key = true;
      }else
      {
        report.hat = GAMEPAD_HAT_CENTERED;
        report.buttons = 0;
        if (has_gamepad_key) tud_hid_report(REPORT_ID_GAMEPAD, &report, sizeof(report));
        has_gamepad_key = false;
      }
    }
    break;

    default: break;
  }
}

// Every 10ms, we will sent 1 report for each HID profile (keyboard, mouse etc ..)
// tud_hid_report_complete_cb() is used to send the next report after previous one is complete
void hid_task(void)
{
  // Poll every 10ms
  const uint32_t interval_ms = 10;
  static uint32_t start_ms = 0;

  if ( board_millis() - start_ms < interval_ms) return; // not enough time
  start_ms += interval_ms;

  if (button_pressed_event())
  {
    // Wake up host if we are in suspend mode
    // and REMOTE_WAKEUP feature is enabled by host
    remote_working_mode = !remote_working_mode;
  }
  gpio_put(LED_PIN, remote_working_mode ? 1 : 0);
  
  if (tud_hid_ready())
  {
    send_hid_report(REPORT_ID_MOUSE, 0);
  }
}

//**static void send_hid_report(uint8_t report_id, uint32_t btn);**//

// Invoked when sent REPORT successfully to host
// Application can use this to send the next report
// Note: For composite reports, report[0] is report ID
void tud_hid_report_complete_cb(uint8_t instance, uint8_t const* report, uint16_t len)
{
  (void) instance;
  (void) len;

  uint8_t next_report_id = report[0] + 1u;

  if (next_report_id < REPORT_ID_COUNT)
  {
    send_hid_report(next_report_id, board_button_read());
  }
}

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen)
{
  // TODO not Implemented
  (void) instance;
  (void) report_id;
  (void) report_type;
  (void) buffer;
  (void) reqlen;

  return 0;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize)
{
  (void) instance;

  if (report_type == HID_REPORT_TYPE_OUTPUT)
  {
    // Set keyboard LED e.g Capslock, Numlock etc...
    if (report_id == REPORT_ID_KEYBOARD)
    {
      // bufsize should be (at least) 1
      if ( bufsize < 1 ) return;

      uint8_t const kbd_leds = buffer[0];

      if (kbd_leds & KEYBOARD_LED_CAPSLOCK)
      {
        // Capslock On: disable blink, turn led on
        blink_interval_ms = 0;
        board_led_write(true);
      }else
      {
        // Caplocks Off: back to normal blink
        board_led_write(false);
        blink_interval_ms = BLINK_MOUNTED;
      }
    }
  }
}

//--------------------------------------------------------------------+
// BLINKING TASK
//--------------------------------------------------------------------+
void led_blinking_task(void)
{
  static uint32_t start_ms = 0;
  static bool led_state = false;

  // blink is disabled
  if (!blink_interval_ms) return;

  // Blink every interval ms
  if ( board_millis() - start_ms < blink_interval_ms) return; // not enough time
  start_ms += blink_interval_ms;

  board_led_write(led_state);
  led_state = 1 - led_state; // toggle
}
