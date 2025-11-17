#pragma once
#include "common_includes.h"
#include "driver/ledc.h"

#define GPIO_EXP_NUM_A0 0
#define GPIO_EXP_NUM_A1 1
#define GPIO_EXP_NUM_A2 2
#define GPIO_EXP_NUM_A3 3
#define GPIO_EXP_NUM_A4 4
#define GPIO_EXP_NUM_A5 5
#define GPIO_EXP_NUM_A6 6
#define GPIO_EXP_NUM_A7 7

#define GPIO_EXP_NUM_B0 8
#define GPIO_EXP_NUM_B1 9
#define GPIO_EXP_NUM_B2 10
#define GPIO_EXP_NUM_B3 11
#define GPIO_EXP_NUM_B4 12
#define GPIO_EXP_NUM_B5 13
#define GPIO_EXP_NUM_B6 14
#define GPIO_EXP_NUM_B7 15


struct t_basic_actuator
{
    bool enabled;
    SemaphoreHandle_t mutex;
    bool expander;              //steered via expander (true) or directly (false)
    uint8_t en_pin;
    gpio_config_t pin_en_cfg;
};

struct t_servo: public t_basic_actuator
{
    bool postion;
    uint32_t pos_duty_cycle[2];
    uint8_t pwm_pin;
    ledc_channel_t pwm_chan;
    ledc_channel_config_t pwm_chan_cfg;

    void update_pos_duty_cycle(float angle, bool _position);
};
