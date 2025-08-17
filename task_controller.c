#include "header.h"

t_page page_list[PAGE_NUM];

void task_controller_main(void *args)
{
    xEventGroupWaitBits(main_event_group, TASK_START_SYNCBIT, pdFALSE, pdFALSE, portMAX_DELAY);
    ESP_LOGI("Controller", "task_controller started");

    vTaskDelay(portMAX_DELAY); //temporary
}


void init_page_info()
{
    page_list[PAGE_INFO_NUMBER].field_list[0].field_type=FT_RETURN;
    page_list[PAGE_INFO_NUMBER].field_list[1].field_type=FT_OUTPUT;
    page_list[PAGE_INFO_NUMBER].field_list[2].field_type=FT_INPUT;
    page_list[PAGE_INFO_NUMBER].field_list[3].field_type=FT_TOGGLE;
    page_list[PAGE_INFO_NUMBER].field_list[4].field_type=FT_DISABLE;
    page_list[PAGE_INFO_NUMBER].field_list[5].field_type=FT_DISABLE;
    page_list[PAGE_INFO_NUMBER].field_list[6].field_type=FT_DISABLE;
    page_list[PAGE_INFO_NUMBER].field_list[7].field_type=FT_DISABLE;
    page_list[PAGE_INFO_NUMBER].field_list[8].field_type=FT_DISABLE;
    page_list[PAGE_INFO_NUMBER].field_list[9].field_type=FT_DISABLE;
}


void init_pages()
{
    init_page_info();
}