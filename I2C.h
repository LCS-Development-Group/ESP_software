#pragma once
#include <i2cdev.h>

extern SemaphoreHandle_t i2c0_mutex;
#define I2C0_PORT               I2C_NUM_0
#define I2C0_SDA_PIN            GPIO_NUM_1
#define I2C0_SCL_PIN            GPIO_NUM_2

extern SemaphoreHandle_t i2c1_mutex;
#define I2C1_PORT               I2C_NUM_1
#define I2C1_SDA_PIN            GPIO_NUM_40
#define I2C1_SCL_PIN            GPIO_NUM_39