#include "header.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_ili9341.h"
#include "graphics.h"

gpio_config_t lcd_bl_cfg;
spi_bus_config_t spi_bus_cfg;
esp_lcd_panel_io_handle_t lcd_io_handle;
esp_lcd_panel_io_spi_config_t lcd_io_cfg;
esp_lcd_panel_handle_t lcd_handle;
esp_lcd_panel_dev_config_t lcd_dev_config;

#define NUM_PX (LCD_HRES*LCD_VRES)
uint16_t bitmask[NUM_PX];
void fill_bitmask(uint16_t color) {for(int i=0; i<NUM_PX; i++) bitmask[i]=color;}

uint8_t char_to_font_index(char c);

// basic_field* field_ptr;
// bool_io_field* bool_io_field_ptr;
// float_io_field* float_io_field_ptr;
// page_link_field* link_field_ptr;

void draw_text(std::string text, uint8_t line, uint8_t pos);
void draw_page();
void draw_field_value(uint8_t i);
void draw_bool_io_field(bool_io_field* bool_io_field_ptr, uint8_t line);
void draw_float_io_field(float_io_field *float_io_field_ptr, uint8_t line);
void draw_select();
void draw_selectbar();
void lcd_init();


void task_visual_main(void *args)
{
    xEventGroupWaitBits(main_event_group, TASK_START_SYNCBIT, pdFALSE, pdFALSE, portMAX_DELAY);
    ESP_LOGI("Visual", "task_visual started");

    lcd_init();

    //draw_text("TEST 19.08.2025",8, 0);
    //draw_text("K. PACH 275442",1, 0);
    //vTaskDelay(portMAX_DELAY); //temporary
    
    uint32_t ntcode=0x00;
    while(true)
    {
        if(xTaskNotifyWaitIndexed(0, 0x00, 0xff, &ntcode, portMAX_DELAY)==pdPASS)
        {
            switch(ntcode)
            {   
                case VIS_NTCODE_REDRAW_VALUE_BAR:
                    
                    draw_selectbar();
                    break;
                
                case VIS_NTCODE_REDRAW_SELECT:
                    draw_select();
                    break;

                case VIS_NTCODE_REDRAW_BAR://is it ever called?
                    draw_selectbar();
                    break;

                case VIS_NTCODE_REDRAW_VALUE:
                    break;

                case VIS_NTCODE_REDRAW_ALL:
                    draw_page();
                    draw_select();
                    break;

                default:
                    ESP_LOGW("Visual", "Woken by unknown ntcode: %d", ntcode);
                    break;
            }
        }
    }
    vTaskDelay(portMAX_DELAY); //temporary
}



void draw_page()
{
    xSemaphoreTake(gui_mutex, portMAX_DELAY);

    page* page=gui->get_current_page();
    uint8_t numof_fields=page->get_numof_fields();
    uint8_t first_line=1;

    draw_text(page->get_page_name()+":", 0, 0);//Page name
    if(page->get_uppage_ptr()!=nullptr)//return (if exists)
    {
        draw_text("<BACK", 1, 1);
        first_line=2;
    }
    
    t_field_type field_type;
    for(uint8_t i=0; i<numof_fields; i++)
    {
        if(i+first_line>LCD_DISPLAYED_FIELDS_PER_PAGE) return;
        
    }
    xSemaphoreGive(gui_mutex);
}

void draw_field_value(uint8_t i)
{

}

void draw_select()
{
    uint8_t pos=gui->get_prev_prim_idx();

    if(pos!=GUI_CURSOR_MAX_INDEX)
    esp_lcd_panel_draw_bitmap(lcd_handle, 0, (pos+1)*GUI_FONT_H, GUI_FONT_W, (pos+2)*GUI_FONT_H, font[GUI_FONT_INDEX_SPACE]);

    pos=gui->get_prim_idx();
    esp_lcd_panel_draw_bitmap(lcd_handle, 0, (pos+1)*GUI_FONT_H, GUI_FONT_W, (pos+2)*GUI_FONT_H, font[GUI_FONT_INDEX_SELECT]);
}

void draw_selectbar()
{
    if(gui->get_prim_lock()==false) return;
}

void draw_bool_io_field(bool_io_field* bool_io_field_ptr, uint8_t line)
{
    draw_text(bool_io_field_ptr->get_name(), line, 1);
    bool val=bool_io_field_ptr->get_val();

    if(val==true) draw_text("ON", line, LCD_FIELD_CONTENT_START);
    else draw_text("OFF", line, LCD_FIELD_CONTENT_START);    
}
void draw_float_io_field(float_io_field *float_io_field_ptr, uint8_t line);


void draw_text(std::string text, uint8_t line, uint8_t pos)
{
    uint8_t index;
    for(uint8_t i=0; i<text.size(); i++)
    {
        if(pos+i>16) return; //not enough space for the rest;
        index=char_to_font_index(text[i]);
        esp_lcd_panel_draw_bitmap(lcd_handle, (pos+i)*GUI_FONT_W, line*GUI_FONT_H, (pos+i+1)*GUI_FONT_W, (line+1)*GUI_FONT_H, font[index]);
    }
}

void lcd_init()
{
    esp_lcd_panel_disp_on_off(lcd_handle, true);
    esp_lcd_panel_swap_xy(lcd_handle, true);
    esp_lcd_panel_mirror(lcd_handle, true, true);

    gpio_set_level(LCD_BL_PIN, LCD_BL_ON_LVL);

    //clear
    fill_bitmask(0x0000);
    esp_lcd_panel_draw_bitmap(lcd_handle, 0, 0, LCD_VRES, LCD_HRES, bitmask);
    vTaskDelay(pdMS_TO_TICKS(100));

    draw_page();
    draw_select();
    draw_selectbar();
    vTaskDelay(pdMS_TO_TICKS(100));

    // //reset temp. variables
    // field_ptr=nullptr;
    // bool_io_field_ptr=nullptr;
    // float_io_field_ptr=nullptr;
    // link_field_ptr=nullptr;
}


void vis_connect_init()
{
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
    spi_bus_initialize(SPI2_HOST, &spi_bus_cfg, SPI_DMA_CH_AUTO);

    /*I/O Pannel*/
    lcd_io_handle=NULL;
    lcd_io_cfg.cs_gpio_num=LCD_CS_PIN;
    lcd_io_cfg.dc_gpio_num=LCD_DC_PIN;
    lcd_io_cfg.pclk_hz=LCD_CLOCK_HZ;
    lcd_io_cfg.lcd_cmd_bits=EXAMPLE_LCD_CMD_BITS;
    lcd_io_cfg.lcd_param_bits=EXAMPLE_LCD_PARAM_BITS;
    lcd_io_cfg.spi_mode=0;
    lcd_io_cfg.trans_queue_depth=10;
    esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_HOST, &lcd_io_cfg, &lcd_io_handle);

    /*Panel driver*/
    lcd_handle=NULL;
    lcd_dev_config.reset_gpio_num=LCD_RST_PIN;
    lcd_dev_config.rgb_ele_order=LCD_RGB_ELEMENT_ORDER_RGB;
    lcd_dev_config.data_endian=LCD_RGB_DATA_ENDIAN_LITTLE;
    lcd_dev_config.bits_per_pixel=LCD_BITS_PX;
    esp_lcd_new_panel_ili9341(lcd_io_handle, &lcd_dev_config, &lcd_handle);

    /*Final init*/
    esp_lcd_panel_reset(lcd_handle);
    esp_lcd_panel_init(lcd_handle);
}

