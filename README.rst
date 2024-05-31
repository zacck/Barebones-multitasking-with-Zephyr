Barebones Threading in Zephyr as  Demo 


This code does the following 

- Sets Up 3 LEDS as our work indicators 
- Sets up a Message Queue running on the System Queue
- Fires a couple of timers to simulate data coming from Sensors and processes them
- Listens to interrupts from a button and processes that event itself.
- Generally a nifty break into Zephyrs RTOS features
