/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>



//1. TIMER SETUP
/* Define a led that we will use with our timers */
#define LED0_NODE DT_ALIAS(led0)
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
#define LED1_NODE DT_ALIAS(led1)
static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(LED1_NODE, gpios);
/* END LED Defines*/

/*Define our Timer and Handler*/
static void timerHandler(struct k_timer *dummy);
K_TIMER_DEFINE(someTimer, timerHandler, NULL);
/*END timer Handler*/









int main(void)
{

  //Init Led
    gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
    gpio_pin_configure_dt(&led1, GPIO_OUTPUT_ACTIVE);
  //End LED Init

  

  //start a our timer, wait 500ms then trigger it then trigger it every 500ms
  k_timer_start(&someTimer, K_MSEC(500), K_MSEC(500));
	
	return 0;
}

static void timerHandler(struct k_timer *dummy){
  static bool flip = true;
   if (flip)
        gpio_pin_toggle_dt(&led);
   else
        gpio_pin_toggle_dt(&led1);

  flip = !flip;
}
