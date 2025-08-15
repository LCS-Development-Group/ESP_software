#include "header.h"

void task_controller_main(void *args)
{
    xEventGroupWaitBits(main_event_group, TASK_START_SYNCBIT, pdFALSE, pdFALSE, portMAX_DELAY);
    ESP_LOGI("Controller", "task_controller started");

    vTaskDelay(portMAX_DELAY); //temporary
}