/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(app, CONFIG_LOG_MAX_LEVEL);


/*Max Entries*/
#define APP_EVENT_QUEUE_SIZE 20 


/*Event types in the application*/
enum app_event_type
{
  APP_EVENT_SENSOR,
  APP_EVENT_TIMER,
  APP_EVENT_ERROR,
};


/*App Event struct*/
struct app_event {
  enum app_event_type type;

  union {
    int err;
    uint32_t value;
  };
};


/*Setup Message Queue*/
//queu name, size of elements size of queue, 
K_MSGQ_DEFINE(app_msgq, sizeof(struct app_event), APP_EVENT_QUEUE_SIZE, 4);


/*Setup Timer*/
void timer_expiry_fn(struct k_timer *dummy){
  struct app_event evt = {
    *type 
  }
}


/* 1000 msec = 1 sec */
#define SLEEP_TIME_MS 1000

/* The devicetree node identifier for the "led0" alias. */
#define LED0_NODE DT_ALIAS(led2)
#define LED1_NODE DT_ALIAS(led1)

/*
 * A build error on this line means your board is unsupported.
 * See the sample documentation for information on how to fix this.
 */
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(LED1_NODE, gpios);







int main(void) {
  int ret;
  int ret1;
  bool led_state = true;
  bool led1_state = false;

  if (!gpio_is_ready_dt(&led) && !gpio_is_ready_dt(&led1)) {
    return 0;
  }

  ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
  ret1 = gpio_pin_configure_dt(&led1, GPIO_OUTPUT_ACTIVE);
  if (ret < 0) {
    return 0;
  }
  if (ret1 < 0) {
    return 0;
  }

  while (1) {
    ret = gpio_pin_toggle_dt(&led);
    ret1 = gpio_pin_toggle_dt(&led1);
    if (ret < 0) {
      return 0;
    }

    led_state = !led_state;
    led1_state = !led1_state;
    printf("LED state: %s\n", led_state ? "ON" : "OFF");
    k_msleep(SLEEP_TIME_MS);
  }
  return 0;
}
