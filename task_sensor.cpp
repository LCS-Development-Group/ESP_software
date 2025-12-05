#include "header.h"

sht3x_t RHT_ext_dev;
sht3x_t RHT_int_dev;
ina219_t cursen_dev;

t_RHT_var RHT_ext_var;
t_RHT_var RHT_int_var;
t_INA_var memb_var;

void task_sensor_main(void *args)
{
    xEventGroupWaitBits(main_event_group, TASK_START_SYNCBIT, pdFALSE, pdFALSE, portMAX_DELAY);
    ESP_LOGI("SEN", "task_sensor started");

    esp_err_t err;
    while(true)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));

        xSemaphoreTake(i2c_bus[SEN_CURSEN_PORT].mutex, portMAX_DELAY);
        ESP_ERROR_CHECK(ina219_trigger(&cursen_dev));
        xSemaphoreGive(i2c_bus[SEN_CURSEN_PORT].mutex);

        vTaskDelay(pdMS_TO_TICKS(SEN_CURSEN_WAIT_MS));

        xSemaphoreTake(i2c_bus[SEN_CURSEN_PORT].mutex, portMAX_DELAY);
        xSemaphoreTake(memb_var.mutex, portMAX_DELAY);

        ESP_ERROR_CHECK(ina219_get_bus_voltage(&cursen_dev, &memb_var.voltage));
        ESP_ERROR_CHECK(ina219_get_current(&cursen_dev, &memb_var.current));
        ESP_ERROR_CHECK(ina219_get_power(&cursen_dev, &memb_var.power));
        
        ESP_LOGI("SEN_CURSEN", "%.3f V, %.3f A, %.3f W,", memb_var.voltage, memb_var.current, memb_var.power);

        xSemaphoreGive(memb_var.mutex);
        xSemaphoreGive(i2c_bus[SEN_CURSEN_PORT].mutex);

        // vTaskDelay(pdMS_TO_TICKS(2000));

        // xSemaphoreTake(RHT_ext_var.mutex, portMAX_DELAY);

        // xSemaphoreTake(i2c_bus[SEN_RHT_EXT_PORT].mutex, portMAX_DELAY);
        // err=sht3x_get_results(&RHT_ext_dev, &RHT_ext_var.T, &RHT_ext_var.RH);
        // xSemaphoreGive(i2c_bus[SEN_RHT_EXT_PORT].mutex);

        // if(err==ESP_OK)
        //     ESP_LOGI("SEN_EXT", "%.3f °C, %.3f %%", RHT_ext_var.T, RHT_ext_var.RH);
        // else
        //     ESP_LOGW("SEN_EXT", "couldn't get results: %s", esp_err_to_name(err));
        // xSemaphoreGive(RHT_ext_var.mutex);

        // vTaskDelay(pdMS_TO_TICKS(2000));

        // xSemaphoreTake(RHT_int_var.mutex, portMAX_DELAY);
        // xSemaphoreTake(i2c_bus[SEN_RHT_INT_PORT].mutex, portMAX_DELAY);
        // err=sht3x_get_results(&RHT_int_dev, &RHT_int_var.T, &RHT_int_var.RH);
        // xSemaphoreGive(i2c_bus[SEN_RHT_INT_PORT].mutex);

        // if(err==ESP_OK)
        //     ESP_LOGI("SEN_INT", "%.3f °C, %.3f %%", RHT_int_var.T, RHT_int_var.RH);
        // else
        //     ESP_LOGW("SEN_INT", "couldn't get results: %s", esp_err_to_name(err));
        // xSemaphoreGive(RHT_int_var.mutex);
    }
}

void init_RHT_int()
{
    //varaible
    RHT_int_var=
    {
        .RH=SEN_VAR_DEF_VAL,
        .T=SEN_VAR_DEF_VAL,
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
    xSemaphoreGive(i2c_bus[SEN_RHT_EXT_PORT].mutex);
}

void init_RHT_ext()
{
    //variable
    RHT_ext_var=
    {
        .RH=SEN_VAR_DEF_VAL,
        .T=SEN_VAR_DEF_VAL,
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
        .current=SEN_VAR_DEF_VAL,
        .voltage=SEN_VAR_DEF_VAL,
        .power=SEN_VAR_DEF_VAL,
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