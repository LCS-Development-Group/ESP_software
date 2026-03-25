#include "header.h"

void task_visual_main(void *args)
{
    xEventGroupWaitBits(main_event_group, TASK_START_SYNCBIT, pdFALSE, pdFALSE, portMAX_DELAY);
    if(DEBUG_TASK_ANOUNCE) ESP_LOGI("VIS", "task_visual started");
    
    uint8_t ntcode=0x00;
    while(true)
    {
        if(xQueueReceive(task_queue_list[VIS_TASKID], &ntcode, portMAX_DELAY)==pdPASS)
        {
            continue;//DEBUG
            // switch(ntcode)
            // {   
            //     case VIS_NTCODE_REDRAW_SELECT:
            //         vis->draw_select();
            //         break;

            //     case VIS_NTCODE_REDRAW_BAR:
            //         vis->draw_bar();
            //         break;
                
            //     case VIS_NTCODE_REDRAW_VALUE:
            //         vis->draw_bar();
            //         vis->draw_current_value();
            //         break;

            //     case VIS_NTCODE_REDRAW_ALL_VALUES:
            //         vis->draw_bar();
            //         vis->draw_all_values();
            //         break;

            //     case VIS_NTCODE_REDRAW_ALL:
            //         vis->draw_page();
            //         vis->draw_select();
            //         vis->draw_all_values();
            //         break;
                
            //     case VIS_NTCODE_REDRAW_VALUE_EDITMODE:
            //         vis->draw_editmode();
            //         vis->draw_bar();
            //         break;

            //     // case VIS_NTCODE_ACTIVATE_SCREENSAVER:
            //     //     ESP_ERROR_CHECK(gpio_set_level(LCD_BL_PIN, !LCD_BL_ON_LVL));//here be PWM
            //     //     ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(lcd_handle, false));
            //     //     break;

            //     // case VIS_NTCODE_DEACTIVATE_SCREENSAVER:;
            //     //     ESP_ERROR_CHECK(gpio_set_level(LCD_BL_PIN, LCD_BL_ON_LVL));//here be PWM
            //     //     ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(lcd_handle, true));
            //     //     break;

            //     default:
            //         ESP_LOGW("Visual", "Woken by unknown ntcode: %d", ntcode);
            //         break;
            // }
        }
    }
    vTaskDelay(portMAX_DELAY); //temporary
}