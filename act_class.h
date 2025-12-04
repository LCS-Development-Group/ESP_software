#pragma once
#include "common_includes.h"
#include <iot_servo.h>

enum t_expander{MCU, EXPANDER};

struct t_basic_actuator
{
    bool enabled;
    SemaphoreHandle_t mutex;
    bool expander;              //steered via expander (true) or directly (false)
    uint8_t en_pin;
};

struct t_servo
{
    bool enabled;
    SemaphoreHandle_t mutex;
    uint8_t en_pin;
    bool position;

    float angle[2];
};