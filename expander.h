#pragma once
#include "common_includes.h"
#include <mcp23x17.h>
#include "I2C.h"

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

#define TP0_PIN     GPIO_EXP_NUM_B3
#define TP1_PIN     GPIO_EXP_NUM_B2
#define TP2_PIN     GPIO_EXP_NUM_B0
#define TP3_PIN     GPIO_EXP_NUM_A6
#define TP4_PIN     GPIO_EXP_NUM_A5
#define TP5_PIN     GPIO_EXP_NUM_A4

#define EXP_FREQ_HZ         400000
#define EXP_ADDR            0x20
#define EXP_GPIO_DEF_LVL    0
//Port 1 -> I2C.h

extern mcp23x17_t exp_dev;
void exp_init();
void exp_set_pin(uint8_t pin, bool lvl);