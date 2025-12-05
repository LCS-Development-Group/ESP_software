#include "header.h"

mcp23x17_t exp_dev;
uint16_t exp_mask=0x00
    |(TP_DEF_LVL<<TP5_PIN)
    |(TP_DEF_LVL<<TP4_PIN)
    |(TP_DEF_LVL<<TP3_PIN)
    |(ACT_STEP_DIS_LVL<<ACT_STEP_EN_PIN)
    |(TP_DEF_LVL<<TP2_PIN)
    |(ACT_STEP_DIR_DEF_LVL<<ACT_STEP_DIR_PIN)
    |(TP_DEF_LVL<<TP1_PIN)
    |(TP_DEF_LVL<<TP0_PIN)
    |(ACT_SERV_DIS_LVL<<ACT_SERV0_EN_PIN)
    |(ACT_SERV_DIS_LVL<<ACT_SERV1_EN_PIN)
    |(ACT_SERV_DIS_LVL<<ACT_SERV2_EN_PIN)
    |(ACT_SERV_DIS_LVL<<ACT_SERV3_EN_PIN)
;



void exp_init()
{
    xSemaphoreTake(i2c_bus[EXP_PORT].mutex, portMAX_DELAY);
    ESP_ERROR_CHECK(mcp23x17_init_desc(&exp_dev, EXP_ADDR, EXP_PORT, i2c_bus[EXP_PORT].SDA_pin, i2c_bus[EXP_PORT].SCL_pin));
    exp_dev.cfg.master.clk_speed=EXP_FREQ_HZ;
    
    for(int i=0; i<15; i++)
        ESP_ERROR_CHECK(mcp23x17_set_mode(&exp_dev, i, MCP23X17_GPIO_OUTPUT));

    ESP_ERROR_CHECK(mcp23x17_port_write(&exp_dev, exp_mask));
    xSemaphoreGive(i2c_bus[EXP_PORT].mutex);
}

void exp_set_pin(uint8_t pin, bool lvl)
{
    if(pin>15) return;
    if(((exp_mask&(1U<<pin))>>pin)==lvl) return; //no change

    exp_mask&=~((uint16_t)1<< pin);//clear
    exp_mask|=(((uint16_t)lvl)<<pin);//set val;

    xSemaphoreTake(i2c_bus[EXP_PORT].mutex, portMAX_DELAY);
    ESP_ERROR_CHECK(mcp23x17_port_write(&exp_dev, exp_mask));
    xSemaphoreGive(i2c_bus[EXP_PORT].mutex);
}