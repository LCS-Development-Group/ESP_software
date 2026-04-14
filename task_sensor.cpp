#include "header.h"

t_RHT_sensor* RHT_int;
t_RHT_sensor* RHT_ext;
t_INA_sensor* CURSEN;

void task_sensor_main(void *args)
{
    xEventGroupWaitBits(main_event_group, TASK_START_SYNCBIT, pdFALSE, pdFALSE, portMAX_DELAY);

    uint8_t ntcode=0x00;
    while(true)
    {
        if(xQueueReceive(task_queue_list[SEN_TASKID], &ntcode, portMAX_DELAY)==pdPASS)
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
    RHT_int=new t_RHT_sensor(SEN_RHT_INT_PORT, SEN_RHT_INT_ADDR, SEN_RHT_INT_PROCEED_CONNFAIL, SEN_MIN_VAL, SEN_MIN_VAL);
    RHT_int->connection_initialize();

    RHT_ext=new t_RHT_sensor(SEN_RHT_EXT_PORT, SEN_RHT_EXT_ADDR, SEN_RHT_EXT_PROCEED_CONNFAIL, SEN_MIN_VAL, SEN_MIN_VAL);
    RHT_ext->connection_initialize();

    CURSEN=new t_INA_sensor(SEN_CURSEN_PORT, SEN_CURSEN_ADDR, SEN_CURSEN_PROCEED_CONNFAIL, SEN_MIN_VAL, SEN_MIN_VAL, SEN_MIN_VAL);
    CURSEN->connection_initialize();
}


//==================================================================================================================
// t_sensor                                                                                               
//==================================================================================================================

t_sensor::t_sensor(uint8_t _port, uint8_t _addr, bool _proceed_when_fail)
:port(_port), addr(_addr), proceed_when_fail(_proceed_when_fail){}

//==================================================================================================================
// RHT                                                                                              
//==================================================================================================================

t_RHT_sensor::t_RHT_sensor(uint8_t _port, uint8_t _addr, bool _proceed_when_fail, float _default_RH, float _default_T)
:t_sensor(_port, _addr, _proceed_when_fail), default_RH(_default_RH), default_T(_default_T)
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

void  t_RHT_sensor::reset_variable_without_mutex()
{
    RH=default_RH;
    T=default_T;
}

void t_RHT_sensor::connection_initialize()
{
    xSemaphoreTake(i2c_bus[port].mutex, portMAX_DELAY);
    if(sht3x_init(dev)==ESP_OK)
    {
        ESP_ERROR_CHECK(sht3x_start_measurement(dev, SHT3X_PERIODIC_1MPS, SHT3X_HIGH));
        vTaskDelay(sht3x_get_measurement_duration(SHT3X_HIGH));
        is_initialized=true;
    }
    else 
    {
        is_initialized=false;
        ESP_LOGW("SEN", "RHT [0x%02X at %d]: failed to initialize connection", addr, port);
    }
    xSemaphoreGive(i2c_bus[port].mutex);
}

void t_RHT_sensor::take_readings()
{
    if(is_initialized)
    {
        xSemaphoreTake(mutex, portMAX_DELAY);
        xSemaphoreTake(i2c_bus[port].mutex, portMAX_DELAY);
        esp_err_t err=sht3x_get_results(dev, &T, &RH);

        if(err!=ESP_OK)
        {
            ESP_LOGW("SEN", "RHT [0x%02X at %d] failed to take readings (%s)", addr, port, esp_err_to_name(err));
            reset_variable_without_mutex();
            xEventGroupSetBits(main_event_group, SYSTEM_REBOOT_EVBIT);
            ESP_LOGW("SEN", "REBOOT");
        }
        xSemaphoreGive(i2c_bus[port].mutex);
        xSemaphoreGive(mutex);
    }
}

//==================================================================================================================
// Current sensor (INA)                                                                                         
//==================================================================================================================

t_INA_sensor::t_INA_sensor(uint8_t _port, uint8_t _addr, bool _proceed_when_fail, float _default_current, float _default_voltage, float _default_power)
:t_sensor(_port, _addr, _proceed_when_fail), default_current(_default_current), default_voltage(_default_voltage), default_power(_default_power)
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

void t_INA_sensor::reset_variable_without_mutex()
{
    current=default_current;
    voltage=default_voltage;
    power=default_power;
}

void t_INA_sensor::connection_initialize()
{
    xSemaphoreTake(i2c_bus[port].mutex, portMAX_DELAY);
    if(ina219_init(dev)==ESP_OK)
    {
        ESP_ERROR_CHECK(ina219_configure(
            dev, 
            SEN_CURSEN_BUS_VOLT_RANGE, 
            SEN_CURSEN_GAIN,                        
            SEN_CURSEN_U_RES, 
            SEN_CURSEN_I_RES, 
            SEN_CURSEN_MODE));

        ESP_ERROR_CHECK(ina219_calibrate(dev, SEN_CURSEN_RSHUNT_mOHM/1000.0f));
        is_initialized=true;
    }
    else 
    {
        is_initialized=false;
        ESP_LOGW("SEN", "Cursen (0x%02X at %d) connection failed", addr, port);
    }
    xSemaphoreGive(i2c_bus[port].mutex);
}

void t_INA_sensor::take_readings()
{
    if(is_initialized)
    {
        xSemaphoreTake(mutex, portMAX_DELAY);
        xSemaphoreTake(i2c_bus[port].mutex, portMAX_DELAY);
        esp_err_t err=ina219_trigger(dev);

        if(err!=ESP_OK)
        {
            ESP_LOGW("SEN", "CURSEN [0x%02X at %d] failed to take readings (%s)", addr, port, esp_err_to_name(err));
        }
        else
        {
            vTaskDelay(pdMS_TO_TICKS(SEN_CURSEN_WAIT_MS));
            ESP_ERROR_CHECK(ina219_get_bus_voltage(dev, &voltage));
            ESP_ERROR_CHECK(ina219_get_current(dev, &current));
            if(current<0) current=0;//at close to 0 it sometimes shows negative
            ESP_ERROR_CHECK(ina219_get_power(dev, &power));
        }
        xSemaphoreGive(i2c_bus[port].mutex);
        xSemaphoreGive(mutex);
    }
}