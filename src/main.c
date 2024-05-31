/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "zephyr/devicetree.h"
#include "zephyr/kernel/thread_stack.h"
#include "zephyr/logging/log_core.h"
#include "zephyr/sys/cbprintf.h"
#include <stdio.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>
#include <zephyr/sys/printk.h>
#include <inttypes.h>
#include <stdio.h>
LOG_MODULE_REGISTER(threading_app, LOG_LEVEL_DBG);
/*Max Entries*/
#define APP_EVENT_QUEUE_SIZE 20

/*Event types in the application*/
enum app_event_type {
  APP_EVENT_SENSOR,
  APP_EVENT_TIMER,
  APP_EVENT_BUTTON,
  APP_EVENT_ERROR,
};

/*App Event struct*/
struct app_event {
  enum app_event_type type;
  struct k_work work;

  union {
    int err;
    uint32_t value;
  };
};

/*Setup Message Queue*/
// queu name, size of elements size of queue,
K_MSGQ_DEFINE(app_msgq, sizeof(struct app_event), APP_EVENT_QUEUE_SIZE, 4);

//separate work queue
K_THREAD_STACK_DEFINE(app_stack_area, 2048);

static struct k_work_q app_work_q;

/*Work*/
static struct k_work some_work;

static void some_work_fn(struct k_work *work)
{
  LOG_INF("Separate thread Button pressed at %"PRIu32 "\n", k_cycle_get_32()); 

}






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
#define LED3_NODE DT_ALIAS(led3)


//Get Button
#define SW0_NODE  DT_ALIAS(sw0)
#if  !DT_NODE_HAS_STATUS(SW0_NODE, okay)
#error "Unsupported board: sw0 devicetree alias not defined"
#endif

static struct gpio_callback button_cb_data;


static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET_OR(SW0_NODE, gpios, {0});


/*
 * A build error on this line means your board is unsupported.
 * See the sample documentation for information on how to fix this.
 */
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static const struct gpio_dt_spec led2 = GPIO_DT_SPEC_GET(LED2_NODE, gpios);
static const struct gpio_dt_spec led3 = GPIO_DT_SPEC_GET(LED3_NODE, gpios);

void button_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins){
  LOG_INF("Button pressed at %"PRIu32 "\n", k_cycle_get_32()); 
  k_work_submit_to_queue(&app_work_q, &some_work);
}





int main(void) {

  printf("I am just hard to reach \n");
  /*GPIO STUFF*/
  int ret;
  int ret1;

  if (!gpio_is_ready_dt(&led))  {
    LOG_ERR("LED1 could not init\n");
  }

  if (!gpio_is_ready_dt(&led2)) {
    LOG_ERR("LED2 could not init\n");
  }

  if (!gpio_is_ready_dt(&led3)) {
    LOG_ERR("LED3 could not init\n");
  }

  //Configure LEDS to start at off
  ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
  ret = gpio_pin_configure_dt(&led2, GPIO_OUTPUT_ACTIVE);
  ret = gpio_pin_configure_dt(&led3, GPIO_OUTPUT_ACTIVE);

  if(!gpio_is_ready_dt(&button)){
    printk("Error: button device %s is not ready \n", button.port->name);
    return  0;
  }

  ret = gpio_pin_configure_dt(&button, GPIO_INPUT);
  if(ret != 0){
    printk("Error %d: failed to configure %s pin %d\n", ret, button.port->name, button.pin);
    return 0;
  }

  ret = gpio_pin_interrupt_configure_dt(&button, GPIO_INT_EDGE_TO_ACTIVE);

  if (ret != 0) {
		printk("Error %d: failed to configure interrupt on %s pin %d\n",
			ret, button.port->name, button.pin);
		return 0;
	}

  gpio_init_callback(&button_cb_data, button_pressed, BIT(button.pin));
  gpio_add_callback(button.port, &button_cb_data);
  printk("Set up button at %s pin %d\n", button.port->name, button.pin); 
  /*END GPIO SETUP*/

  
  /**/
  struct app_event evt;
  LOG_INF("Lets Start threading");
  k_timer_start(&timer, K_MSEC(2000), K_MSEC(4400));
  k_timer_start(&timer1, K_MSEC(4000), K_MSEC(16500));


  while (1) {
    int val = gpio_pin_get_dt(&button);

			if (val >= 0) {

				gpio_pin_set_dt(&led3, val);
			}
    // poll the event queue indefinitely
    k_msgq_get(&app_msgq, &evt, K_FOREVER);
    
    //start off system queue thread
    k_work_queue_start(&app_work_q, app_stack_area, sizeof(app_stack_area), CONFIG_SYSTEM_WORKQUEUE_PRIORITY, NULL);

    k_work_init(&some_work,some_work_fn);

    switch (evt.type) {
    case APP_EVENT_TIMER:
      LOG_INF("Timer event value: %i\n", evt.value);
      ret = gpio_pin_toggle_dt(&led);
      break;
    case APP_EVENT_SENSOR:
      LOG_INF("SENSOR event value: %i\n", evt.value);
      ret1 = gpio_pin_toggle_dt(&led2);
      break;
    case APP_EVENT_BUTTON:
      LOG_INF("Delayed BUTTON event value: %i\n", evt.value);
      ret1 = gpio_pin_toggle_dt(&led3);
      break;
    default:
      break;
    }
  }
  return 0;
}
