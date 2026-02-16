#include "header.h"
#include "vis_class.h"
#include <sstream>
#include <iomanip>

vis_controller *vis;

void task_visual_main(void *args)
{
    xEventGroupWaitBits(main_event_group, TASK_START_SYNCBIT, pdFALSE, pdFALSE, portMAX_DELAY);
    if(DEBUG_TASK_ANOUNCE) ESP_LOGI("VIS", "task_visual started");

    vis->start();
    
    uint8_t ntcode=0x00;
    while(true)
    {
        if(xQueueReceive(task_queue_list[VIS_TASKID], &ntcode, portMAX_DELAY)==pdPASS)
        {
            switch(ntcode)
            {   
                case VIS_NTCODE_REDRAW_SELECT:
                    vis->draw_select();
                    break;

                case VIS_NTCODE_REDRAW_BAR:
                    vis->draw_bar();
                    break;
                
                case VIS_NTCODE_REDRAW_VALUE:
                    vis->draw_bar();
                    vis->draw_current_value();
                    break;

                case VIS_NTCODE_REDRAW_ALL_VALUES:
                    vis->draw_bar();
                    vis->draw_all_values();
                    break;

                case VIS_NTCODE_REDRAW_ALL:
                    vis->draw_page();
                    vis->draw_select();
                    vis->draw_all_values();
                    break;
                
                case VIS_NTCODE_REDRAW_VALUE_EDITMODE:
                    vis->draw_editmode();
                    vis->draw_bar();
                    break;

                // case VIS_NTCODE_ACTIVATE_SCREENSAVER:
                //     ESP_ERROR_CHECK(gpio_set_level(LCD_BL_PIN, !LCD_BL_ON_LVL));//here be PWM
                //     ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(lcd_handle, false));
                //     break;

                // case VIS_NTCODE_DEACTIVATE_SCREENSAVER:;
                //     ESP_ERROR_CHECK(gpio_set_level(LCD_BL_PIN, LCD_BL_ON_LVL));//here be PWM
                //     ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(lcd_handle, true));
                //     break;

                default:
                    ESP_LOGW("Visual", "Woken by unknown ntcode: %d", ntcode);
                    break;
            }
        }
    }
    vTaskDelay(portMAX_DELAY); //temporary
}

/*vis_class methods definitions*/
vis_controller::vis_controller(esp_lcd_panel_handle_t* _lcd_handle)
:lcd_handle(_lcd_handle)
{
    if(lcd_handle==nullptr)
    {
        ESP_LOGE("VIS", "vis_controller initialized with nullptr pointer");
        exit(-1);
    }
}

void vis_controller::draw_page()
{
    clear();
    xSemaphoreTake(gui_mutex, portMAX_DELAY);
    draw_text(gui->get_current_page()->get_page_name()+":", 0, 0);//Page name    
    vis->draw_names(); //field names
    xSemaphoreGive(gui_mutex);
}

void vis_controller::draw_current_value()
{
    xSemaphoreTake(gui_mutex, portMAX_DELAY);
    clear_field(gui->get_prim_idx());
    draw_value(gui->get_prim_idx());
    xSemaphoreGive(gui_mutex);
}   

void vis_controller::draw_all_values()
{
    xSemaphoreTake(gui_mutex, portMAX_DELAY);
    for(uint8_t idx=0; idx<gui->get_current_page()->get_numof_fields(); idx++)
    {
        //clear_field(idx);
        draw_value(idx);
    }

    xSemaphoreGive(gui_mutex);
}

void vis_controller::draw_select()
{
    xSemaphoreTake(gui_mutex, portMAX_DELAY);
    uint8_t idx=gui->get_prim_idx();
    uint8_t prev_idx=gui->get_prev_prim_idx();
    xSemaphoreGive(gui_mutex);

    if(idx==GUI_CURSOR_MAX_INDEX) return;

    if(prev_idx!=GUI_CURSOR_MAX_INDEX) draw_text(" ", prev_idx+1, 0);
    draw_text("}", idx+1, 0);
}

void vis_controller::draw_bar()
{
    xSemaphoreTake(gui_mutex, portMAX_DELAY);

    uint8_t row, col;

    col=gui->get_prev_sec_idx();
    row=gui->get_prim_idx();
    if(col!=GUI_CURSOR_MAX_INDEX) draw_bar_at(row, col, VIS_SELECTBAR_NEG);

    col=gui->get_sec_idx();
    if(col!=GUI_CURSOR_MAX_INDEX)
    {
        if(gui->get_sec_lock()==true) draw_bar_at(row, col, VIS_SELECTBAR_NORM);
        else draw_bar_at(row, col, VIS_SELECTBAR_NSEL);
    }
    xSemaphoreGive(gui_mutex);
}

void vis_controller::start()
{
    clear();
    vTaskDelay(pdMS_TO_TICKS(100));

    draw_page();
    draw_select();
    draw_all_values();
    vTaskDelay(pdMS_TO_TICKS(100));
}

void vis_controller::clear() 
{
    for(uint16_t row=0; row<LCD_MAX_LINES; row++)
    for(uint16_t col=0; col<LCD_MAX_CHARS_PER_LINE; col++)
    {
        //higher processor usage but saving like 150kB
        esp_lcd_panel_draw_bitmap(*lcd_handle, col*VIS_FONT_W, row*VIS_FONT_H, (col+1)*VIS_FONT_W, (row+1)*VIS_FONT_H, font[VIS_FONT_INDEX_SPACE]);
    }
}

