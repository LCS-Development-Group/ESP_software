#pragma once
#include "common_includes.h"
#include "gui_class.h"
#include "vis_class.h"
#include "act_class.h"

/*======================================================================================*/
/* GENERAL                                                                              */
/*======================================================================================*/
#define TASK_NUM 7
#define ENC_TASKID 0
#define GUI_TASKID 1
#define VIS_TASKID 2
#define SEN_TASKID 3
#define ACT_TASKID 4
#define REG_TASKID 5
#define COM_TASKID 6

/*planned to use evbits - might not*/
#define ENC_EVBIT (1<<ENC_TASKID)
#define GUI_EVBIT (1<<GUI_TASKID)
#define VIS_EVBIT (1<<VIS_TASKID)
#define SEN_EVBIT (1<<SEN_TASKID)
#define ACT_EVBIT (1<<ACT_TASKID)
#define REG_EVBIT (1<<REG_TASKID)
#define COM_EVBIT (1<<COM_TASKID)

#define TASK_START_SYNCBIT (1<<23)
#define APP_MAIN_EVBIT (1<<22)

extern EventGroupHandle_t main_event_group;
extern TaskHandle_t task_handle_list[TASK_NUM];

/*======================================================================================*/
/* Actuator                                                                             */
/*======================================================================================*/
void task_actuator_main(void *args);

//membrana
extern t_basic_actuator act_membrane;
#define ACT_MEMB_EN_PIN         GPIO_NUM_42
#define ACT_MEMB_EN_LVL         0
#define ACT_MEMB_DIS_LVL        !(ACT_MEMB_EN_LVL)

//serwa
#define ACT_SERV0_EN_PIN        GPIO_EXP_NUMB4
#define ACT_SERV1_EN_PIN        GPIO_EXP_NUMB5
#define ACT_SERV2_EN_PIN        GPIO_EXP_NUMB6
#define ACT_SERV3_EN_PIN        GPIO_EXP_NUMB7

#define ACT_SERV0_PWM_PIN       GPIO_NUM_4
#define ACT_SERV1_PWM_PIN       GPIO_NUM_5
#define ACT_SERV2_PWM_PIN       GPIO_NUM_6
#define ACT_SERV3_PWM_PIN       GPIO_NUM_7

#define ACT_SERV_PWM_FREQ_HZ    50
#define ACT_SERV_PWM_TIMER      LEDC_TIMER_0
#define ACT_SERV_PWM_

void act_init();

/*======================================================================================*/
/* Regulator                                                                            */
/*======================================================================================*/
void task_regulator_main(void *args);

/*======================================================================================*/
/* Sensor                                                                               */
/*======================================================================================*/
void task_sensor_main(void *args);

/*======================================================================================*/
/* Comm                                                                                 */
/*======================================================================================*/
void task_comm_main(void *args);

/*======================================================================================*/
/* Visual                                                                               */
/*======================================================================================*/
void task_visual_main(void *args);

void vis_init();

#define LCD_HOST                SPI2_HOST
#define LCD_CLOCK_HZ            (10 * 1000 * 1000) //10MHz
#define LCD_BL_ON_LVL           1
#define LCD_BL_OFF_LVL          !LCD_BL_ON_LVL //necessery?
#define LCD_DIN_PIN             GPIO_NUM_10 //din
#define LCD_CLK_PIN             GPIO_NUM_9 //clk
#define LCD_CS_PIN              GPIO_NUM_8 //cs
#define LCD_DC_PIN              GPIO_NUM_18 //dc
#define LCD_RST_PIN             GPIO_NUM_17 //rst
#define LCD_BL_PIN              GPIO_NUM_16 //bl
#define LCD_HRES                240 
#define LCD_VRES                320
#define LCD_NUM_PX              (LCD_HRES*LCD_VRES)
#define LCD_BITS_PX             16
#define LCD_CMD_BITS            8
#define LCD_PARAM_BITS          8

/*Content stuff*/
//in vis_class.h

/*Notification Codes*/
#define VIS_NTCODE_REDRAW_ALL               0
#define VIS_NTCODE_REDRAW_SELECT            1
#define VIS_NTCODE_REDRAW_BAR               2
#define VIS_NTCODE_REDRAW_VALUE             3
#define VIS_NTCODE_REDRAW_ALL_VALUES        4
#define VIS_NTCODE_REDRAW_VALUE_EDITMODE    5

/*======================================================================================*/
/* UI ROTATIONAL ENCODER                                                                */
/*======================================================================================*/
void task_encoder_main(void *args);
#define ENC_CLK_PIN                 GPIO_NUM_14
#define ENC_DT_PIN                  GPIO_NUM_13
#define ENC_SW_PIN                  GPIO_NUM_12
#define ENC_PCNT_HIGH               1      //after reaching that many steps a one "big step" is registered
#define ENC_PCNT_LOW                (-1)*ENC_PCNT_HIGH
void enc_gpio_init();
void enc_pnct_init();

/*Notification Codes*/
#define ENC_NTCODE_ROT              0
#define ENC_NTCODE_SW_CLICK         1
#define ENC_NTCODE_SW_PRESSED       2

/*======================================================================================*/
/* GUI                                                                                  */
/*======================================================================================*/
void task_gui_main(void *args);

extern gui_controller *gui;
extern SemaphoreHandle_t gui_mutex;

void gui_init();

/*Notification Codes*/
#define GUI_NTCODE_CUR_POS      0
#define GUI_NTCODE_CUR_NEG      1
#define GUI_NTCODE_CUR_ENT      2
#define GUI_NTCODE_CUR_BCK      3

#define GUI_CURSOR_MAX_INDEX    255

/*======================================================================================*/
/* MISC                                                                                 */
/*======================================================================================*/

extern bool DEBUG_BOOL;
extern SemaphoreHandle_t DEBUG_BOOL_MUT;

extern float DEBUG_FLOAT;
extern SemaphoreHandle_t DEBUG_FLOAT_MUT;

extern float DEBUG_FLOAT_2;
extern SemaphoreHandle_t DEBUG_FLOAT_2_MUT;


// #define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
// #define BYTE_TO_BINARY(byte) 
//     ((byte) & 0x80 ? '1' : '0'), 
//     ((byte) & 0x40 ? '1' : '0'), 
//     ((byte) & 0x20 ? '1' : '0'), 
//     ((byte) & 0x10 ? '1' : '0'), 
//     ((byte) & 0x08 ? '1' : '0'), 
//     ((byte) & 0x04 ? '1' : '0'), 
//     ((byte) & 0x02 ? '1' : '0'), 
//     ((byte) & 0x01 ? '1' : '0')
