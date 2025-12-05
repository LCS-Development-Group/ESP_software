#include "header.h"

sht3x_t RHT_ext_dev;
sht3x_t RHT_int_dev;
ina219_t cursen_dev;

t_RHT_var RHT_ext_var;
t_RHT_var RHT_int_var;
t_INA_var memb_var;

void update_RHT_int();
void update_RHT_ext();
void update_cursen();

void task_sensor_main(void *args)
{
    xEventGroupWaitBits(main_event_group, TASK_START_SYNCBIT, pdFALSE, pdFALSE, portMAX_DELAY);
    if(DEBUG_TASK_ANOUNCE) ESP_LOGI("SEN", "task_sensor started");

    uint32_t ntcode=0x00;
    while(true)
    {
        if(xTaskNotifyWaitIndexed(0, 0x00, 0xff, &ntcode, portMAX_DELAY)==pdPASS)
        {
            switch(ntcode)
            {
            case SEN_NTCODE_UPDATE_ALL:
                update_RHT_int();
                update_RHT_ext();
                update_cursen();
                break;
            
            case SEN_NTCODE_UPDATE_MEMB:
                update_cursen();
                break;

            case SEN_NTCODE_UPDATE_RHT_EXT:
                update_RHT_ext();
                break;

            case SEN_NTCODE_UPDATE_RHT_INT:
                update_RHT_int();
                break;
            
            default:
                ESP_LOGW("SEN", "Woken by unknown ntcode: %d", ntcode);
                break;
            }
        }
    }
}

void update_RHT_int()
{
    xSemaphoreTake(RHT_int_var.mutex, portMAX_DELAY);
    xSemaphoreTake(i2c_bus[SEN_RHT_INT_PORT].mutex, portMAX_DELAY);
    
    esp_err_t err=sht3x_get_results(&RHT_int_dev, &RHT_int_var.T, &RHT_int_var.RH);
   
    xSemaphoreGive(i2c_bus[SEN_RHT_INT_PORT].mutex);
    xSemaphoreGive(RHT_int_var.mutex); 

    if(err!=ESP_OK) ESP_LOGW("SEN_INT", "couldn't get results: %s", esp_err_to_name(err));
}

void update_RHT_ext()
{
    xSemaphoreTake(RHT_ext_var.mutex, portMAX_DELAY);
    xSemaphoreTake(i2c_bus[SEN_RHT_EXT_PORT].mutex, portMAX_DELAY);

    esp_err_t err=sht3x_get_results(&RHT_ext_dev, &RHT_ext_var.T, &RHT_ext_var.RH);

    xSemaphoreGive(i2c_bus[SEN_RHT_EXT_PORT].mutex);
    xSemaphoreGive(RHT_ext_var.mutex);

    if(err!=ESP_OK) ESP_LOGW("SEN_EXT", "couldn't get results: %s", esp_err_to_name(err));
}

void update_cursen()
{
    xSemaphoreTake(i2c_bus[SEN_CURSEN_PORT].mutex, portMAX_DELAY);
    ESP_ERROR_CHECK(ina219_trigger(&cursen_dev));
    xSemaphoreGive(i2c_bus[SEN_CURSEN_PORT].mutex);

    vTaskDelay(pdMS_TO_TICKS(SEN_CURSEN_WAIT_MS));

    xSemaphoreTake(i2c_bus[SEN_CURSEN_PORT].mutex, portMAX_DELAY);
    xSemaphoreTake(memb_var.mutex, portMAX_DELAY);

    ESP_ERROR_CHECK(ina219_get_bus_voltage(&cursen_dev, &memb_var.voltage));
    ESP_ERROR_CHECK(ina219_get_current(&cursen_dev, &memb_var.current));
    memb_var.current=std::fabsf(memb_var.current);//at close to 0 it sometimes shows negative

    ESP_ERROR_CHECK(ina219_get_power(&cursen_dev, &memb_var.power));
    
    xSemaphoreGive(memb_var.mutex);
    xSemaphoreGive(i2c_bus[SEN_CURSEN_PORT].mutex);
}

