/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <sys/_stdint.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(devHeadsThreads, LOG_LEVEL_DBG);





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


//2. THREAD SETUP
// Lets have some accelometer data to pass around some threads
typedef struct {
  uint32_t x_reading;
  uint32_t y_reading;
  uint32_t z_reading;
}AccelReading;

//Make a Queue that we can share between threads
K_MSGQ_DEFINE(my_queue,               // name of queue
              sizeof(AccelReading),   // Size of queue item  
              16,                     // How many Items we can have at once
              4                       // Alignment of queue items
              );


//Let's make producer and consumer threads to use the queue above
static void producer_func(void *unused1, void *unused2, void *unused3);
static void consumer_func(void *unused1, void *unused2, void *unused3);
//params accept(name , stack size, entry, entry_params, priority, options, delay before start)


K_THREAD_DEFINE(producer ,      // Name of thread 
                2048,           // Stack Size
                producer_func,  // Entry function 
                NULL,           // Entry Params 
                NULL,   
                NULL, 
                6,              // Priority   
                0,              // Kernel Options
                0               // Delay before start
                );


K_THREAD_DEFINE(consumer , 2048, consumer_func, NULL, NULL, NULL, 7, 0, 0);



int main(void)
{

  //Init Led
  gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
  gpio_pin_configure_dt(&led1, GPIO_OUTPUT_ACTIVE);
  //End LED Init

  //start a our timer, wait 500ms then trigger it then trigger it every 500ms
  k_timer_start(&someTimer, K_MSEC(500), K_MSEC(500));

  LOG_INF("New Logs man \r\n");
	
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

static void producer_func(void *unused1, void *unused2, void *unused3) {
  (void)unused1;
  (void)unused2;
  (void)unused3;
  while (1) {
    static AccelReading acc_val = {100, 100, 100};
    int ret;
    /*  Write messages to the message queue
     * Wait as long as it takes to be able to write to queue
     * This is a form of backpressure since we wait until there is demand
     */
    ret = k_msgq_put(&my_queue, &acc_val, K_FOREVER);
    if (ret) {
      LOG_ERR("Return value from k_msgq_put = %d", ret);
    }

    acc_val.x_reading += 1;
    acc_val.y_reading += 1;
    acc_val.z_reading += 1;
    k_msleep(2200);
  }
}
static void consumer_func(void *unused1, void *unused2, void *unused3)
{
  (void) unused1;
  (void) unused2;
  (void) unused3;
  while(1) {
    AccelReading temp;
    int ret;

    /* Note how we use the K_FOREVER wait here this makes our thread unready 
    * Which means it will only be startted when we have data in the queue
    */
    ret = k_msgq_get(&my_queue, &temp, K_FOREVER);
    if (ret){
            LOG_ERR("Return value from k_msgq_get = %d",ret);
        }

    LOG_INF("Values got from the queue: %d.%d.%d\r\n", temp.x_reading, temp.y_reading,
			temp.z_reading);

    
  }

}


