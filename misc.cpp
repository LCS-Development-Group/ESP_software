#include "header.h"
#include "esp_rom_sys.h"


void misc_init()
{
    ESP_ERROR_CHECK(i2cdev_init());

    i2c_bus[I2C0_PORT]={
        .SDA_pin=I2C0_SDA_PIN,
        .SCL_pin=I2C0_SCL_PIN,
        .mutex=xSemaphoreCreateMutex(),
    };
    if(i2c_bus[I2C0_PORT].mutex==NULL) 
    {
        ESP_LOGE("Main", "i2c%d_mutex creation failed", I2C0_PORT);
        exit(-1);
    }

    i2c_bus[I2C1_PORT]={
        .SDA_pin=I2C1_SDA_PIN,
        .SCL_pin=I2C1_SCL_PIN,
        .mutex=xSemaphoreCreateMutex(),
    };
    if(i2c_bus[I2C1_PORT].mutex==NULL) 
    {
        ESP_LOGE("Main", "i2c%d_mutex creation failed", I2C1_PORT);
        exit(-1);
    }

    dev_id=DEFAULT_DEV_ID;
    dev_id_mutex=xSemaphoreCreateMutex();
    if(dev_id_mutex==NULL)
    {
        ESP_LOGE("Main", "dev_id mutex creation failed");
        exit(-1);
    }
}

void task_create_fail(uint8_t taskid)
{
    ESP_LOGE("Setup", "Task id=%d creation failed\n", taskid);
    vTaskDelay(pdMS_TO_TICKS(100));
    exit(-1);
}

void init_queue(uint8_t taskid, uint8_t depth)
{
    task_queue_list[taskid]=xQueueCreate(depth, sizeof(uint8_t));
    if(task_queue_list[taskid]==NULL)
    {
        ESP_LOGE("Setup", "Task id=%d queue creation failed\n", taskid);
        vTaskDelay(pdMS_TO_TICKS(100));
        exit(-1);
    }
}

void unstuck_i2c_bus(uint8_t port)
{
    //port mutex expected to be taken

    //temp. gpio config
    gpio_config_t cfg={
        .pin_bit_mask=(1ULL<<i2c_bus[port].SCL_pin)|(1ULL<<i2c_bus[port].SDA_pin),
        .mode=GPIO_MODE_INPUT_OUTPUT_OD,
        .pull_up_en=GPIO_PULLUP_DISABLE,
        .pull_down_en=GPIO_PULLDOWN_DISABLE,
        .intr_type=GPIO_INTR_DISABLE,
    };
    gpio_config(&cfg);

    //trigger SCL 9 times to reset
    for(uint8_t i=0; i<9; i++)
    {
        gpio_set_level((gpio_num_t)i2c_bus[port].SCL_pin, 0);
        esp_rom_delay_us(5);
        gpio_set_level((gpio_num_t)i2c_bus[port].SCL_pin, 1);
        esp_rom_delay_us(5);
    }

    //manual stop condition
    gpio_set_level((gpio_num_t)i2c_bus[port].SCL_pin, 1);
    esp_rom_delay_us(5);
    gpio_set_level((gpio_num_t)i2c_bus[port].SDA_pin, 0);
    esp_rom_delay_us(5);
    gpio_set_level((gpio_num_t)i2c_bus[port].SDA_pin, 1);
    esp_rom_delay_us(5);

    //reset pins
    gpio_reset_pin((gpio_num_t)i2c_bus[port].SCL_pin);
    gpio_reset_pin((gpio_num_t)i2c_bus[port].SDA_pin);
}

void system_gentle_reboot()
{
    ESP_LOGE("Main", "System reboot triggered");

    nvs_save_values();

    //something to check if task comm finished

    vTaskDelay(pdMS_TO_TICKS(300));
    esp_restart();
}

bool probe_I2C_sensor(i2c_port_t port, uint16_t addr)
{

    i2c_dev_t dev;
    memset(&dev, 0, sizeof(i2c_dev_t));
    dev.port=port;
    dev.addr=addr;
    dev.cfg.scl_io_num=i2c_bus[port].SCL_pin;
    dev.cfg.sda_io_num=i2c_bus[port].SDA_pin;

    esp_err_t err=i2c_dev_check_present(&dev);
    if(err!=ESP_OK) return false;
    else return true;
}

