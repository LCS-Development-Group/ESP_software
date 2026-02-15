#include "header.h"

gpio_config_t lcd_bl_cfg;
spi_bus_config_t spi_bus_cfg;
esp_lcd_panel_io_handle_t lcd_io_handle;
esp_lcd_panel_io_spi_config_t lcd_io_cfg;
esp_lcd_panel_handle_t lcd_handle;
esp_lcd_panel_dev_config_t lcd_dev_config;

TimerHandle_t screensaver_timer;
lcd_settings_t lcd_settings;

void screensaver_timer_callback(TimerHandle_t timer);
void update_settings();

void task_lcd_main(void *args)
{
    xEventGroupWaitBits(main_event_group, TASK_START_SYNCBIT, pdFALSE, pdFALSE, portMAX_DELAY);
    if(DEBUG_TASK_ANOUNCE) ESP_LOGI("LCD", "task_lcdstarted");

    xSemaphoreTake(lcd_settings.mutex, portMAX_DELAY);
    if(lcd_settings.ss_enabled) xTimerStart(screensaver_timer, 0);
    xSemaphoreGive(lcd_settings.mutex);
    
    uint32_t ntcode=0x00;
    while(true)
    {
        if(xTaskNotifyWaitIndexed(0, 0x00, 0xff, &ntcode, portMAX_DELAY)==pdPASS)
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
                            ESP_ERROR_CHECK(gpio_set_level(LCD_BL_PIN, LCD_BL_ON_LVL));//here be PWM
                            ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(lcd_handle, true));
                            lcd_settings.ss_state=false;
                        }
                        xTimerReset(screensaver_timer, 0);
                    }
                    break;

                case LCD_NTCODE_ACTIVATE_SCREENSAVER:
                    ESP_ERROR_CHECK(gpio_set_level(LCD_BL_PIN, !LCD_BL_ON_LVL));//here be PWM
                    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(lcd_handle, false));
                    lcd_settings.ss_state=true;
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

void lcd_init()
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
    ESP_ERROR_CHECK(esp_lcd_panel_mirror(lcd_handle, !DEBUG_INVERT_LCD, !DEBUG_INVERT_LCD));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(lcd_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(lcd_handle));

    /*Screensaver*/
    lcd_settings={
        .mutex=xSemaphoreCreateMutex(),
        .ss_enabled=nvs_get_bool(NVS_GUI_SS_EN, GUI_SS_DEF_ENABLED),
        .ss_state=false,
        .ss_delay=nvs_get_float(NVS_GUI_SS_DELAY, GUI_SS_DEF_DELAY_S),
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

    ESP_ERROR_CHECK(gpio_set_level(LCD_BL_PIN, LCD_BL_ON_LVL));//here be PWM
}


void update_settings()
{
    xTimerStop(screensaver_timer, 0);

    TickType_t new_delay=pdMS_TO_TICKS((uint32_t)(lcd_settings.ss_delay*1000));
    TickType_t old_delay=xTimerGetPeriod(screensaver_timer);

    if(new_delay!=old_delay) xTimerChangePeriod(screensaver_timer, new_delay, 0);
    if(lcd_settings.ss_enabled) xTimerReset(screensaver_timer, 0);


    // xSemaphoreTake(lcd_settings.mutex, portMAX_DELAY);

    // xSemaphoreGive(screensaver_en.mutex);

    // if(enabled==false) return;

    // TickType_t old_period=xTimerGetPeriod(screensaver_timer);

    // xSemaphoreTake(screensaver_delay.mutex, portMAX_DELAY);
    // TickType_t new_period=pdMS_TO_TICKS((uint32_t)(screensaver_delay.var*1000));
    // xSemaphoreGive(screensaver_delay.mutex);
    
    // if(new_period!=old_period) xTimerChangePeriod(screensaver_timer, new_period, 0);
    
    // xTimerReset(screensaver_timer, 0);
}

void screensaver_timer_callback(TimerHandle_t timer)
{
    xTaskNotifyIndexed(task_handle_list[LCD_TASKID], 0, LCD_NTCODE_ACTIVATE_SCREENSAVER, eSetValueWithoutOverwrite);
}