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

void handle_missing_sensors()
{
    if(RHT_int->get_is_initialized()==false) 
        vis->draw_missing_sensor_msg(
            RHT_int->get_port(), 
            RHT_int->get_addr(), 
            RHT_int->get_proceed_when_fail(), 
            "RHT_INT");

    if(RHT_ext->get_is_initialized()==false) 
        vis->draw_missing_sensor_msg(
            RHT_ext->get_port(), 
            RHT_ext->get_addr(), 
            RHT_ext->get_proceed_when_fail(), 
            "RHT_ext");

    if(CURSEN->get_is_initialized()==false) 
        vis->draw_missing_sensor_msg(
            CURSEN->get_port(), 
            CURSEN->get_addr(), 
            CURSEN->get_proceed_when_fail(), 
            "CURSEN");
}