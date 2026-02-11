#include "header.h"

t_RHT_sensor* RHT_int;
t_RHT_sensor* RHT_ext;
t_INA_sensor* CURSEN;

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

    if(RHT_int_dev.i2c_dev.mutex!=NULL) ESP_LOGI("SEN", "mutex not NULL");
    else ESP_LOGI("SEN", "mutex NULL");

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
    //init_cursen();
    //init_RHT_ext();
    //init_RHT_int();

    RHT_int=new t_RHT_sensor(SEN_RHT_INT_PORT, SEN_RHT_INT_ADDR, false, false, 0.0, 0.0);
    RHT_int->connect();
}



//==================================================================================================================
// t_sensor                                                                                               
//==================================================================================================================

t_sensor::t_sensor(uint8_t _port, uint8_t _addr, bool _proceed_nonpresent, bool _auto_reconnect)
:port(_port), addr(_addr), proceed_nonpresent(_proceed_nonpresent), auto_reconnect(_auto_reconnect){}

//==================================================================================================================
// RHT                                                                                              
//==================================================================================================================

t_RHT_sensor::t_RHT_sensor(uint8_t _port, uint8_t _addr, bool _proceed_nonpresent, bool _auto_reconnect, float _default_RH, float _default_T)
:t_sensor(_port, _addr, _proceed_nonpresent, _auto_reconnect), default_RH(_default_RH), default_T(_default_T)
{
    mutex=xSemaphoreCreateMutex();
    if(mutex==NULL)
    {
        ESP_LOGE("SEN", "t_RHT_sensor variable mutex creation error");
        exit(-1);
    }
    
    reset_variable();

    //dev descr. creation
    dev=new sht3x_t();

    if(dev->i2c_dev.mutex!=NULL) ESP_LOGI("SEN", "mutex not NULL");
    ESP_ERROR_CHECK(sht3x_init_desc(dev, addr, static_cast<i2c_port_t>(port), i2c_bus[port].SDA_pin , i2c_bus[port].SCL_pin));
}

t_RHT_sensor::~t_RHT_sensor(){if(dev!=nullptr) delete dev;}

void t_RHT_sensor::reset_variable()
{
    xSemaphoreTake(mutex, portMAX_DELAY);
    RH=default_RH;
    T=default_T;
    xSemaphoreGive(mutex);
}

void t_RHT_sensor::connect()
{
    xSemaphoreTake(i2c_bus[port].mutex, portMAX_DELAY);
    ESP_ERROR_CHECK(sht3x_init_desc(dev, addr, static_cast<i2c_port_t>(port), i2c_bus[port].SDA_pin, i2c_bus[port].SCL_pin));

    esp_err_t err=sht3x_init(dev);
    if(err==ESP_OK)
    {
        present=true;
        ESP_ERROR_CHECK(sht3x_start_measurement(dev, SHT3X_PERIODIC_1MPS, SHT3X_HIGH));
        vTaskDelay(sht3x_get_measurement_duration(SHT3X_HIGH));
    }
    else
    {
        present=false;
        ESP_LOGW("SEN", "RHT_INT connection error: %s", esp_err_to_name(err));
    }
    xSemaphoreGive(i2c_bus[port].mutex);







    // xSemaphoreTake(i2c_bus[port].mutex, portMAX_DELAY);

    // //if(dev.i2c_dev.mutex!=NULL) i2c_dev_delete_mutex(&(dev.i2c_dev));
    // ESP_LOGW("DEBUG", "pre sht init");
    // esp_err_t err=sht3x_init(&dev);
    // ESP_LOGW("DEBUG", "post sht init");
    
    // if(err==ESP_OK)
    // {
    //     present=true;
    //     ESP_ERROR_CHECK(sht3x_start_measurement(&dev, SHT3X_PERIODIC_1MPS, SHT3X_HIGH));
    //     vTaskDelay(sht3x_get_measurement_duration(SHT3X_HIGH));
    // }
    // else 
    // {
    //     present=false;
    //     ESP_LOGW("SEN", "RHT (0x%02X at %d) connection failed", addr, port);
    // }
    // xSemaphoreGive(i2c_bus[port].mutex);
}

void t_RHT_sensor::take_readings()
{
    if(present)
    {//device connected (in theory)
        xSemaphoreTake(i2c_bus[port].mutex, portMAX_DELAY);
        xSemaphoreTake(mutex, portMAX_DELAY);

        esp_err_t err=sht3x_get_results(dev, &T, &RH);

        xSemaphoreGive(mutex);
        xSemaphoreGive(i2c_bus[port].mutex);

        if(err!=ESP_OK)
        {//was connected - now isn't
            ESP_LOGW("SEN", "RHT (port: %d) failed to take readings", port);
            if(auto_reconnect) connect();
            else
            {
                reset_variable();
                present=false;
            }
        }
    }
    else if(auto_reconnect) connect();
}

//==================================================================================================================
// Current sensor (INA)                                                                                         
//==================================================================================================================

t_INA_sensor::t_INA_sensor(uint8_t _port, uint8_t _addr, bool _proceed_nonpresent, bool _auto_reconnect, float _default_current, float _default_voltage, float _default_power)
:t_sensor(_port, _addr, _proceed_nonpresent, _auto_reconnect), default_current(_default_current), default_voltage(_default_voltage), default_power(_default_power)
{
    mutex=xSemaphoreCreateMutex();
    if(mutex==NULL)
    {
        ESP_LOGE("SEN", "t_INA_sensor variable mutex creation error");
        exit(-1);
    }

    
    
    reset_variable();

    //dev descr. creation
    ESP_ERROR_CHECK(ina219_init_desc(&dev, addr, static_cast<i2c_port_t>(port), i2c_bus[port].SDA_pin , i2c_bus[port].SCL_pin));
}

void t_INA_sensor::reset_variable()
{
    xSemaphoreTake(mutex, portMAX_DELAY);
    current=default_current;
    voltage=default_voltage;
    power=default_power;
    xSemaphoreGive(mutex);
}

void t_INA_sensor::connect()
{
    xSemaphoreTake(i2c_bus[port].mutex, portMAX_DELAY);

    //if(dev.i2c_dev.mutex!=NULL) i2c_dev_delete_mutex(&(dev.i2c_dev));

    if(ina219_init(&dev)==ESP_OK)
    {
        present=true;
        ESP_ERROR_CHECK(ina219_init(&dev));
        ESP_ERROR_CHECK(ina219_configure(
            &dev, 
            SEN_CURSEN_BUS_VOLT_RANGE, 
            SEN_CURSEN_GAIN,                        
            SEN_CURSEN_U_RES, 
            SEN_CURSEN_I_RES, 
            SEN_CURSEN_MODE));

        ESP_ERROR_CHECK(ina219_calibrate(&dev, SEN_CURSEN_RSHUNT_mOHM/1000.0f));
    }
    else 
    {
        present=false;
        ESP_LOGW("SEN", "Cursen (0x%02X %d) connection failed", addr, port);
    }
    xSemaphoreGive(i2c_bus[port].mutex);
}

void t_INA_sensor::take_readings()
{

}