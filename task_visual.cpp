#include "header.h"
#include "vis_class.h"

gpio_config_t lcd_bl_cfg;
spi_bus_config_t spi_bus_cfg;
esp_lcd_panel_io_handle_t lcd_io_handle;
esp_lcd_panel_io_spi_config_t lcd_io_cfg;
esp_lcd_panel_handle_t lcd_handle;
esp_lcd_panel_dev_config_t lcd_dev_config;

vis_controller *vis;

void task_visual_main(void *args)
{
    xEventGroupWaitBits(main_event_group, TASK_START_SYNCBIT, pdFALSE, pdFALSE, portMAX_DELAY);
    ESP_LOGI("Visual", "task_visual started");

    vis->start();
    
    uint32_t ntcode=0x00;
    while(true)
    {
        if(xTaskNotifyWaitIndexed(0, 0x00, 0xff, &ntcode, portMAX_DELAY)==pdPASS)
        {
            switch(ntcode)
            {   
                case VIS_NTCODE_REDRAW_BAR:
                    vis->draw_bar();
                    break;
                
                case VIS_NTCODE_REDRAW_SELECT:
                    vis->draw_select();
                    break;

                case VIS_NTCODE_REDRAW_VALUE:
                    vis->draw_current_value();
                    break;

                case VIS_NTCODE_REDRAW_ALL_VALUES:
                    vis->draw_all_values();
                    vis->draw_bar();
                    break;

                case VIS_NTCODE_REDRAW_ALL:
                    vis->draw_page();
                    vis->draw_select();
                    vis->draw_all_values();
                    break;

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
    bool idx=gui->get_prim_idx();
    clear_field(idx);

    draw_value(gui->get_prim_idx());
    xSemaphoreGive(gui_mutex);
}   

void vis_controller::draw_all_values()
{
    xSemaphoreTake(gui_mutex, portMAX_DELAY);

    for(uint8_t idx=0; idx<gui->get_current_page()->get_numof_fields(); idx++)
        draw_value(idx);

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

void vis_controller::draw_bar(){}

void vis_controller::start()
{
    esp_lcd_panel_disp_on_off(*lcd_handle, true);
    esp_lcd_panel_swap_xy(*lcd_handle, true);
    //esp_lcd_panel_mirror(*lcd_handle, true, true);

    gpio_set_level(LCD_BL_PIN, LCD_BL_ON_LVL);

    clear();
    vTaskDelay(pdMS_TO_TICKS(100));

    draw_page();
    draw_select();
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

void vis_controller::clear_field(uint8_t line)
{
    for(uint16_t col=LCD_FIELD_VALUE_START; col<LCD_MAX_CHARS_PER_LINE; col++)
        esp_lcd_panel_draw_bitmap(*lcd_handle, col*VIS_FONT_W, line*VIS_FONT_H, (col+1)*VIS_FONT_W, (line+1)*VIS_FONT_H, font[VIS_FONT_INDEX_SPACE]);
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
        draw_text(gui->get_current_page()->get_field_ptr(idx)->get_name(), idx+1, LCD_R_MARGIN);
    }
}

void vis_controller::draw_bool_io_field(bool_io_field* bool_io_field_ptr, uint8_t line)
{
    bool val=bool_io_field_ptr->get_val();

    if(val==true) draw_text("ON ", line+1, LCD_FIELD_VALUE_START);
    else draw_text("OFF", line+1, LCD_FIELD_VALUE_START);    
}

void vis_controller::draw_float_io_field(float_io_field *float_io_field_ptr, uint8_t line)
{
    //insert advanced float formatting here
    draw_text(float_io_field_ptr->get_unit(), line+1, LCD_FIELD_VALUE_START);
}

void vis_controller::draw_text(std::string text, uint8_t line, uint8_t pos)
{
    uint8_t index;
    for(uint8_t i=0; i<text.size(); i++)
    {
        if(pos+i>16) return; //not enough space for the rest;
        index=char_to_font_index(text[i]);
        esp_lcd_panel_draw_bitmap(*lcd_handle, (pos+i)*VIS_FONT_W, line*VIS_FONT_H, (pos+i+1)*VIS_FONT_W, (line+1)*VIS_FONT_H, font[index]);
    }
}

void vis_init()
{
    /*Most of the confing taken from example code for ILI9341 EDP-IDF driver*/

    /*BL gpio init*/
    lcd_bl_cfg.mode=GPIO_MODE_OUTPUT;
    lcd_bl_cfg.pin_bit_mask=1ULL<<LCD_BL_PIN;
    lcd_bl_cfg.pull_down_en=GPIO_PULLDOWN_DISABLE;
    lcd_bl_cfg.pull_up_en=GPIO_PULLUP_DISABLE;
    lcd_bl_cfg.intr_type=GPIO_INTR_DISABLE;
    ESP_ERROR_CHECK(gpio_config(&lcd_bl_cfg));


    /*Bus init*/
    spi_bus_cfg.sclk_io_num=LCD_CLK_PIN;
    spi_bus_cfg.mosi_io_num=LCD_DIN_PIN;
    spi_bus_cfg.miso_io_num=-1; //miso unused
    spi_bus_cfg.quadhd_io_num=-1;
    spi_bus_cfg.quadwp_io_num=-1;
    spi_bus_cfg.max_transfer_sz=LCD_HRES*LCD_VRES*LCD_BITS_PX;
    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &spi_bus_cfg, SPI_DMA_CH_AUTO));

    /*I/O Pannel*/
    lcd_io_handle=NULL;
    lcd_io_cfg.cs_gpio_num=LCD_CS_PIN;
    lcd_io_cfg.dc_gpio_num=LCD_DC_PIN;
    lcd_io_cfg.pclk_hz=LCD_CLOCK_HZ;
    lcd_io_cfg.lcd_cmd_bits=LCD_CMD_BITS;
    lcd_io_cfg.lcd_param_bits=LCD_PARAM_BITS;
    lcd_io_cfg.spi_mode=0;
    lcd_io_cfg.trans_queue_depth=10;
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_HOST, &lcd_io_cfg, &lcd_io_handle));

    /*Panel driver*/
    lcd_handle=NULL;
    lcd_dev_config.reset_gpio_num=LCD_RST_PIN;
    lcd_dev_config.rgb_ele_order=LCD_RGB_ELEMENT_ORDER_RGB;
    lcd_dev_config.data_endian=LCD_RGB_DATA_ENDIAN_LITTLE;
    lcd_dev_config.bits_per_pixel=LCD_BITS_PX;
    ESP_ERROR_CHECK(esp_lcd_new_panel_ili9341(lcd_io_handle, &lcd_dev_config, &lcd_handle));

    /*Final init*/
    ESP_ERROR_CHECK(esp_lcd_panel_reset(lcd_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(lcd_handle));

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