void probe_for_I2C_sensors()
{
    RHT_int->set_is_present(probe_I2C_sensor(SEN_RHT_INT_PORT, SEN_RHT_INT_ADDR));
    RHT_ext->set_is_present(probe_I2C_sensor(SEN_RHT_EXT_PORT, SEN_RHT_EXT_ADDR));
    CURSEN->set_is_present(probe_I2C_sensor(SEN_CURSEN_PORT, SEN_CURSEN_ADDR));
    //STARTER ENCODER HERE
}

bool_mutex_t starter_en;
void starter_init()
{
    starter_en.var=false;
    starter_en.mutex=xSemaphoreCreateMutex();
    if(starter_en.mutex==nullptr)
    {
        ESP_LOGE("STA", "starter mutex creation failed");
        exit(-1);
    }
}
//true - critical
bool missing_sensor_msg(t_sensor *sensor, uint8_t line, const char* name, lv_obj_t *label)
{
    lv_obj_set_style_text_color(label, GUI_COLOR_TEXT, 0);
    lv_obj_set_pos(label, 0, 20+line*20);

    char buffer[64];
    if(sensor->get_proceed_when_fail())
    {
        sprintf(buffer, "%s [0x%02X at i2c%d] error", name, sensor->get_addr(), sensor->get_port());
        lv_label_set_text(label, buffer);
        return false;
    }
    else
    {
        sprintf(buffer, "%s [0x%02X at i2c%d] error - Critical", name, sensor->get_addr(), sensor->get_port());
        lv_label_set_text(label, buffer);
        return true;
    }
}

void handle_missing_sensors()
{
    bool all_sensor_present=RHT_int->get_is_initialized() && RHT_ext->get_is_initialized() && CURSEN->get_is_initialized();
    if(all_sensor_present) return;//all is good - nothing to do here
    
    lv_obj_t *screen=lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen, GUI_COLOR_PAGE_BG, 0);

    lv_obj_t *warning=lv_label_create(screen);
    lv_obj_set_style_text_color(warning, GUI_COLOR_TEXT, 0);
    lv_label_set_text(warning, "Warning:");
    lv_obj_align(warning, LV_ALIGN_TOP_LEFT,0, 0);

    uint8_t line=0;
    bool critical=false;

    //not mutexed - potential problem?
    if(RHT_int->get_is_initialized()==false)
    {
        lv_obj_t* RHT_int_label=lv_label_create(screen);
        critical=critical || missing_sensor_msg(RHT_int, line, "RHT chamber", RHT_int_label);
        line++;
    }

    if(RHT_ext->get_is_initialized()==false)
    {
        lv_obj_t* RHT_ext_label=lv_label_create(screen);
        critical=critical || missing_sensor_msg(RHT_ext, line, "RHT external", RHT_ext_label);
        line++;
    }

    if(CURSEN->get_is_initialized()==false)
    {
        lv_obj_t* CURSEN_label=lv_label_create(screen);
        critical=critical || missing_sensor_msg(CURSEN, line, "Current sen.", CURSEN_label);
        line++;
    }

    lv_obj_t *proceed=lv_label_create(screen);
    lv_obj_set_style_text_color(proceed, GUI_COLOR_TEXT, 0);
    lv_obj_set_pos(proceed, 0, 40+line*30);
    
    lv_obj_t *main_screen=lv_screen_active();
    lv_screen_load(screen);

    xSemaphoreTake(gui->get_mutex(), portMAX_DELAY);//simplest (not perfect) way to lock the gui
    if(critical)
    {
        lv_label_set_text(proceed, "Check connection and restart the device");
        lv_refr_now(display);
        vTaskDelay(portMAX_DELAY);
    }
    else 
    {
        uint8_t counter=SEN_PROCEED_DELAY_S;
        char buffer[3];
        lv_label_set_text(proceed, "Listed sensors won't work. Proceeding in: ");
        lv_obj_update_layout(proceed);

        lv_obj_t *countdown=lv_label_create(screen);
        lv_obj_set_style_text_color(countdown, GUI_COLOR_TEXT, 0);
        lv_obj_align_to(countdown, proceed, LV_ALIGN_OUT_RIGHT_MID,0,0);

        do
        {
            sprintf(buffer, "%u", counter);
            lv_label_set_text(countdown, buffer);
            lv_refr_now(display);
            counter--;
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
        while(counter>0);
    }
    xSemaphoreGive(gui->get_mutex());

    /*cleanup*/
    lv_screen_load(main_screen);
    lv_obj_delete(screen);//also deletes the parrents
}