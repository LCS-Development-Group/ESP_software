#include "header.h"

void task_actuator_main(void *args)
{
    xEventGroupWaitBits(main_event_group, TASK_START_SYNCBIT, pdFALSE, pdFALSE, portMAX_DELAY);
    ESP_LOGI("ACT", "task_actuator started");

    vTaskDelay(portMAX_DELAY);//temporary
}