#include "header.h"

esp_lcd_panel_io_handle_t lcd_io_handle;
esp_lcd_panel_handle_t lcd_handle;

TimerHandle_t screensaver_timer;
lcd_settings_t lcd_settings;

#define BL_LUT_IDMAX 100
uint16_t BL_LUT[]=
{
    0, 1, 2, 3, 5, 6, 7, 8, 9, 10,
    12, 13, 14, 16, 18, 20, 21, 24, 26, 28,
    31, 33, 36, 39, 42, 45, 49, 52, 56, 60,
    64, 68, 72, 77, 82, 87, 92, 98, 103, 109,
    115, 121, 128, 135, 142, 149, 156, 164, 172, 180,
    188, 197, 206, 215, 225, 235, 245, 255, 266, 276,
    288, 299, 311, 323, 336, 348, 361, 375, 388, 402,
    417, 432, 447, 462, 478, 494, 510, 527, 544, 562,
    580, 598, 617, 636, 655, 675, 696, 716, 737, 759,
    781, 803, 826, 849, 872, 896, 921, 946, 971, 997,
    1023,
};

void screensaver_timer_callback(TimerHandle_t timer);
void update_settings();
void update_duty();

void task_lcd_main(void *args)
{
    xEventGroupWaitBits(main_event_group, TASK_START_SYNCBIT, pdFALSE, pdFALSE, portMAX_DELAY);

    xSemaphoreTake(lcd_settings.mutex, portMAX_DELAY);
    if(lcd_settings.ss_enabled) xTimerStart(screensaver_timer, 0);
    xSemaphoreGive(lcd_settings.mutex);
    
    uint8_t ntcode=0x00;
    while(true)
    {
        if(xQueueReceive(task_queue_list[LCD_TASKID], &ntcode, portMAX_DELAY)==pdPASS)
        {
            xSemaphoreTake(lcd_settings.mutex, portMAX_DELAY);
            switch(ntcode)
            {   
                case LCD_NTCODE_DEACTIVATE_SCREENSAVER:
                    xTimerStop(screensaver_timer, 0);
                    if(lcd_settings.ss_enabled)
                    {
                        if(lcd_settings.ss_state==true)
                        {
                            lcd_settings.ss_state=false;
                            
                            esp_lcd_panel_disp_on_off(lcd_handle, true);
                            update_duty();
                        }
                        xTimerReset(screensaver_timer, 0);
                    }
                    break;

                case LCD_NTCODE_ACTIVATE_SCREENSAVER:
                    lcd_settings.ss_state=true;
                    esp_lcd_panel_disp_on_off(lcd_handle, false);
                    update_duty();
                    break;

                case LCD_NTCODE_UPDATE_SETTINGS:
                    update_settings();
                    break;

                default:
                    ESP_LOGW("LCD", "Woken by unknown ntcode: %d", ntcode);
                    break;
            }
            xSemaphoreGive(lcd_settings.mutex);
        }
    }
}

bool lcd_flushed_isr(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx)
{
    lv_display_t *disp = (lv_display_t *)user_ctx;
    lv_display_flush_ready(disp);
    return false;
}

