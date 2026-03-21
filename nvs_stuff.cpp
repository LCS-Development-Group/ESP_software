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
const char* NVS_GUI_SS_DELAY="SSDL";
const char* NVS_GUI_SS_EN="SSEN";

nvs_handle_t nvs;
typedef union
{
    float float_rep;
    uint32_t uint_rep;
} float_uint32_t;

void init_nvs()
{
    if(DEBUG_NVS_ERASE_ON_INIT) ESP_ERROR_CHECK(nvs_flash_erase());

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
        ESP_LOGE("NVS", "Error opening NVS space: %s", esp_err_to_name(err));
        exit(-1);
    }
}

/*true: unequal (!=), false: equal (==)*/
bool floats_unequal(float A, float B)
{
    return (fabsf(A-B)>NVS_FLOAT_COMPARE_EPSILON);
}

float nvs_get_float(const char* key, float def)
{
    float_uint32_t data;
    esp_err_t err=nvs_get_u32(nvs, key, &(data.uint_rep));
    if(err==ESP_OK) return data.float_rep;
    else 
    {
        if(err!=ESP_ERR_NVS_NOT_FOUND) ESP_LOGW("NVS", "Failed getting \"%s\"", key);
        return def;
    }
}

bool nvs_get_bool(const char* key, bool def)
{
    uint8_t data;
    esp_err_t err=nvs_get_u8(nvs, key, &(data));
    if(err==ESP_OK) return data;
    else 
    {
        if(err!=ESP_ERR_NVS_NOT_FOUND) ESP_LOGW("NVS", "Failed getting \"%s\"", key);
        return def;
    }
}

bool save_float(const char* key, float val)
{
    float_uint32_t data;
    esp_err_t err=nvs_get_u32(nvs, key, &(data.uint_rep));
    if(err==ESP_ERR_NVS_NOT_FOUND || (err==ESP_OK && floats_unequal(val, data.float_rep)))
    {
        data.float_rep=val;
        err=nvs_set_u32(nvs, key, data.uint_rep);
        if(err!=ESP_OK) 
        {
            ESP_LOGW("NVS", "Failed setting \"%s\" (%s)", key, esp_err_to_name(err));
            return false;
        }
        return true;
    }
    return false;
}

bool save_bool(const char* key, bool val)
{
    uint8_t data;
    esp_err_t err=nvs_get_u8(nvs, key, &(data));
    if(err==ESP_OK || (err==ESP_OK && data!=val))
    {
        if(data!=val)
        {
            data=val;
            err=nvs_set_u8(nvs, key, data);
            if(err!=ESP_OK) 
            {
                ESP_LOGW("NVS", "Failed setting \"%s\" (%s)", key, esp_err_to_name(err));
                return false;
            }
            return true;
        }
    }
    return false;
}

void nvs_save_values()
{
    bool commit=false;
    
    /*serva*/
    uint8_t j;
    for(uint8_t i=0; i<ACT_SERV_NUMOF; i++)
    {
        xSemaphoreTake(servos[i].mutex, portMAX_DELAY);

        for(j=0; j<2; j++)
            if(save_float(NVS_SERVOS[i][j], servos[i].angle[j])) commit=true;

        xSemaphoreGive(servos[i].mutex);
    }

    /*regulator*/
    xSemaphoreTake(regulator.mutex, portMAX_DELAY);
    if(save_float(NVS_REG_H, regulator.H)) commit=true; //histeresis
    if(save_float(NVS_REG_SP, regulator.SP)) commit=true; //setpoint
    xSemaphoreGive(regulator.mutex);

    /*LCD*/
    xSemaphoreTake(lcd_settings.mutex, portMAX_DELAY);
    if(save_float(NVS_GUI_SS_DELAY, lcd_settings.ss_delay)) commit=true;

    if(save_bool(NVS_GUI_SS_EN, lcd_settings.ss_enabled)) commit=true;
    xSemaphoreGive(lcd_settings.mutex);

    if(commit)
    {
        esp_err_t err=nvs_commit(nvs);
        if(err!=ESP_OK) ESP_LOGW("NVS", "Failed commit (%s)", esp_err_to_name(err));
    }
}