void init_RHT_int()
{
    //variable
    RHT_int_var=
    {
        .RH=SEN_MIN_VAL,
        .T=SEN_MIN_VAL,
        .mutex=xSemaphoreCreateMutex()
    };
    if(RHT_int_var.mutex==NULL)
    {
        ESP_LOGE("SEN", "RHT_int_var mutex creation error");
        exit(-1);
    }

    //sensor
    xSemaphoreTake(i2c_bus[SEN_RHT_INT_PORT].mutex, portMAX_DELAY);
    ESP_ERROR_CHECK(sht3x_init_desc(&RHT_int_dev, SEN_RHT_INT_ADDR, SEN_RHT_INT_PORT, i2c_bus[SEN_RHT_INT_PORT].SDA_pin, i2c_bus[SEN_RHT_INT_PORT].SCL_pin));
    ESP_ERROR_CHECK(sht3x_init(&RHT_int_dev));
    ESP_ERROR_CHECK(sht3x_start_measurement(&RHT_int_dev, SHT3X_PERIODIC_1MPS, SHT3X_HIGH));
    vTaskDelay(sht3x_get_measurement_duration(SHT3X_HIGH));
    xSemaphoreGive(i2c_bus[SEN_RHT_INT_PORT].mutex);
}

void init_RHT_ext()
{
    //variable
    RHT_ext_var=
    {
        .RH=SEN_MIN_VAL,
        .T=SEN_MIN_VAL,
        .mutex=xSemaphoreCreateMutex()
    };
    if(RHT_ext_var.mutex==NULL)
    {
        ESP_LOGE("SEN", "RHT_ext_var mutex creation error");
        exit(-1);
    }

    //sensor
    xSemaphoreTake(i2c_bus[SEN_RHT_EXT_PORT].mutex, portMAX_DELAY);
    ESP_ERROR_CHECK(sht3x_init_desc(&RHT_ext_dev, SEN_RHT_EXT_ADDR, SEN_RHT_EXT_PORT, i2c_bus[SEN_RHT_EXT_PORT].SDA_pin, i2c_bus[SEN_RHT_EXT_PORT].SCL_pin));
    ESP_ERROR_CHECK(sht3x_init(&RHT_ext_dev));
    ESP_ERROR_CHECK(sht3x_start_measurement(&RHT_ext_dev, SHT3X_PERIODIC_1MPS, SHT3X_HIGH));
    vTaskDelay(sht3x_get_measurement_duration(SHT3X_HIGH));
    xSemaphoreGive(i2c_bus[SEN_RHT_EXT_PORT].mutex);
}

void init_cursen()
{
    memb_var=
    {
        .current=SEN_MIN_VAL,
        .voltage=SEN_MIN_VAL,
        .power=SEN_MIN_VAL,
        .mutex=xSemaphoreCreateMutex()
    };
    if(memb_var.mutex==NULL)
    {
        ESP_LOGE("SEN", "memb_var mutex creation error");
        exit(-1);
    }

    //sensor
    xSemaphoreTake(i2c_bus[SEN_CURSEN_PORT].mutex, portMAX_DELAY);
    ESP_ERROR_CHECK(ina219_init_desc(&cursen_dev, SEN_CURSEN_ADDR, SEN_CURSEN_PORT, i2c_bus[SEN_CURSEN_PORT].SDA_pin , i2c_bus[SEN_CURSEN_PORT].SCL_pin));
    ESP_ERROR_CHECK(ina219_init(&cursen_dev));
    ESP_ERROR_CHECK(ina219_configure(
        &cursen_dev, 
        SEN_CURSEN_BUS_VOLT_RANGE, 
        SEN_CURSEN_GAIN,                        
        SEN_CURSEN_U_RES, 
        SEN_CURSEN_I_RES, 
        SEN_CURSEN_MODE));

    ESP_ERROR_CHECK(ina219_calibrate(&cursen_dev, SEN_CURSEN_RSHUNT_mOHM/1000.0f));
    xSemaphoreGive(i2c_bus[SEN_CURSEN_PORT].mutex);
}

void sen_init()
{
    init_cursen();
    init_RHT_ext();
    init_RHT_int();
}