#include "header.h"
#include "000_gui.h"

lv_color_t lvgl_buffer[LCD_WIDTH*LCD_BUF_LINES];
lv_display_t *display;
gui_controller_t *gui;
lv_style_t *list_style_def;
lv_style_t *list_style_sel;


void task_gui_main(void *args)
{
    xEventGroupWaitBits(main_event_group, TASK_START_SYNCBIT, pdFALSE, pdFALSE, portMAX_DELAY);
    if(DEBUG_TASK_ANOUNCE) ESP_LOGI("GUI", "task_gui started");

    uint8_t ntcode=0x00;
    uint8_t sendval=0x00;
    bool screensaver_lock=false, update_scheduled=true;

    while(true)
    {
        if(xQueueReceive(task_queue_list[GUI_TASKID], &ntcode, portMAX_DELAY)==pdPASS)
        {
            xSemaphoreTake(lcd_settings.mutex, portMAX_DELAY);
            screensaver_lock=lcd_settings.ss_state;
            xSemaphoreGive(lcd_settings.mutex);


            sendval=LCD_NTCODE_DEACTIVATE_SCREENSAVER;
            xQueueSend(task_queue_list[LCD_TASKID], &sendval, 0);
            if(screensaver_lock) 
            {
                continue;//
                if(!update_scheduled && ntcode==GUI_NTCODE_UPDATE_PAGE)  update_scheduled=true;
            }

            

            xSemaphoreTake(gui->get_mutex(), portMAX_DELAY);
            if(update_scheduled)
            {
                gui->cmd_update_page();
                update_scheduled=false;
            }
            
            switch(ntcode)
            {
                case GUI_NTCODE_UPDATE_PAGE:
                    gui->cmd_update_page();
                    break;

                case GUI_NTCODE_CUR_NEG:
                    gui->cmd_next();
                    break;

                case GUI_NTCODE_CUR_POS:
                    gui->cmd_prev();
                    break;

                case GUI_NTCODE_CUR_ENT:
                    gui->cmd_enter();
                    break;

                case GUI_NTCODE_CUR_BCK:
                    gui->cmd_back();
                    break;

                default:
                    ESP_LOGW("GUI", "Woken by unknown ntcode: %d", ntcode);
                    break;
            }
            xSemaphoreGive(gui->get_mutex());
            lv_refr_now(display);
        }
    }
}

void gui_flush_display(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map);
void gui_setup_global_styles();
void gui_init()
{
    lv_init();
    display=lv_display_create(LCD_WIDTH, LCD_HEIGHT);
    lv_display_set_buffers(display, lvgl_buffer, NULL, sizeof(lvgl_buffer), LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_color_format(display, LV_COLOR_FORMAT_RGB565);
    lv_display_set_flush_cb(display, gui_flush_display);

    const esp_lcd_panel_io_callbacks_t panel_cb={
        .on_color_trans_done=lcd_flushed_isr,
    };
    ESP_ERROR_CHECK(esp_lcd_panel_io_register_event_callbacks(lcd_io_handle, &panel_cb, display));

    //styles
    gui_setup_global_styles();

    gui=new gui_controller_t;
    if(gui==nullptr)
    {
        ESP_LOGE("GUI", "GUI controller creation fail");
        exit(-1);
    }
    if(SETTING_GUI_START_FROM_MAIN) gui->cmd_enter();
    lv_refr_now(display);
}

void gui_setup_global_styles()
{
    //menu list default
    list_style_def=new lv_style_t;
    lv_style_init(list_style_def);
    lv_style_set_bg_color(list_style_def, GUI_COLOR_TILE_BG);
    lv_style_set_bg_opa(list_style_def, LV_OPA_COVER);
    lv_style_set_radius(list_style_def, GUI_TILE_CORNER_RADIUS);
    lv_style_set_pad_all(list_style_def, 6);
    lv_style_set_text_color(list_style_def, GUI_COLOR_TEXT);

    //menu list selected
    list_style_sel=new lv_style_t;
    lv_style_init(list_style_sel);
    lv_style_set_bg_color(list_style_sel, GUI_COLOR_SELECT);
    lv_style_set_bg_opa(list_style_sel, LV_OPA_COVER);
    lv_style_set_radius(list_style_sel, GUI_TILE_CORNER_RADIUS);
    lv_style_set_pad_all(list_style_sel, 6);
    lv_style_set_text_color(list_style_sel, GUI_COLOR_TEXT);
}

void gui_flush_display(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
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