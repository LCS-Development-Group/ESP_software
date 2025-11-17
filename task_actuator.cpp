#include "header.h"
#include "act_class.h"

t_basic_actuator act_membrane;

void task_actuator_main(void *args)
{
    xEventGroupWaitBits(main_event_group, TASK_START_SYNCBIT, pdFALSE, pdFALSE, portMAX_DELAY);
    ESP_LOGI("ACT", "task_actuator started");

    vTaskDelay(portMAX_DELAY);//temporary
}

void act_init_membrane()
{
    act_membrane.pin_en_cfg={
        .pin_bit_mask=(1ULL<<ACT_MEMB_EN_PIN),
        .mode=GPIO_MODE_OUTPUT,
        .pull_up_en=GPIO_PULLUP_DISABLE,
        .pull_down_en=GPIO_PULLDOWN_DISABLE,
        .intr_type=GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&act_membrane.pin_en_cfg));
    gpio_set_level(ACT_MEMB_EN_PIN, ACT_MEMB_DIS_LVL);

    act_membrane.enabled=false;
    act_membrane.expander=false;
    act_membrane.mutex=xSemaphoreCreateMutex();
    if(act_membrane.mutex==NULL) 
    {
        ESP_LOGE("ACT", "Membrane mutex creation failed");
        exit(-1);
    }

    act_membrane.en_pin=ACT_MEMB_EN_PIN;
}

void act_init_servos()
{
    
}

void act_init_stepper()
{

}

void act_init()
{
    act_init_membrane();
    act_init_servos();
    act_init_stepper();
}