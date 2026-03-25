#include "header.h"
#define GUI_RETCODE_DEFAULT 0

SemaphoreHandle_t gui_mutex;
lv_color_t lvgl_buffer[LCD_WIDTH*LCD_BUF_LINES];
lv_display_t *display;

void gui2_flush_display(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    uint32_t w=lv_area_get_width(area);
    uint32_t h=lv_area_get_height(area);
    lv_draw_sw_rgb565_swap(px_map, w*h);

    esp_lcd_panel_draw_bitmap(lcd_handle,
        area->x1,
        area->y1,
        area->x2+1,
        area->y2+1,
        px_map);
}

void gui2_init()
{
    lv_init();
    display=lv_display_create(LCD_WIDTH, LCD_HEIGHT);
    lv_display_set_buffers(display, lvgl_buffer, NULL, sizeof(lvgl_buffer), LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_color_format(display, LV_COLOR_FORMAT_RGB565);
    lv_display_set_flush_cb(display, gui2_flush_display);

    const esp_lcd_panel_io_callbacks_t panel_cb={
        .on_color_trans_done=lcd_flushed_isr,
    };
    ESP_ERROR_CHECK(esp_lcd_panel_io_register_event_callbacks(lcd_io_handle, &panel_cb, display));
}

void task_gui_main(void *args)
{
    gui2_init();
    lv_obj_t *screen=lv_screen_active();
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x000000), 0);

    lv_obj_t *sw=lv_switch_create(screen);
    lv_obj_set_style_bg_color(sw, lv_palette_main(LV_PALETTE_GREY), LV_PART_MAIN);
    lv_obj_set_style_bg_color(sw, lv_palette_main(LV_PALETTE_GREEN), LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(sw, lv_color_hex(0xFFFFFF), LV_PART_KNOB);

    lv_obj_add_state(sw, LV_STATE_CHECKED);
    lv_obj_set_pos(sw, 0, 0);

    lv_obj_t * label = lv_label_create(screen);
    lv_label_set_text(label, "Simple Test");
    lv_obj_set_style_text_color(label, lv_color_hex(0x0000ff), 0);


    lv_obj_center(label);
    lv_obj_add_state(sw, LV_STATE_CHECKED);
    lv_timer_handler();//update

    vTaskDelay(pdMS_TO_TICKS(2000));
    lv_obj_clear_state(sw, LV_STATE_CHECKED);
    lv_timer_handler();//update

    xEventGroupWaitBits(main_event_group, TASK_START_SYNCBIT, pdFALSE, pdFALSE, portMAX_DELAY);
    if(DEBUG_TASK_ANOUNCE) ESP_LOGI("GUI", "task_gui started");

    uint8_t ntcode=0x00;
    uint8_t sendval=0x00;
    uint8_t retcode=GUI_RETCODE_DEFAULT;
    bool screensaver_lock;

    while(true)
    {
        if(xQueueReceive(task_queue_list[GUI_TASKID], &ntcode, portMAX_DELAY)==pdPASS)
        {
            xSemaphoreTake(lcd_settings.mutex, portMAX_DELAY);
            screensaver_lock=lcd_settings.ss_state;
            xSemaphoreGive(lcd_settings.mutex);

            sendval=LCD_NTCODE_DEACTIVATE_SCREENSAVER;
            xQueueSend(task_queue_list[LCD_TASKID], &sendval, 0);

            if(screensaver_lock) continue;//

            xSemaphoreTake(gui_mutex, portMAX_DELAY);
            switch(ntcode)
            {

                case GUI_NTCODE_CUR_NEG:
                    //ESP_LOGI("GUI", "UP");
                    //retcode=gui->move_cursor_down();
                    break;

                case GUI_NTCODE_CUR_POS:
                    //ESP_LOGI("GUI", "DOWN");
                    //retcode=gui->move_cursor_up();
                    break;

                case GUI_NTCODE_CUR_ENT:
                    //ESP_LOGI("GUI", "ENTER");
                    //retcode=gui->enter();
                    break;

                case GUI_NTCODE_CUR_BCK:
                    //retcode=gui->go_back();
                    //ESP_LOGI("GUI", "BACK");
                    break;

                default:
                    ESP_LOGW("GUI", "Woken by unknown ntcode: %d", ntcode);
                    break;
            }
            xSemaphoreGive(gui_mutex);

            if(retcode!=GUI_RETCODE_DEFAULT)
            {
                //xQueueSend(task_queue_list[VIS_TASKID], &retcode, 0);
                retcode=GUI_RETCODE_DEFAULT;
                lv_timer_handler();
            }
        }
    }
}

void gui_init()
{
    // if(gui==nullptr)
    // {
    //     gui=new gui_controller;
    //     if(gui==nullptr)
    //     {
    //         ESP_LOGE("GUI", "gui_controller creation failed");
    //         exit(-1);
    //     }
    // }

    gui_mutex=xSemaphoreCreateMutex();
    if(gui_mutex==nullptr)
    {
        ESP_LOGE("GUI", "mutex creation failed");
        exit(-1);
    }

    //gui->fill_fields();
}