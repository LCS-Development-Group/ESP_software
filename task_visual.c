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

uint16_t letter_bitmask[GUI_FONT_PX];
void fill_letter_bitmask(uint8_t letter){for(int i=0; i<GUI_FONT_PX; i++) letter_bitmask[i]=font[letter][i];}

uint8_t char_to_font_index(char c);


void draw_text(char* text, uint8_t line, uint8_t pos)
{
    int i=0, index;
    while(text[i]!='\0' && i<17)
    {
        index=char_to_font_index(text[i]);
        if(index!=255)  esp_lcd_panel_draw_bitmap(lcd_handle, (pos+i)*GUI_FONT_W, line*GUI_FONT_H, (pos+i+1)*GUI_FONT_W, (line+1)*GUI_FONT_H, font[index]);
        else            esp_lcd_panel_draw_bitmap(lcd_handle, (pos+i)*GUI_FONT_W, line*GUI_FONT_H, (pos+i+1)*GUI_FONT_W, (line+1)*GUI_FONT_H, font[GUI_FONT_INDEX_SPACE]);
        i++;
    }
}


void task_visual_main(void *args)
{
    xEventGroupWaitBits(main_event_group, TASK_START_SYNCBIT, pdFALSE, pdFALSE, portMAX_DELAY);
    ESP_LOGI("Visual", "task_visual started");
    esp_lcd_panel_disp_on_off(lcd_handle, true);
    esp_lcd_panel_swap_xy(lcd_handle, true);
    esp_lcd_panel_mirror(lcd_handle, true, true);
    
    gpio_set_level(LCD_BL_PIN, LCD_BL_ON_LVL);

    fill_bitmask(0x0000);
    esp_lcd_panel_draw_bitmap(lcd_handle, 0, 0, LCD_VRES, LCD_HRES, bitmask);
    vTaskDelay(pdMS_TO_TICKS(100));
    draw_text("TEST 19.08.2025",0, 0);
    draw_text("K. PACH 275442",1, 0);

    //vTaskDelay(pdMS_TO_TICKS(000));
    vTaskDelay(portMAX_DELAY); //temporary
}


void vis_connect_init()
{
    /*BL gpio init*/
    lcd_bl_cfg.mode=GPIO_MODE_OUTPUT;
    lcd_bl_cfg.pin_bit_mask=1ULL<<LCD_BL_PIN;
    lcd_bl_cfg.pull_down_en=GPIO_PULLDOWN_DISABLE;
    lcd_bl_cfg.pull_up_en=GPIO_PULLDOWN_DISABLE;
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

uint8_t char_to_font_index(char c)
{
    switch(c)
    {
    /*Numbers*/
    case '0': return GUI_FONT_INDEX_0;
    case '1': return GUI_FONT_INDEX_1;
    case '2': return GUI_FONT_INDEX_2;
    case '3': return GUI_FONT_INDEX_3;
    case '4': return GUI_FONT_INDEX_4;
    case '5': return GUI_FONT_INDEX_5;
    case '6': return GUI_FONT_INDEX_6;
    case '7': return GUI_FONT_INDEX_7;
    case '8': return GUI_FONT_INDEX_8;
    case '9': return GUI_FONT_INDEX_9;
    
    /*Uppercase Letters*/
    case 'A': return GUI_FONT_INDEX_UP_A;
    case 'B': return GUI_FONT_INDEX_UP_B;
    case 'C': return GUI_FONT_INDEX_UP_C;
    case 'D': return GUI_FONT_INDEX_UP_D;
    case 'E': return GUI_FONT_INDEX_UP_E;
    case 'F': return GUI_FONT_INDEX_UP_F;
    case 'G': return GUI_FONT_INDEX_UP_G;
    case 'H': return GUI_FONT_INDEX_UP_H;
    case 'I': return GUI_FONT_INDEX_UP_I;
    case 'J': return GUI_FONT_INDEX_UP_J;
    case 'K': return GUI_FONT_INDEX_UP_K;
    case 'L': return GUI_FONT_INDEX_UP_L;
    case 'M': return GUI_FONT_INDEX_UP_M;
    case 'N': return GUI_FONT_INDEX_UP_N;
    case 'O': return GUI_FONT_INDEX_UP_O;
    case 'P': return GUI_FONT_INDEX_UP_P;
    case 'Q': return GUI_FONT_INDEX_UP_Q;
    case 'R': return GUI_FONT_INDEX_UP_R;
    case 'S': return GUI_FONT_INDEX_UP_S;
    case 'T': return GUI_FONT_INDEX_UP_T;
    case 'U': return GUI_FONT_INDEX_UP_U;
    case 'V': return GUI_FONT_INDEX_UP_V;
    case 'W': return GUI_FONT_INDEX_UP_W;
    case 'X': return GUI_FONT_INDEX_UP_X;
    case 'Y': return GUI_FONT_INDEX_UP_Y;
    case 'Z': return GUI_FONT_INDEX_UP_Z;

    /*Special characters*/
    case '%': return GUI_FONT_INDEX_PERCENT;
    case '.': return GUI_FONT_INDEX_DOT;
    case '^': return GUI_FONT_INDEX_DEGC;
    case ' ': return GUI_FONT_INDEX_SPACE;
    
    default: return 255;
    }
}