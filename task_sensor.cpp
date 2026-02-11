#include "header.h"

t_RHT_sensor* RHT_int;
t_RHT_sensor* RHT_ext;
t_INA_sensor* CURSEN;

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
                RHT_int->take_readings();
                RHT_ext->take_readings();
                CURSEN->take_readings();
                break;
            
            case SEN_NTCODE_UPDATE_MEMB:
                CURSEN->take_readings();
                break;

            case SEN_NTCODE_UPDATE_RHT_EXT:
                RHT_ext->take_readings();
                break;

            case SEN_NTCODE_UPDATE_RHT_INT:
                RHT_int->take_readings();
                break;
            
            default:
                ESP_LOGW("SEN", "Woken by unknown ntcode: %d", ntcode);
                break;
            }
        }
    }
}

void sen_init()
{
    RHT_int=new t_RHT_sensor(SEN_RHT_INT_PORT, SEN_RHT_INT_ADDR, SEN_RHT_INT_PROCEED_NONCONN, SEN_RHT_INT_AUTORECONNECT, SEN_MIN_VAL, SEN_MIN_VAL);
    RHT_int->connect();

    RHT_ext=new t_RHT_sensor(SEN_RHT_EXT_PORT, SEN_RHT_EXT_ADDR, SEN_RHT_EXT_PROCEED_NONCONN, SEN_RHT_EXT_AUTORECONNECT, SEN_MIN_VAL, SEN_MIN_VAL);
    RHT_ext->connect();

    CURSEN=new t_INA_sensor(SEN_CURSEN_PORT, SEN_CURSEN_ADDR, SEN_CURSEN_PROCEED_NONCONN, SEN_CURSEN_AUTORECONNECT, SEN_MIN_VAL, SEN_MIN_VAL, SEN_MIN_VAL);
    CURSEN->connect();
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
    if(sht3x_init(dev)==ESP_OK)
    {
        present=true;
        ESP_ERROR_CHECK(sht3x_start_measurement(dev, SHT3X_PERIODIC_1MPS, SHT3X_HIGH));
        vTaskDelay(sht3x_get_measurement_duration(SHT3X_HIGH));
    }
    else 
    {
        present=false;
        ESP_LOGW("SEN", "RHT (0x%02X at %d) connection failed", addr, port);
    }
    xSemaphoreGive(i2c_bus[port].mutex);
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
    dev=new ina219_t();
    ESP_ERROR_CHECK(ina219_init_desc(dev, addr, static_cast<i2c_port_t>(port), i2c_bus[port].SDA_pin , i2c_bus[port].SCL_pin));
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
    if(ina219_init(dev)==ESP_OK)
    {
        present=true;
        ESP_ERROR_CHECK(ina219_configure(
            dev, 
            SEN_CURSEN_BUS_VOLT_RANGE, 
            SEN_CURSEN_GAIN,                        
            SEN_CURSEN_U_RES, 
            SEN_CURSEN_I_RES, 
            SEN_CURSEN_MODE));

        ESP_ERROR_CHECK(ina219_calibrate(dev, SEN_CURSEN_RSHUNT_mOHM/1000.0f));
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