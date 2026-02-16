#include "header.h"
#include "act_class.h"

t_basic_actuator act_membrane;
servo_config_t serv_cfg;
t_servo servos[ACT_SERV_NUMOF];

void update_servo(uint8_t id);
void update_basic_actuator(t_basic_actuator *actuator);

void task_actuator_main(void *args)
{
    xEventGroupWaitBits(main_event_group, TASK_START_SYNCBIT, pdFALSE, pdFALSE, portMAX_DELAY);
    if(DEBUG_TASK_ANOUNCE) ESP_LOGI("ACT", "task_actuator started");

    uint8_t ntcode=0x00;
    while(true)
    {
        if(xQueueReceive(task_queue_list[ACT_TASKID], &ntcode, portMAX_DELAY)==pdPASS)
        {
            //ESP_LOGI("ACT", "Notified with %d", ntcode);
            switch(ntcode)
            {
                case ACT_NTCODE_UPDATE_MEMB:
                    update_basic_actuator(&act_membrane);
                    break;

                case ACT_NTCODE_UPDATE_SERV0:
                    update_servo(0);
                    break;

                case ACT_NTCODE_UPDATE_SERV1:
                    update_servo(1);
                    break;

                case ACT_NTCODE_UPDATE_SERV2:
                    update_servo(2);
                    break;

                case ACT_NTCODE_UPDATE_SERV3:
                    update_servo(3);
                    break;

                default:
                    ESP_LOGW("ACT", "Woken by unknown ntcode: %d", ntcode);
                    break;
            }
        }
    }
}

void update_servo(uint8_t id)
{
    if(id>ACT_SERV_NUMOF) return;
    xSemaphoreTake(servos[id].mutex, portMAX_DELAY);
    
    //enable
    exp_set_pin(servos[id].en_pin, servos[id].enabled);

    //angle
    float angle;
    
    ESP_ERROR_CHECK(iot_servo_read_angle(LEDC_LOW_SPEED_MODE, id, &angle));
    if(angle!=servos[id].angle[servos[id].position])
        ESP_ERROR_CHECK(iot_servo_write_angle(LEDC_LOW_SPEED_MODE, id, servos[id].angle[servos[id].position]));

    xSemaphoreGive(servos[id].mutex);
}

void update_basic_actuator(t_basic_actuator *actuator)
{
    if(actuator==nullptr) return;

    xSemaphoreTake(actuator->mutex, portMAX_DELAY);

    //enable
    if(actuator->expander==true) exp_set_pin(actuator->en_pin, actuator->enabled);
    else ESP_ERROR_CHECK(gpio_set_level(gpio_num_t(actuator->en_pin), actuator->enabled));
    xSemaphoreGive(actuator->mutex);
}

void act_init_membrane()
{
    gpio_config_t pin_en_cfg={
        .pin_bit_mask=(1ULL<<ACT_MEMB_EN_PIN),
        .mode=GPIO_MODE_OUTPUT,
        .pull_up_en=GPIO_PULLUP_DISABLE,
        .pull_down_en=GPIO_PULLDOWN_DISABLE,
        .intr_type=GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&pin_en_cfg));

    act_membrane.enabled=false;
    act_membrane.expander=false;
    act_membrane.mutex=xSemaphoreCreateMutex();
    if(act_membrane.mutex==NULL) 
    {
        ESP_LOGE("ACT", "Membrane mutex creation failed");
        exit(-1);
    }
    act_membrane.en_pin=ACT_MEMB_EN_PIN;
    gpio_set_level(gpio_num_t(act_membrane.en_pin), ACT_MEMB_DIS_LVL);
}

void act_init_servos()
{
    serv_cfg={
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

    ESP_ERROR_CHECK(iot_servo_init(LEDC_LOW_SPEED_MODE, &serv_cfg));
    vTaskDelay(pdMS_TO_TICKS(200));

    /*servo 0*/
    servos[0]=
    {
        .enabled=false,
        .mutex=xSemaphoreCreateMutex(),
        .en_pin=ACT_SERV0_EN_PIN,//expander
        .position=0,
        .angle={
            nvs_get_float(NVS_SERVOS[0][0], ACT_SERV_POS0_DEF),
            nvs_get_float(NVS_SERVOS[0][1], ACT_SERV_POS1_DEF)}
    };

    /*servo 1*/
    servos[1]=
    {
        .enabled=false,
        .mutex=xSemaphoreCreateMutex(),
        .en_pin=ACT_SERV1_EN_PIN,//expander
        .position=0,
        .angle={
            nvs_get_float(NVS_SERVOS[1][0], ACT_SERV_POS0_DEF),
            nvs_get_float(NVS_SERVOS[1][1], ACT_SERV_POS1_DEF)}
    };

    /*servo 2*/
    servos[2]=
    {
        .enabled=false,
        .mutex=xSemaphoreCreateMutex(),
        .en_pin=ACT_SERV2_EN_PIN,//expander
        .position=0,
        .angle={
            nvs_get_float(NVS_SERVOS[2][0], ACT_SERV_POS0_DEF),
            nvs_get_float(NVS_SERVOS[2][1], ACT_SERV_POS1_DEF)}
    };

    /*servo 3*/
    servos[3]=
    {
        .enabled=false,
        .mutex=xSemaphoreCreateMutex(),
        .en_pin=ACT_SERV3_EN_PIN,//expander
        .position=0,
        .angle={
            nvs_get_float(NVS_SERVOS[3][0], ACT_SERV_POS0_DEF),
            nvs_get_float(NVS_SERVOS[3][1], ACT_SERV_POS1_DEF)}
    };

    for(uint8_t i=0; i<ACT_SERV_NUMOF; i++)
    {
        if(servos[i].mutex==NULL)
        {
            ESP_LOGE("ACT", "Servo%d mutex creation failed", i);
            exit(-1);
        }
        iot_servo_write_angle(LEDC_LOW_SPEED_MODE, i, servos[i].angle[servos[i].position]);
    }
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