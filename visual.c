#include "header.h"

void task_visual_main(void *args)
{
    xEventGroupWaitBits(main_event_group, TASK_START_SYNCBIT, pdFALSE, pdFALSE, portMAX_DELAY);
    ESP_LOGI("Visual", "task_visual started");

    vTaskDelay(portMAX_DELAY); //temporary
}