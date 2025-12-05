#include "header.h"

void task_comm_main(void *args)
{
    xEventGroupWaitBits(main_event_group, TASK_START_SYNCBIT, pdFALSE, pdFALSE, portMAX_DELAY);
    if(DEBUG_TASK_ANOUNCE) ESP_LOGI("COM", "task_comm started");

    vTaskDelay(portMAX_DELAY);//temporary
}