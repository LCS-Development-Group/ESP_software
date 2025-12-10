#include "header.h"
#include "common_includes.h"

//keys
const char* NVS_SERVOS[ACT_SERV_NUMOF][2]={
    {"S0P0", "S0P1"},
    {"S1P0", "S1P1"},
    {"S2P0", "S2P1"},
    {"S3P0", "S3P1"}
};

const char* NVS_REG_H="HIST";
const char* NVS_REG_SP="SETP";
const char* NVS_COM_PERIOD="COMP";

nvs_handle_t nvs;
typedef union
{
    float float_rep;
    uint32_t uint_rep;
} float_uint32_t;

void init_nvs()
{
    esp_err_t err=nvs_flash_init();
    if (err==ESP_ERR_NVS_NO_FREE_PAGES || err==ESP_ERR_NVS_NEW_VERSION_FOUND) 
    {
        //ESP_LOGI("test", error)
        ESP_LOGW("NVS", "partition corrupted, erasing");
        ESP_ERROR_CHECK(nvs_flash_erase());
        err=nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    //opening space
    err=nvs_open(NVS_SPACE_NAME, NVS_READWRITE, &nvs);
    if(err!=ESP_OK)
    {
        ESP_LOGE("NVS", "Error opening NVS handle: %s", esp_err_to_name(err));
        exit(-1);
    }
}

float nvs_get_float(const char* key, float def)
{
    float_uint32_t data;
    esp_err_t err=nvs_get_u32(nvs, key, &(data.uint_rep));
    if(err==ESP_OK) return data.float_rep;
    else 
    {
        if(err!=ESP_ERR_NVS_NOT_FOUND) 
        {
            ESP_LOGW("NVS", "Error reading %s while loading", key);
            nvs_close(nvs);
        }
        return def;
    }
}

void nvs_save_values()
{
    float_uint32_t data;
    esp_err_t err;
    bool commit=false;

    /*serva*/
    uint8_t j;
    for(uint8_t i=0; i<ACT_SERV_NUMOF; i++)
    {
        xSemaphoreTake(servos[i].mutex, portMAX_DELAY);
        for(j=0; j<2; j++)
        {
            err=nvs_get_u32(nvs, NVS_SERVOS[i][j], &(data.uint_rep));
            if(err==ESP_OK || err==ESP_ERR_NVS_NOT_FOUND)
            {
                if(data.float_rep!=servos[i].angle[j])
                {
                    data.float_rep=servos[i].angle[j];
                    nvs_set_u32(nvs, NVS_SERVOS[i][j], data.uint_rep);
                    commit=true;
                }
            }
            else
            {
                ESP_LOGW("NVS", "Error reading %s while saving", NVS_SERVOS[i][j]);
                nvs_close(nvs);
            }
        }
        xSemaphoreGive(servos[i].mutex);
    }

    /*regulator*/
    xSemaphoreTake(regulator.mutex, portMAX_DELAY);
    
    //histeresis
    err=nvs_get_u32(nvs, NVS_REG_H, &(data.uint_rep));
    if(err==ESP_OK || err==ESP_ERR_NVS_NOT_FOUND)
    {
        if(data.float_rep!=regulator.H)
        {
            data.float_rep=regulator.H;
            nvs_set_u32(nvs, NVS_REG_H, data.uint_rep);
            commit=true;
        }
    }

    //setpoint
    err=nvs_get_u32(nvs, NVS_REG_SP, &(data.uint_rep));
    if(err==ESP_OK || err==ESP_ERR_NVS_NOT_FOUND)
    {
        if(data.float_rep!=regulator.SP)
        {
            data.float_rep=regulator.SP;
            nvs_set_u32(nvs, NVS_REG_SP, data.uint_rep);
            commit=true;
        }
    }
    xSemaphoreGive(regulator.mutex);

    /*communicator*/
    xSemaphoreTake(com_send_mutex, portMAX_DELAY);
    err=nvs_get_u32(nvs, NVS_COM_PERIOD, &(data.uint_rep));
    if(err==ESP_OK || err==ESP_ERR_NVS_NOT_FOUND)
    {
        if(data.float_rep!=com_send_period)
        {
            data.float_rep=com_send_period;
            nvs_set_u32(nvs, NVS_COM_PERIOD, data.uint_rep);
            commit=true;
        }
    }
    xSemaphoreGive(com_send_mutex);

    if(commit)ESP_ERROR_CHECK(nvs_commit(nvs));
}