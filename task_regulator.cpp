#include "header.h"

t_reg_var regulator;
bool prev_enabled=false;
bool send_redraw=false;
bool prev_cv=false;
void zero_regulator();
bool update_reg();

void set_membrane();

void task_regulator_main(void *args)
{
    xEventGroupWaitBits(main_event_group, TASK_START_SYNCBIT, pdFALSE, pdFALSE, portMAX_DELAY);
    if(DEBUG_TASK_ANOUNCE) ESP_LOGI("REG", "task_regulator started");

    uint32_t ntcode=0x00;
    while(true)
    {
        if(xTaskNotifyWaitIndexed(0, 0x00, 0xff, &ntcode, portMAX_DELAY)==pdPASS)
        {
            switch(ntcode)
            {
            case REG_NTCODE_UPDATE:
                if(update_reg())
                {
                    xSemaphoreTake(gui_mutex, portMAX_DELAY);
                    send_redraw=(gui->check_if_displayed(&regulator.enabled) && !(gui->get_prim_lock()));
                    xSemaphoreGive(gui_mutex);
                    if(send_redraw) xTaskNotifyIndexed(task_handle_list[VIS_TASKID], 0, VIS_NTCODE_REDRAW_ALL_VALUES, eSetValueWithoutOverwrite);
                }
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
    xSemaphoreTake(RHT_int_var.mutex, portMAX_DELAY);
    regulator.PV=RHT_int_var.RH;
    xSemaphoreGive(RHT_int_var.mutex);

    //update erorr value (E)
    regulator.E=regulator.SP-regulator.PV;

    //regulation logic
    if(regulator.E>(regulator.H/2)) regulator.CV=false;
    else if(regulator.E<(-1*regulator.H/2)) regulator.CV=true;

    //ESP_LOGI("REG","E=%f, E/2=%f, E/2=%f, CV=%d, %d", regulator.E, regulator.H/2, -1*regulator.H/2, regulator.CV, regulator.E<(-1*regulator.H/2));

    if(regulator.CV!=prev_cv)
    {//just changed
        set_membrane();
    }

    xSemaphoreGive(regulator.mutex);
    return true;
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
    xTaskNotifyIndexed(task_handle_list[ACT_TASKID], 0, ACT_NTCODE_UPDATE_MEMB, eSetValueWithoutOverwrite);
}