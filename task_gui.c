#include "header.h"

void task_gui_main(void *args)
{
    xEventGroupWaitBits(main_event_group, TASK_START_SYNCBIT, pdFALSE, pdFALSE, portMAX_DELAY);
    ESP_LOGI("Gui", "task_gui started");
    
    vTaskDelay(portMAX_DELAY); //temporary
}


// gui_main_menu=
// {{.} }