#include <stdio.h>
#include <stdlib.h>
//#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
//#include "esp_flash.h"
#include "esp_system.h"
#include "esp_log.h" // makra do sygnalizacji LOGx

/*mine*/
#include "driver/gpio.h"
/*======================================================================================*/
/* GENERAL                                                                              */
/*======================================================================================*/
#define TASK_NUM 1
#define ENC_TASKID 0



#define ENC_EVBIT (1<<ENC_TASKID)
#define TASK_START_SYNCBIT (1<<23)
#define APP_MAIN_EVBIT (1<<22)

extern EventGroupHandle_t main_event_group;
extern TaskHandle_t task_handle_list[TASK_NUM];




/*======================================================================================*/
/* UI ROTATIONAL ENCODER                                                                */
/*======================================================================================*/
#define ENC_CLK_PIN 1
#define ENC_DT_PIN 2
#define ENC_SW_PIN 42
#define ENC_PCNT_HIGH 1      //after reaching that many steps a one "big step" is registered
#define ENC_PCNT_LOW (-1)*ENC_PCNT_HIGH
extern uint8_t enc_pos;
void task_encoder_main(void *args);
void enc_gpio_init();
void enc_pnct_init();

/*Notification Codes*/
#define ENC_NTCODE_ROT 1
#define ENC_NTCODE_SW 2



void task_test(void *args);


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