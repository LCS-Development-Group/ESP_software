#include "header.h"
#include "act_class.h"

t_basic_actuator act_membrane;
servo_config_t servos;
float serv0_angle=ACT_SERV_DEF_ANGLE;
float serv1_angle=ACT_SERV_DEF_ANGLE;
float serv2_angle=ACT_SERV_DEF_ANGLE;
float serv3_angle=ACT_SERV_DEF_ANGLE;

void task_actuator_main(void *args)
{
    xEventGroupWaitBits(main_event_group, TASK_START_SYNCBIT, pdFALSE, pdFALSE, portMAX_DELAY);
    ESP_LOGI("ACT", "task_actuator started");

    exp_set_pin(ACT_SERV0_EN_PIN, ACT_SERV_EN_LVL);
    float angles[2]={0.0, 180.0};
    bool idx=0;

    while(1)
    {
        iot_servo_write_angle(LEDC_LOW_SPEED_MODE, 0, angles[idx]);
        idx=!idx;
        vTaskDelay(pdMS_TO_TICKS(2000));
    }

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
    servos={
        .max_angle=ACT_SERV_MAX_ANGLE_DEG,
        .min_width_us=ACT_SERV_MIN_WIDTH_US,
        .max_width_us=ACT_SERV_MAX_WIDTH_US,
        .freq=ACT_SERV_FREQ_HZ,
        .timer_number=LEDC_TIMER_0,
        .channels={
            .servo_pin={
                ACT_SERV0_PWM_PIN,
                ACT_SERV1_PWM_PIN,
                ACT_SERV2_PWM_PIN,
                ACT_SERV3_PWM_PIN,
            },
            .ch={
                LEDC_CHANNEL_0,
                LEDC_CHANNEL_1,
                LEDC_CHANNEL_2,
                LEDC_CHANNEL_3
            }
        },
        .channel_number=ACT_SERV_NUMOF
    };

    ESP_ERROR_CHECK(iot_servo_init(LEDC_LOW_SPEED_MODE, &servos));
    vTaskDelay(pdMS_TO_TICKS(200));
    
    iot_servo_write_angle(LEDC_LOW_SPEED_MODE, 0, serv0_angle);
    exp_set_pin(ACT_SERV0_EN_PIN, ACT_SERV_DIS_LVL);

    iot_servo_write_angle(LEDC_LOW_SPEED_MODE, 1, serv0_angle);
    exp_set_pin(ACT_SERV1_EN_PIN, ACT_SERV_DIS_LVL);

    iot_servo_write_angle(LEDC_LOW_SPEED_MODE, 2, serv0_angle);
    exp_set_pin(ACT_SERV2_EN_PIN, ACT_SERV_DIS_LVL);

    iot_servo_write_angle(LEDC_LOW_SPEED_MODE, 3, serv0_angle);
    exp_set_pin(ACT_SERV3_EN_PIN, ACT_SERV_DIS_LVL);
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