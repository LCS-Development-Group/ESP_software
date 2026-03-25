#include "header.h"

//DEBUG
bool DEBUG_BOOL=false;
SemaphoreHandle_t DEBUG_BOOL_MUT=xSemaphoreCreateMutex();

float DEBUG_FLOAT=21.65;
SemaphoreHandle_t DEBUG_FLOAT_MUT=xSemaphoreCreateMutex();

float DEBUG_FLOAT_2=15.3;
SemaphoreHandle_t DEBUG_FLOAT_2_MUT=xSemaphoreCreateMutex();


TaskHandle_t task_handle_list[TASK_NUM];
QueueHandle_t task_queue_list[TASK_NUM];
EventGroupHandle_t main_event_group;

t_i2c_bus i2c_bus[I2C_BUS_NUMOF];

uint8_t dev_id;
SemaphoreHandle_t dev_id_mutex;

void main_loop();
void setup();

extern "C" void app_main(void)
{
    setup();

    vTaskDelay(pdMS_TO_TICKS(500));
    xEventGroupSetBits(main_event_group, TASK_START_SYNCBIT);//start the tasks
    vTaskDelay(pdMS_TO_TICKS(100));
    xEventGroupClearBits(main_event_group, TASK_START_SYNCBIT);//safely clear the syncbit

    main_loop();
}
void main_loop()
{
    TickType_t last_wakeup=xTaskGetTickCount();
    uint8_t nvs_counter=0;
    bool retval_rht, retval_cur;
    EventBits_t ev_retval;
    uint8_t sendval;
    while(true)
    {
        //schedule sensor reading aquisition
        vTaskDelay(pdMS_TO_TICKS(MAIN_LOOP_MINIDELAY_MS));
        sendval=SEN_NTCODE_UPDATE_ALL;
        xQueueSend(task_queue_list[SEN_TASKID], &sendval, 0);

        //update the regulator
        vTaskDelay(pdMS_TO_TICKS(MAIN_LOOP_MINIDELAY_MS));
        sendval=REG_NTCODE_UPDATE_NO_COM;
        xQueueSend(task_queue_list[REG_TASKID], &sendval, 0);

        //send data to the pc - every iteration
        vTaskDelay(pdMS_TO_TICKS(MAIN_LOOP_MINIDELAY_MS));
        if(!DEBUG_COM_DISABLE_READINGS)
        {
            sendval=COM_NTCODE_SEND_SEN;
            xQueueSend(task_queue_list[COM_TASKID], &sendval, 0);
        }

        //check if the page with readings is displayed -> redraw it
        // vTaskDelay(pdMS_TO_TICKS(MAIN_LOOP_MINIDELAY_MS));
        // if(xSemaphoreTake(gui_mutex, pdMS_TO_TICKS(MAIN_LOOP_REDRAW_WAIT_MS))==pdTRUE)
        // {
        //     retval_rht=gui->check_if_displayed(RHT_int->get_T_ptr());
        //     retval_cur=gui->check_if_displayed(CURSEN->get_current_ptr());
        //     xSemaphoreGive(gui_mutex);

        //     if(retval_rht || retval_cur)
        //     {
        //         sendval=VIS_NTCODE_REDRAW_ALL_VALUES;
        //         xQueueSend(task_queue_list[VIS_TASKID], &sendval, 0);
        //     } 
        // }
        
        //saving some values to NVS - not every iteration
        nvs_counter++;
        if(nvs_counter==NVS_SAVE_PERIOD_LOOPS)
        {
            nvs_save_values();
            nvs_counter=0;
        }
        
        //check for system reboot trigger
        ev_retval=xEventGroupWaitBits(main_event_group, SYSTEM_REBOOT_EVBIT, pdTRUE, pdFALSE, pdMS_TO_TICKS(10));
        if((ev_retval & SYSTEM_REBOOT_EVBIT)!=0) system_gentle_reboot();

        //sleep until the next loop iteration
        vTaskDelayUntil(&last_wakeup, pdMS_TO_TICKS(MAIN_LOOP_DELAY_MS));
        last_wakeup=xTaskGetTickCount();
    }
}

void setup()
{
    misc_init();
    exp_init();
    init_nvs();

    enc_gpio_init();
    enc_pnct_init();
    lcd_init();
    
    act_init();
    sen_init();
    vTaskDelay(pdMS_TO_TICKS(100));
    reg_init();
    starter_init();
    com_init();

    gui_init();

    handle_missing_sensors();

    main_event_group=xEventGroupCreate();
    if(main_event_group==NULL)
    {
        ESP_LOGE("main", "main_event_group creation failed\n");
        exit(-1);
    }

    if(xTaskCreate(task_encoder_main, "task_encoder", 2048, NULL, 1, &task_handle_list[ENC_TASKID])!=pdPASS) task_create_fail(ENC_TASKID);
    if(xTaskCreate(task_gui_main, "task_gui", 4096, NULL, 1, &task_handle_list[GUI_TASKID])!=pdPASS) task_create_fail(GUI_TASKID);
    if(xTaskCreate(task_visual_main, "task_visual", 8192, NULL, 1, &task_handle_list[VIS_TASKID])!=pdPASS) task_create_fail(VIS_TASKID);
    if(xTaskCreate(task_sensor_main, "task_sensor", 4096, NULL, 1, &task_handle_list[SEN_TASKID])!=pdPASS) task_create_fail(SEN_TASKID);
    if(xTaskCreate(task_actuator_main, "task_actuator", 2048, NULL, 1, &task_handle_list[ACT_TASKID])!=pdPASS) task_create_fail(ACT_TASKID);
    if(xTaskCreate(task_regulator_main, "task_regulator", 2048, NULL, 1, &task_handle_list[REG_TASKID])!=pdPASS) task_create_fail(REG_TASKID);
    if(xTaskCreate(task_comm_main, "task_comm", 4096, NULL, 1, &task_handle_list[COM_TASKID])!=pdPASS) task_create_fail(COM_TASKID);
    if(xTaskCreate(task_lcd_main, "task_lcd", 4096, NULL, 1, &task_handle_list[LCD_TASKID])!=pdPASS) task_create_fail(LCD_TASKID);

    init_queue(ENC_TASKID, ENC_QUEUE_DEPTH);
    init_queue(GUI_TASKID, GUI_QUEUE_DEPTH);
    init_queue(VIS_TASKID, VIS_QUEUE_DEPTH);
    init_queue(SEN_TASKID, SEN_QUEUE_DEPTH);
    init_queue(ACT_TASKID, ACT_QUEUE_DEPTH);
    init_queue(REG_TASKID, REG_QUEUE_DEPTH);
    init_queue(COM_TASKID, COM_QUEUE_DEPTH);
    init_queue(LCD_TASKID, LCD_QUEUE_DEPTH);
}

