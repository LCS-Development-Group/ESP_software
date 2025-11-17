#include "header.h"

void task_regulator_main(void *args)
{
    xEventGroupWaitBits(main_event_group, TASK_START_SYNCBIT, pdFALSE, pdFALSE, portMAX_DELAY);
    ESP_LOGI("REG", "task_regulator started");

    vTaskDelay(portMAX_DELAY);//temporary
}