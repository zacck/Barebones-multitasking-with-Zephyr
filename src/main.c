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
enum app_event_type {
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
// queu name, size of elements size of queue,
K_MSGQ_DEFINE(app_msgq, sizeof(struct app_event), APP_EVENT_QUEUE_SIZE, 4);

/*Setup Timer Callback*/
void timer_expiry_fn(struct k_timer *dummy) {
  struct app_event evt = {
      .type = APP_EVENT_TIMER,
      .value = 1234,
  };

  k_msgq_put(&app_msgq, &evt, K_NO_WAIT);
}

void sensor_expiry_fn(struct k_timer *dummy) {
  struct app_event evt = {
      .type = APP_EVENT_SENSOR,
      .value = 4321,
  };

  k_msgq_put(&app_msgq, &evt, K_NO_WAIT);
}

/*Define the Timer*/
K_TIMER_DEFINE(timer, timer_expiry_fn, NULL);
K_TIMER_DEFINE(timer1, sensor_expiry_fn, NULL);

/* 1000 msec = 1 sec */
#define SLEEP_TIME_MS 1000

/* The devicetree node identifier for the "led0" alias. */
#define LED0_NODE DT_ALIAS(led0)
#define LED2_NODE DT_ALIAS(led2)

/*
 * A build error on this line means your board is unsupported.
 * See the sample documentation for information on how to fix this.
 */
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static const struct gpio_dt_spec led2 = GPIO_DT_SPEC_GET(LED2_NODE, gpios);

int main(void) {
  /*GPIO STUFF*/
  int ret;
  int ret1;

  if (!gpio_is_ready_dt(&led))  {
    LOG_ERR("GPIO1 could not init");
  }

  if (!gpio_is_ready_dt(&led2)) {
    LOG_ERR("GPIO1 could not init");
  }

  //Configure both LEDS to start at off
  ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
  ret1 = gpio_pin_configure_dt(&led2, GPIO_OUTPUT_ACTIVE);
 
  /*END GPIO SETUP*/

  /**/
  struct app_event evt;
  LOG_INF("Lets Start threading");
  k_timer_start(&timer, K_MSEC(599), K_MSEC(44));
  k_timer_start(&timer1, K_MSEC(234), K_MSEC(165));

  while (1) {
    // poll the event queue indefinitely
    k_msgq_get(&app_msgq, &evt, K_FOREVER);

    LOG_INF("Event type: %i", &evt.type);

    switch (evt.type) {
    case APP_EVENT_TIMER:
      LOG_INF("Timer event value: %i", evt.value);
      ret = gpio_pin_toggle_dt(&led);
      break;
    case APP_EVENT_SENSOR:
      LOG_INF("SENSOR event value: %i", evt.value);
      ret1 = gpio_pin_toggle_dt(&led2);
      break;
    default:
      break;
    }
  }
  return 0;
}
