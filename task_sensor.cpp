#include "header.h"

void task_sensor_main(void *args)
{
    xEventGroupWaitBits(main_event_group, TASK_START_SYNCBIT, pdFALSE, pdFALSE, portMAX_DELAY);
    ESP_LOGI("SEN", "task_sensor started");

    vTaskDelay(portMAX_DELAY);//temporary
}