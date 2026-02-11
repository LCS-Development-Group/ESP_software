#include "header.h"

void misc_init()
{
    ESP_ERROR_CHECK(i2cdev_init());

    i2c_bus[I2C0_PORT]={
        .SDA_pin=I2C0_SDA_PIN,
        .SCL_pin=I2C0_SCL_PIN,
        .mutex=xSemaphoreCreateMutex(),
    };
    if(i2c_bus[I2C0_PORT].mutex==NULL) 
    {
        ESP_LOGE("Main", "i2c%d_mutex creation failed", I2C0_PORT);
        exit(-1);
    }

    i2c_bus[I2C1_PORT]={
        .SDA_pin=I2C1_SDA_PIN,
        .SCL_pin=I2C1_SCL_PIN,
        .mutex=xSemaphoreCreateMutex(),
    };
    if(i2c_bus[I2C1_PORT].mutex==NULL) 
    {
        ESP_LOGE("Main", "i2c%d_mutex creation failed", I2C1_PORT);
        exit(-1);
    }
}

void task_create_fail(uint8_t taskid)
{
    ESP_LOGE("Setup", "Task id=%d creation failed\n", taskid);
    exit(-1);
}