void vis_controller::draw_editmode()
{
    xSemaphoreTake(gui_mutex, portMAX_DELAY);
    uint8_t current=gui->get_prim_idx();
    for(uint8_t idx=0; idx<gui->get_current_page()->get_numof_fields(); idx++)
    {
        if(current==idx) draw_value(idx);
        else clear_field(idx);
    }
    xSemaphoreGive(gui_mutex);
}

void vis_controller::clear_field(uint8_t line)
{
    for(uint16_t col=LCD_FIELD_VALUE_START; col<LCD_MAX_CHARS_PER_LINE; col++)
        esp_lcd_panel_draw_bitmap(*lcd_handle, col*VIS_FONT_W, (line+1)*VIS_FONT_H, (col+1)*VIS_FONT_W, (line+2)*VIS_FONT_H, font[VIS_FONT_INDEX_SPACE]);
}

void vis_controller::draw_value(uint8_t line)
{
    /*Expecting mutex already taken*/
    switch(gui->get_current_page()->get_field_ptr(line)->get_field_type())
    {
    case t_field_type::FLOAT_IO:
        draw_float_io_field(gui->cast_to_float_io(gui->get_current_page()->get_field_ptr(line)), line);
        break;

    case t_field_type::BOOL_IO:
        draw_bool_io_field(gui->cast_to_bool_io(gui->get_current_page()->get_field_ptr(line)), line);
        break;

    case t_field_type::SUBPAGE_LINK:
        draw_text(">", line+1, LCD_FIELD_VALUE_START);
        break;

    case t_field_type::TEXT:
        break;
    }
}

void vis_controller::draw_names()
{
    /*Expecting mutex already taken*/
    for(uint8_t idx=0; idx<gui->get_current_page()->get_numof_fields(); idx++)
    {
        draw_text(gui->get_current_page()->get_field_ptr(idx)->get_name(), idx+1, LCD_L_MARGIN);
    }
}

void vis_controller::draw_bool_io_field(bool_io_field* bool_io_field_ptr, uint8_t line)
{
    bool val=bool_io_field_ptr->get_val();

    if(val==true) draw_text(bool_io_field_ptr->get_true_text(), line+1, LCD_FIELD_VALUE_START);
    else draw_text(bool_io_field_ptr->get_false_text(), line+1, LCD_FIELD_VALUE_START);    
}

void vis_controller::draw_float_io_field(float_io_field *float_io_field_ptr, uint8_t line)
{
    std::ostringstream oss;
    oss<<std::fixed<<std::setprecision(float_io_field_ptr->get_prec())<<float_io_field_ptr->get_val();

    draw_text(oss.str()+float_io_field_ptr->get_unit()+" ", line+1, LCD_FIELD_VALUE_START);
}

void vis_controller::draw_text(std::string text, uint8_t line, uint8_t pos)
{
    uint8_t index;
    for(uint8_t i=0; i<text.size(); i++)
    {
        if(pos+i>LCD_MAX_CHARS_PER_LINE) return; //not enough space for the rest;
        index=char_to_font_index(text[i]);
        esp_lcd_panel_draw_bitmap(*lcd_handle, (pos+i)*VIS_FONT_W, line*VIS_FONT_H, (pos+i+1)*VIS_FONT_W, (line+1)*VIS_FONT_H, font[index]);
    }
}

void vis_controller::draw_bar_at(uint8_t row, uint8_t col, uint8_t type)
{
    esp_lcd_panel_draw_bitmap(
        *lcd_handle, 
        (LCD_FIELD_VALUE_START+col)*VIS_SELECTBAR_W,
        (row+2)*VIS_FONT_H+VIS_SELECTBAR_VOFFSET,
        (LCD_FIELD_VALUE_START+col+1)*VIS_SELECTBAR_W,
        (row+2)*VIS_FONT_H+VIS_SELECTBAR_VOFFSET+VIS_SELECTBAR_H,
        selectbar[type]
    );
}

void vis_init()
{
    
    /*controller instance creation*/
    if(vis==nullptr)
    {
        vis=new vis_controller(&lcd_handle);
        if(vis==nullptr)
        {
            ESP_LOGE("VIS", "vis_controller creation failed");
            exit(-1);
        }
    }
}

void vis_controller::draw_missing_sensor_msg(uint8_t port, uint8_t addr, bool proceed, std::string name)
{
    std::stringstream ss;
    clear();

    draw_text("sensor "+name, 0, 0);
    // ss<<"addr. 0x"<<std::hex<<std::uppercase<<std::setw(2)<<std::setfill('0')<<(int)addr;
    // ss<<std::dec<<std::nouppercase<<std::setfill(' ')<<" at port "<<port;
    // draw_text(ss.str(), 1, 0);
    draw_text("Connection Fail", 1, 0);

    if(proceed)
    {
        draw_text("proceeding in: ", 4, 0);

        for(uint8_t i=SEN_MISSING_PROCEED_DELAY_S; i>0; i--)
        {
            draw_text(std::to_string(i)+" ", 4, 15);
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }
    else
    {
        draw_text("Not proceeding", 4, 0);
        draw_text("reboot required", 5, 0);
        vTaskDelay(portMAX_DELAY);
    }
}