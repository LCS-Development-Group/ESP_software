#include "expander.h"

mcp23x17_t exp_dev;
void exp_init()
{
    xSemaphoreTake(i2c1_mutex, portMAX_DELAY);
    ESP_ERROR_CHECK(mcp23x17_init_desc(&exp_dev, EXP_ADDR, I2C1_PORT, (gpio_num_t)I2C1_SDA_PIN, I2C1_SCL_PIN));
    exp_dev.cfg.master.clk_speed=EXP_FREQ_HZ;

    for(int i=0; i<15; i++)
    {
        ESP_ERROR_CHECK(mcp23x17_set_mode(&exp_dev, i, MCP23X17_GPIO_OUTPUT));
        ESP_ERROR_CHECK(mcp23x17_set_level(&exp_dev, i, EXP_GPIO_DEF_LVL));
    }
    xSemaphoreGive(i2c1_mutex);
}

void exp_set_pin(uint8_t pin, bool lvl)
{
    if(pin>15) return;
    xSemaphoreTake(i2c1_mutex, portMAX_DELAY);
    ESP_ERROR_CHECK(mcp23x17_set_level(&exp_dev, pin, lvl));
    xSemaphoreGive(i2c1_mutex);
}