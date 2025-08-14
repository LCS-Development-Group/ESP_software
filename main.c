#include "header.h"

TaskHandle_t task_handle_list[TASK_NUM];
uint8_t task_signal_flag=0;
EventGroupHandle_t main_event_group;

void task_create_fail(uint8_t taskid)
{
    ESP_LOGE("Setup", "Task id=%d creation failed\n", taskid);
    exit(-1);
}

void gpio_init()
{
    enc_gpio_init();
}

void app_main(void)
{
    gpio_init();   
    enc_pnct_init();




    main_event_group=xEventGroupCreate();
    if(main_event_group==NULL)
    {
        ESP_LOGE("Setup", "main_event_group creation failed\n");
        exit(-1);
    }

    if(xTaskCreate(task_encoder_main, "task_encoder", 2048, NULL, 1, &task_handle_list[ENC_TASKID])!=pdPASS) task_create_fail(ENC_TASKID);
    
    vTaskDelay(pdMS_TO_TICKS(100));
    xEventGroupSetBits(main_event_group, TASK_START_SYNCBIT);//start the tasks
    vTaskDelay(pdMS_TO_TICKS(100));
    xEventGroupClearBits(main_event_group, TASK_START_SYNCBIT);//safely clear the syncbit

    xEventGroupWaitBits(main_event_group, APP_MAIN_EVBIT, pdTRUE, pdFALSE, portMAX_DELAY);
    ESP_LOGW("Setup", "app_main awoken\n");    
}


