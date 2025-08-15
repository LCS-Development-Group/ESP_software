#include <stdio.h>
#include <stdlib.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
//#include "esp_flash.h"
#include "esp_system.h"
#include "esp_log.h" // makra do sygnalizacji LOGx

/*mine*/
#include "driver/gpio.h"
#include "driver/spi_master.h"


/*======================================================================================*/
/* GENERAL                                                                              */
/*======================================================================================*/
#define TASK_NUM 3
#define ENC_TASKID 0
#define CON_TASKID 1
#define VIS_TASKID 2

#define ENC_EVBIT (1<<ENC_TASKID)
#define CON_EVBIT (1<<CON_TASKID)
#define VIS_EVBIT (1<<VIS_TASKID)

#define TASK_START_SYNCBIT (1<<23)
#define APP_MAIN_EVBIT (1<<22)

extern EventGroupHandle_t main_event_group;
extern TaskHandle_t task_handle_list[TASK_NUM];

/*======================================================================================*/
/* Controller                                                                           */
/*======================================================================================*/
void task_controller_main(void *args);

/*======================================================================================*/
/* Visual                                                                               */
/*======================================================================================*/
void task_visual_main(void *args);
#define LCD_HOST                SPI2_HOST
#define LCD_CLOCK_HZ            (10 * 1000 * 1000) //10MHz
#define LCD_BL_ON_LVL           1
#define LCD_BL_OFF_LVL          !LCD_BL_ON_LVL //necessery?
#define LCD_DIN_PIN             9 //din
#define LCD_CLK_PIN             10 //clk
#define LCD_CS_PIN              11 //cs
#define LCD_DC_PIN              12 //dc
#define LCD_RST_PIN             13 //rst
#define LCD_BL_PIN              14 //bl
#define LCD_HRES                240 
#define LCD_VRES                320
#define LCD_BITS_PX             16

#define EXAMPLE_LCD_CMD_BITS           8
#define EXAMPLE_LCD_PARAM_BITS         8

#define EXAMPLE_LVGL_DRAW_BUF_LINES    20 // number of display lines in each draw buffer
#define EXAMPLE_LVGL_TICK_PERIOD_MS    2
#define EXAMPLE_LVGL_TASK_MAX_DELAY_MS 500
#define EXAMPLE_LVGL_TASK_MIN_DELAY_MS 1000 / CONFIG_FREERTOS_HZ
#define EXAMPLE_LVGL_TASK_STACK_SIZE   (4 * 1024)
#define EXAMPLE_LVGL_TASK_PRIORITY     2

void vis_connect_init();

/*======================================================================================*/
/* UI ROTATIONAL ENCODER                                                                */
/*======================================================================================*/
void task_encoder_main(void *args);
#define ENC_CLK_PIN 1
#define ENC_DT_PIN 2
#define ENC_SW_PIN 42
#define ENC_PCNT_HIGH 1      //after reaching that many steps a one "big step" is registered
#define ENC_PCNT_LOW (-1)*ENC_PCNT_HIGH
extern uint8_t enc_pos;
void enc_gpio_init();
void enc_pnct_init();

/*Notification Codes*/
#define ENC_NTCODE_ROT 1
#define ENC_NTCODE_SW 2



/*======================================================================================*/
/* MISC                                                                                 */
/*======================================================================================*/
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