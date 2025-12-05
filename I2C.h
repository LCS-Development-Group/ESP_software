#pragma once
#include <i2cdev.h>

#define I2C_BUS_NUMOF   2
struct t_i2c_bus
{
    gpio_num_t SDA_pin;
    gpio_num_t SCL_pin;
    SemaphoreHandle_t mutex;
};
extern t_i2c_bus i2c_bus[I2C_BUS_NUMOF];

#define I2C0_PORT               I2C_NUM_0
#define I2C0_SDA_PIN            GPIO_NUM_1
#define I2C0_SCL_PIN            GPIO_NUM_2

#define I2C1_PORT               I2C_NUM_1
#define I2C1_SDA_PIN            GPIO_NUM_40
#define I2C1_SCL_PIN            GPIO_NUM_39