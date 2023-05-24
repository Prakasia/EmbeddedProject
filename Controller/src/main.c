
/*
 * **********	MASTER CODE		***********
 * 	-> BME680
 * 	-> SEN0114
 * 	-> AIR QUALITY SENSOR V1.3
 * 
 * --------------------------------------------------
 * 
 * 	Editor : Shalu Prakasia
 * 
 * --------------------------------------------------
 * Soil Moisture Sensor value Range
 * 0 	~	300	 - dry soil
 * 300 	~	700  - humid soil
 * 700	~	1023 - water
 * --------------------------------------------------
 * Air Quality Sensor value Range
 * 0	~	300	- Fresh air 
 * 300	~	700	- Low pollution
 * > 700		- High pollution
 * --------------------------------------------------
 *
*/

#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>
#include <stdio.h>

#define I2C_NODE    DT_NODELABEL(i2c1) 	
#define I2C_TARGET_ADDRESS 0x07
#define I2C_BUF_LEN 12

#define DELAY_MS  500
#define STACK_SIZE  500

#define SOIL_LED            DT_ALIAS(ledsoil)           //LED FOR INTERRUPT INDICATOR
#define SOIL_INPUT            DT_ALIAS(soilpin)           //LED FOR INTERRUPT 

bool moisture_flag;

struct gpio_dt_spec soil_pin = GPIO_DT_SPEC_GET(SOIL_INPUT, gpios); 
struct gpio_dt_spec soil_led = GPIO_DT_SPEC_GET(SOIL_LED, gpios); 
static struct gpio_callback soil_cb_data;

K_SEM_DEFINE(sem, 0, 1);
K_MUTEX_DEFINE(mutex);

void response_isr() {
    k_sem_give(&sem);
}

// void main()
void main_task()
{
    int ret;

    uint8_t i2c_rx_data[I2C_BUF_LEN];
    int16_t soil_moisture,air_quality;
    int32_t temperature;
    // int32_t humidity;
    const struct device *const dev = DEVICE_DT_GET(I2C_NODE);
    if (!device_is_ready(dev)) { 
        printk("error");
        return;
	}
    else{
        printk("\nDevice ready\n");
    }

    while (1) {
        ret = i2c_read(dev,i2c_rx_data,12,I2C_TARGET_ADDRESS);
        if (ret < 0) {
            printk("Error\n");
        } 
        else {
            soil_moisture = (int16_t)(((int16_t)i2c_rx_data[0] << 8) | i2c_rx_data[1]);
            air_quality = (int16_t)(((int16_t)i2c_rx_data[2] << 8) | i2c_rx_data[3]);
            temperature = (int32_t)((int32_t)i2c_rx_data[8] << 24) | ((int32_t)i2c_rx_data[9] << 16) | ((int16_t)i2c_rx_data[10] << 8) | (i2c_rx_data[11]);

            printk("\nSoil Moisture value = %d", soil_moisture);
            printk("\nAir Quality value = %d", air_quality);
            printk("\nTemperature = %d.%06d degC", (temperature / 100), ((temperature % 100) * 1000));
            printk("\n******************************************************************************\n");

            if(soil_moisture > 300 && soil_moisture < 800)
            {
                k_mutex_lock(&mutex, K_FOREVER);
                moisture_flag = 0;
                k_mutex_unlock(&mutex);
            }
        }
        k_sleep(K_MSEC(5000));
    }
}

void trigger_task(){
    gpio_pin_configure_dt(&soil_pin, GPIO_INPUT);
    // gpio_pin_configure_dt(&air_pin, GPIO_INPUT);

    // Enable interrupts
    gpio_pin_interrupt_configure_dt(&soil_pin, GPIO_INT_EDGE_FALLING);
    // gpio_pin_interrupt_configure_dt(&air_pin, GPIO_INT_LEVEL_LOW);

    gpio_init_callback(&soil_cb_data, response_isr, BIT(soil_pin.pin));
    gpio_add_callback(soil_pin.port, &soil_cb_data);

    // gpio_init_callback(&air_cb_data, response_isr, BIT(air_pin.pin));
    // gpio_add_callback(&air_pin.port, &air_cb_data);

    while(1){
        k_sem_take(&sem, K_FOREVER);
        printk("\nInterrupt");
        k_mutex_lock(&mutex, K_FOREVER);
        moisture_flag = 1;
        k_mutex_unlock(&mutex);
    }

}

void blinky_task(){
    bool f;
    gpio_pin_configure_dt(&soil_led, GPIO_OUTPUT);
    // gpio_pin_configure_dt(&air_led, GPIO_OUTPUT_LOW);

    while (1)
    {
        k_mutex_lock(&mutex, K_FOREVER);
        f = moisture_flag;
        k_mutex_unlock(&mutex);
        if(f){
            printk("\ntoggle led");
            gpio_pin_toggle_dt(&soil_led);
        }
        else{
            printk("\noff led");
            gpio_pin_set_dt(&soil_led,0);
        } 
        k_msleep(500);  
    }

}

K_THREAD_DEFINE(thread1_id, STACK_SIZE, main_task, NULL, NULL, NULL, 5, 0, 0);
K_THREAD_DEFINE(thread2_id, STACK_SIZE, blinky_task, NULL, NULL, NULL, 5, 0, 0);
K_THREAD_DEFINE(thread3_id, STACK_SIZE, trigger_task, NULL, NULL, NULL, 5, 0, 0);






// humidity = (int32_t)((int32_t)i2c_rx_data[4] << 24) | ((int32_t)i2c_rx_data[5] << 16) | ((int16_t)i2c_rx_data[6] << 8) | (i2c_rx_data[7]);
// printk("\nHumidity = %d.%06d",(humidity / 1000),((humidity % 1000)*1000));