void lcd_init()
{
    /*Most of the confing taken from example code for ILI9341 EDP-IDF driver*/
    lcd_settings.brightness=LCD_DEF_BRIGHT;

    /*Backlight PWM*/
    ledc_timer_config_t lcd_bl_timer={};
    lcd_bl_timer.speed_mode         =LCD_BL_LEDC_MODE;
    lcd_bl_timer.duty_resolution    =LEDC_TIMER_10_BIT;
    lcd_bl_timer.timer_num          =LCD_BL_LEDC_TIMER;
    lcd_bl_timer.freq_hz            =LCD_BL_FREQ_HZ;
    lcd_bl_timer.clk_cfg            =LEDC_USE_XTAL_CLK;
    ESP_ERROR_CHECK(ledc_timer_config(&lcd_bl_timer));

    ledc_channel_config_t lcd_bl_channel={};
    lcd_bl_channel.gpio_num       =LCD_BL_PIN;
    lcd_bl_channel.speed_mode     =LCD_BL_LEDC_MODE;
    lcd_bl_channel.channel        =LCD_BL_LEDC_CHANNEL;
    lcd_bl_channel.intr_type      =LEDC_INTR_DISABLE;
    lcd_bl_channel.timer_sel      =LCD_BL_LEDC_TIMER;
    lcd_bl_channel.duty           =0;
    lcd_bl_channel.hpoint         =0;
    ESP_ERROR_CHECK(ledc_channel_config(&lcd_bl_channel));


    /*Bus init*/
    spi_bus_config_t spi_bus_cfg={};
    spi_bus_cfg.sclk_io_num=LCD_CLK_PIN;
    spi_bus_cfg.mosi_io_num=LCD_DIN_PIN;
    spi_bus_cfg.miso_io_num=-1; //miso unused
    spi_bus_cfg.quadhd_io_num=-1;
    spi_bus_cfg.quadwp_io_num=-1;
    spi_bus_cfg.max_transfer_sz=LCD_HRES*LCD_VRES*LCD_BITS_PX;
    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &spi_bus_cfg, SPI_DMA_CH_AUTO));

    /*I/O Pannel*/
    lcd_io_handle=NULL;
    esp_lcd_panel_io_spi_config_t lcd_io_cfg={};
    lcd_io_cfg.cs_gpio_num=LCD_CS_PIN;
    lcd_io_cfg.dc_gpio_num=LCD_DC_PIN;
    lcd_io_cfg.pclk_hz=LCD_CLOCK_HZ;
    lcd_io_cfg.lcd_cmd_bits=LCD_CMD_BITS;
    lcd_io_cfg.lcd_param_bits=LCD_PARAM_BITS;
    lcd_io_cfg.spi_mode=0;
    lcd_io_cfg.trans_queue_depth=10;
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_HOST, &lcd_io_cfg, &lcd_io_handle));

    /*Panel driver*/
    esp_lcd_panel_dev_config_t lcd_dev_config={};
    lcd_dev_config.reset_gpio_num=LCD_RST_PIN;
    lcd_dev_config.rgb_ele_order=LCD_RGB_ELEMENT_ORDER_BGR;
    lcd_dev_config.data_endian=LCD_RGB_DATA_ENDIAN_LITTLE;
    lcd_dev_config.bits_per_pixel=LCD_BITS_PX;
    ESP_ERROR_CHECK(esp_lcd_new_panel_ili9341(lcd_io_handle, &lcd_dev_config, &lcd_handle));

    /*Final init*/
    ESP_ERROR_CHECK(esp_lcd_panel_mirror(lcd_handle, !DEBUG_INVERT_LCD, !DEBUG_INVERT_LCD));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(lcd_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(lcd_handle));

    /*Screensaver*/
    lcd_settings={
        .mutex=xSemaphoreCreateMutex(),
        .ss_enabled=nvs_get_bool(NVS_GUI_SS_EN, LCD_SS_DEF_ENABLED),
        .ss_state=false,
        .ss_delay=nvs_get_float(NVS_GUI_SS_DELAY, LCD_SS_DEF_DELAY_S),
        .brightness=100,
    };
    if(lcd_settings.mutex==nullptr)
    {
        ESP_LOGE("LCD", "lcd_settings mutex creation failed");
        exit(-1);
    }

    //timer
    screensaver_timer=xTimerCreate(
        "SS_TMR",
        pdMS_TO_TICKS((uint32_t)(lcd_settings.ss_delay*1000)),
        pdFALSE,
        NULL,
        screensaver_timer_callback
    );
    if(screensaver_timer==NULL)
    {
        ESP_LOGE("LCD", "Screensaver_timer creation failed");
        exit(-1);
    }

    /*Starting the panel*/
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(lcd_handle, true));
    ESP_ERROR_CHECK(esp_lcd_panel_swap_xy(lcd_handle, true));
    update_duty();
}

void update_duty()
{
    uint32_t duty=0;
    int8_t index=0;
    if(lcd_settings.ss_state==true) duty=LCD_BL_DUTY_OFF;
    else
    {
        index=lcd_settings.brightness;
        if(index<0) index=0;
        else if(index>BL_LUT_IDMAX) index=BL_LUT_IDMAX;
        duty=(uint32_t)(BL_LUT[index]);
        //duty=(uint32_t)(LCD_BL_DUTY_ON*lcd_settings.brightness)/100;
    }

    ledc_set_duty(LCD_BL_LEDC_MODE, LCD_BL_LEDC_CHANNEL, duty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LCD_BL_LEDC_CHANNEL);
}

void update_settings()
{
    xTimerStop(screensaver_timer, 0);

    TickType_t new_delay=pdMS_TO_TICKS((uint32_t)(lcd_settings.ss_delay*1000));
    TickType_t old_delay=xTimerGetPeriod(screensaver_timer);

    if(new_delay!=old_delay) xTimerChangePeriod(screensaver_timer, new_delay, 0);
    if(lcd_settings.ss_enabled) xTimerReset(screensaver_timer, 0);

    update_duty();
}

void screensaver_timer_callback(TimerHandle_t timer)
{
    uint8_t sendval=LCD_NTCODE_ACTIVATE_SCREENSAVER;
    xQueueSend(task_queue_list[LCD_TASKID], &sendval, 0);
}