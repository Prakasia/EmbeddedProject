# EmbeddedProject
# Environmental Monitoring System

The Environmental monitoring system is used for collecting data samples from the sensors deployed in the environment and triggering warnings if the collected samples vary from its ideal values. 
The data collected are the amount of VOC gases, soil humidity and temperature within the setup.

## Design
 Two RPI PICO was used : 
 Target Pico (Sensor Module):: Reads Soil Moisture Sensor, Air quality sensor and BME680  
 Controller (Controller Module):: Reads data from the target pico, it also recieves an interrupt 

###Sensors Used
 1. Temperature Sensor (BME680) - I2C for communication
 2. Soil Moisture Sensor (SEN0114) - ADC channel
 3. Air Quality Sensor - ADC channel 
