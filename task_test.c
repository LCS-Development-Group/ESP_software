#include "header.h"

void task_test(void *args)
{
    ESP_LOGW("test", "created, going dark\n");
    vTaskDelay(portMAX_DELAY);
}