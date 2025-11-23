#include "header.h"

//DEBUG
bool DEBUG_BOOL=false;
SemaphoreHandle_t DEBUG_BOOL_MUT=xSemaphoreCreateMutex();

float DEBUG_FLOAT=420.2137;
SemaphoreHandle_t DEBUG_FLOAT_MUT=xSemaphoreCreateMutex();

float DEBUG_FLOAT_2=15.3;
SemaphoreHandle_t DEBUG_FLOAT_2_MUT=xSemaphoreCreateMutex();


TaskHandle_t task_handle_list[TASK_NUM];
EventGroupHandle_t main_event_group;

SemaphoreHandle_t i2c0_mutex;
SemaphoreHandle_t i2c1_mutex;

void task_create_fail(uint8_t taskid)
{
    ESP_LOGE("Setup", "Task id=%d creation failed\n", taskid);
    exit(-1);
}

extern "C" void app_main(void)
{
    misc_init();
    exp_init();

    enc_gpio_init();
    enc_pnct_init();
    gui_init();
    vis_init();
    act_init();

    exp_set_pin(TP2_PIN, 1);

    main_event_group=xEventGroupCreate();
    if(main_event_group==NULL)
    {
        ESP_LOGE("Setup", "main_event_group creation failed\n");
        exit(-1);
    }

    if(xTaskCreate(task_encoder_main, "task_encoder", 2048, NULL, 1, &task_handle_list[ENC_TASKID])!=pdPASS) task_create_fail(ENC_TASKID);
    if(xTaskCreate(task_gui_main, "task_gui", 2048, NULL, 1, &task_handle_list[GUI_TASKID])!=pdPASS) task_create_fail(GUI_TASKID);
    if(xTaskCreate(task_visual_main, "task_visual", 8192, NULL, 1, &task_handle_list[VIS_TASKID])!=pdPASS) task_create_fail(VIS_TASKID);
    if(xTaskCreate(task_sensor_main, "task_sensor", 2048, NULL, 1, &task_handle_list[SEN_TASKID])!=pdPASS) task_create_fail(SEN_TASKID);
    if(xTaskCreate(task_actuator_main, "task_actuator", 2048, NULL, 1, &task_handle_list[ACT_TASKID])!=pdPASS) task_create_fail(ACT_TASKID);
    if(xTaskCreate(task_regulator_main, "task_regulator", 2048, NULL, 1, &task_handle_list[REG_TASKID])!=pdPASS) task_create_fail(REG_TASKID);
    if(xTaskCreate(task_comm_main, "task_comm", 2048, NULL, 1, &task_handle_list[COM_TASKID])!=pdPASS) task_create_fail(COM_TASKID);
    
    vTaskDelay(pdMS_TO_TICKS(100));
    xEventGroupSetBits(main_event_group, TASK_START_SYNCBIT);//start the tasks
    vTaskDelay(pdMS_TO_TICKS(100));
    xEventGroupClearBits(main_event_group, TASK_START_SYNCBIT);//safely clear the syncbit

    xEventGroupWaitBits(main_event_group, APP_MAIN_EVBIT, pdTRUE, pdFALSE, portMAX_DELAY);
    ESP_LOGW("Main", "app_main awoken\n");    
}

void misc_init()
{
    ESP_ERROR_CHECK(i2cdev_init());
    i2c0_mutex=xSemaphoreCreateMutex();
    if(i2c0_mutex==NULL) 
    {
        ESP_LOGE("Main", "i2c0_mutex creation failed");
        exit(-1);
    }

    i2c1_mutex=xSemaphoreCreateMutex();
    if(i2c1_mutex==NULL) 
    {
        ESP_LOGE("Main", "i2c1_mutex creation failed");
        exit(-1);
    }
}
