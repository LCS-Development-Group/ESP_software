#include "header.h"

t_reg_var regulator;
bool prev_enabled=false;
bool send_redraw=false;
bool prev_cv=false;
void zero_regulator();
bool update_reg();
void two_state_regulate();

void set_membrane();

void task_regulator_main(void *args)
{
    xEventGroupWaitBits(main_event_group, TASK_START_SYNCBIT, pdFALSE, pdFALSE, portMAX_DELAY);
    if(DEBUG_TASK_ANOUNCE) ESP_LOGI("REG", "task_regulator started");

    uint8_t ntcode=0x00;
    uint8_t sendval=0x00;
    while(true)
    {
        if(xQueueReceive(task_queue_list[REG_TASKID], &ntcode, portMAX_DELAY)==pdPASS)
        {
            switch(ntcode)
            {
            case REG_NTCODE_UPDATE:
                if(update_reg())
                {
                    //isnt the main already do this check? yes xd NO really xd
                    xSemaphoreTake(gui_mutex, portMAX_DELAY);
                    send_redraw=(gui->check_if_displayed(&regulator.enabled) && !(gui->get_prim_lock()));
                    xSemaphoreGive(gui_mutex);
                    if(send_redraw)
                    {
                        sendval=VIS_NTCODE_REDRAW_ALL_VALUES;
                        xQueueSend(task_queue_list[VIS_TASKID], &sendval, 0);
                    }
                }
                /*respond regardless to the changes or lack thereof*/
                sendval=COM_NTCODE_SEND_REG;
                xQueueSend(task_queue_list[COM_TASKID], &sendval, portMAX_DELAY);
                break;

            case REG_NTCODE_UPDATE_NO_COM:
                // if(update_reg())
                // {
                //     //isnt the main already do this check? yes xd NO really xd
                //     xSemaphoreTake(gui_mutex, portMAX_DELAY);
                //     send_redraw=(gui->check_if_displayed(&regulator.enabled) && !(gui->get_prim_lock()));
                //     xSemaphoreGive(gui_mutex);
                //     if(send_redraw)
                //     {
                //         sendval=VIS_NTCODE_REDRAW_ALL_VALUES;
                //         xQueueSend(task_queue_list[VIS_TASKID], &sendval, 0);
                //     }
                // }
                break;
            
            
            default:
                ESP_LOGW("REG", "Woken by unknown ntcode: %d", ntcode);
                break;
            }
        }
        
    }
}

void reg_init()
{
    regulator={
        .enabled=REG_DEF_BOOL,
        .H=nvs_get_float(NVS_REG_H, REG_DEF_VAL),
        .SP=nvs_get_float(NVS_REG_SP, REG_DEF_VAL),
        .PV=REG_DEF_VAL,
        .E=REG_DEF_VAL,
        .CV=REG_DEF_BOOL,   
        .mutex=xSemaphoreCreateMutex()
    };
    if(regulator.mutex==NULL)
    {
        ESP_LOGE("REG", "regulator mutex creation error");
        exit(-1);
    }
}

bool update_reg()
{
    xSemaphoreTake(regulator.mutex, portMAX_DELAY);
    
    if(prev_enabled!=regulator.enabled)
    {
        prev_enabled=regulator.enabled;
        if(regulator.enabled==false)
        {//just disabled
            zero_regulator();
            set_membrane();
            xSemaphoreGive(regulator.mutex);
            return true;
        }
        //else: just enabled -> rest of function
    }
    else if(regulator.enabled==false)
    {//still disabled
        xSemaphoreGive(regulator.mutex);
        return false;
    }
    //else: still enabled -> rest of function

    //update process value (PV)
    xSemaphoreTake(*RHT_int->get_mutex_ptr(), portMAX_DELAY);
    regulator.PV=RHT_int->get_RH();
    xSemaphoreGive(*RHT_int->get_mutex_ptr());

    //update erorr value (E)
    regulator.E=regulator.SP-regulator.PV;

    //regulation logic
    two_state_regulate();

    if(regulator.CV!=prev_cv)
    {//just changed
        set_membrane();
    }

    xSemaphoreGive(regulator.mutex);
    return true;
}
void two_state_regulate()
{
    //expects mutex to be taken
    float H_2=regulator.H/2;
    if(regulator.E<(-1*H_2)) regulator.CV=true;
    if(regulator.E>(H_2)) regulator.CV=false;
}

void zero_regulator()
{
    regulator.CV=false;
    regulator.E=REG_DEF_VAL;
    regulator.PV=REG_DEF_VAL;
}


void set_membrane()
{
    xSemaphoreTake(act_membrane.mutex, portMAX_DELAY);
    act_membrane.enabled=regulator.CV;
    xSemaphoreGive(act_membrane.mutex);
    prev_cv=regulator.CV;

    uint8_t sendval=ACT_NTCODE_UPDATE_MEMB;
    xQueueSend(task_queue_list[ACT_TASKID], &sendval, 0);
}