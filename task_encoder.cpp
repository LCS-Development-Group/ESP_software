#include "header.h"
#include "driver/pulse_cnt.h"
#include "button.h"

pcnt_unit_handle_t count;
pcnt_unit_config_t count_cfg;
pcnt_channel_handle_t count_channel;
pcnt_chan_config_t count_channel_cfg;
pcnt_glitch_filter_config_t glitch_config;
pcnt_event_callbacks_t count_callback;
button_t enc_sw;

static bool counter_isr(pcnt_unit_handle_t unit, const pcnt_watch_event_data_t *edata, void *user_ctx)
{
    BaseType_t awoken_task=pdFALSE;
    pcnt_unit_stop(count);
    uint8_t sendval=ENC_NTCODE_ROT;
    if(xQueueSendFromISR(task_queue_list[ENC_TASKID], &sendval, &awoken_task)==pdPASS) portYIELD_FROM_ISR(awoken_task);
    return true;
}

void enc_sw_cb(button_t *btn, button_state_t state)
{
    uint8_t sendval=0x00;
    if(state==BUTTON_CLICKED) sendval=ENC_NTCODE_SW_CLICK;
    else 
    {
        if(state==BUTTON_PRESSED_LONG) sendval=ENC_NTCODE_SW_PRESSED;
        else return;
    }
    xQueueSend(task_queue_list[ENC_TASKID], &sendval, 0);
}

void task_encoder_main(void *args)
{
    xEventGroupWaitBits(main_event_group, TASK_START_SYNCBIT, pdFALSE, pdFALSE, portMAX_DELAY);
    if(DEBUG_TASK_ANOUNCE) ESP_LOGI("ENC", "task_encoder started");

    pcnt_unit_enable(count);
    pcnt_unit_clear_count(count);
    pcnt_unit_start(count);
    
    uint8_t ntcode=0x0;
    uint8_t sendval=0x00;
    int pos=0;
    while(true)
    {
        if(xQueueReceive(task_queue_list[ENC_TASKID], &ntcode, portMAX_DELAY)==pdPASS)
        {
            switch(ntcode)
            {
            case ENC_NTCODE_ROT://rotacja
                pcnt_unit_get_count(count, &pos);
                switch(pos)
                {
                case ENC_PCNT_HIGH:
                    sendval=GUI_NTCODE_CUR_POS;
                    xQueueSend(task_queue_list[GUI_TASKID], &sendval, 0);
                    break;

                case ENC_PCNT_LOW:
                    sendval=GUI_NTCODE_CUR_NEG;
                    xQueueSend(task_queue_list[GUI_TASKID], &sendval, 0);
                    break;

                default:
                    ESP_LOGW("Encoder", "ENC_NTCODE_ROT at undefined counter value: %d", pos);
                    break;
                }
                pcnt_unit_clear_count(count);
                pcnt_unit_start(count);
                break;

            case ENC_NTCODE_SW_CLICK:
                sendval=GUI_NTCODE_CUR_ENT;
                xQueueSend(task_queue_list[GUI_TASKID], &sendval, 0);
                break;

            case ENC_NTCODE_SW_PRESSED:
                sendval=GUI_NTCODE_CUR_BCK;
                xQueueSend(task_queue_list[GUI_TASKID], &sendval, 0);
                break;
                
            default:
                ESP_LOGW("Encoder", "Undefined ntcode: %d", ntcode);
                break;
            }
        }
    }
}

void enc_gpio_init()
{   
    /*common*/
    gpio_config_t cfg;
    cfg.mode=GPIO_MODE_INPUT;
    cfg.pull_up_en=GPIO_PULLUP_ENABLE;
    cfg.pull_down_en=GPIO_PULLDOWN_DISABLE;

    /*CLK*/
    cfg.pin_bit_mask=(1ULL<<ENC_CLK_PIN);
    // cfg.intr_type=GPIO_INTR_ANYEDGE;
    cfg.intr_type=GPIO_INTR_DISABLE;
    gpio_config(&cfg);

    /*DT*/
    cfg.pin_bit_mask=(1ULL<<ENC_DT_PIN);
    cfg.intr_type=GPIO_INTR_DISABLE;
    gpio_config(&cfg);

    /*SW button.h*/
    enc_sw.gpio=ENC_SW_PIN;
    enc_sw.internal_pull=true;
    enc_sw.pressed_level=0;
    enc_sw.autorepeat=false;
    enc_sw.callback=NULL;
    enc_sw.ctx=NULL;
    ESP_ERROR_CHECK(button_init(&enc_sw));
    enc_sw.callback=enc_sw_cb;
}
void enc_pnct_init()
{
    count=NULL;
    count_cfg.high_limit=ENC_PCNT_HIGH+1;
    count_cfg.low_limit=ENC_PCNT_LOW-1;    
    count_cfg.intr_priority=1;      
    pcnt_new_unit(&count_cfg, &count);

    count_channel=NULL;
    count_channel_cfg.edge_gpio_num=ENC_CLK_PIN;
    count_channel_cfg.level_gpio_num=ENC_DT_PIN;
    pcnt_new_channel(count, &count_channel_cfg , &count_channel);

    pcnt_channel_set_edge_action(count_channel, PCNT_CHANNEL_EDGE_ACTION_DECREASE, PCNT_CHANNEL_EDGE_ACTION_INCREASE);
    pcnt_channel_set_level_action(count_channel, PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_INVERSE);

    glitch_config.max_glitch_ns=10000;
    pcnt_unit_set_glitch_filter(count, &glitch_config);
    
    count_callback.on_reach=counter_isr;//register callback function
    pcnt_unit_register_event_callbacks(count, &count_callback, NULL);

    pcnt_unit_add_watch_point(count, ENC_PCNT_HIGH);
    pcnt_unit_add_watch_point(count, ENC_PCNT_LOW);
}