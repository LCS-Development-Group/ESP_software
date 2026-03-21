#include "header.h"
#include <iomanip>
#include <sstream>
#include "driver/uart.h"
#include "cJSON.h"

#define COM_OUT_BUFSIZ 256
#define COM_IN_BUFSIZ 128
#define UART_TX_BUFFSIZ 512
#define UART_RX_BUFFSIZ 256

char *output_buffer;
char *input_buffer;

QueueSetHandle_t queue_set;
QueueHandle_t uart_queue;

bool update_regulator, update_starter;

const char* ls_temp[]={"Mode-Locked", "OFF", "QCW", "CW"};
uint8_t ls_index=0;

void send_readings();
void send_regulator();
void send_starter();
void parse_incoming_json(char *json_string);

void task_comm_main(void *args)
{
    xEventGroupWaitBits(main_event_group, TASK_START_SYNCBIT, pdFALSE, pdFALSE, portMAX_DELAY);
    if(DEBUG_TASK_ANOUNCE) ESP_LOGI("COM", "task_comm started");

    update_regulator=false;
    update_starter=false;

    xQueueAddToSet(task_queue_list[COM_TASKID], queue_set);
    xQueueAddToSet(uart_queue, queue_set);

    QueueSetMemberHandle_t activated_queue;
    uart_event_t event;
    uint8_t ntcode=0x00;
    uint8_t send_code=0x00;
    int len;
    while(true)
    {
        //if(xQueueReceive(task_queue_list[COM_TASKID], &ntcode, portMAX_DELAY)==pdPASS)
        
        activated_queue=xQueueSelectFromSet(queue_set, portMAX_DELAY);
        if(activated_queue==task_queue_list[COM_TASKID])
        {
            if(xQueueReceive(task_queue_list[COM_TASKID], &ntcode, portMAX_DELAY)==pdPASS)
            {
                switch(ntcode)
                {
                    case COM_NTCODE_SEND_SEN:
                        if(!DEBUG_COM_DISABLE_READINGS) send_readings();
                        break;
                    
                    case COM_NTCODE_SEND_REG:
                        send_regulator();
                        break;

                    case COM_NTCODE_SEND_STA:
                        send_starter();
                        break;
                    
                    case COM_NTCODE_SENDALL:
                        send_regulator();
                        send_starter();
                        break;

                    default:
                        ESP_LOGW("COM", "Woken by unknown ntcode: %d", ntcode);
                        break;
                }
            }
        }
        else 
        if(activated_queue==uart_queue)
        {
            if(xQueueReceive(uart_queue, &event, 0))
            {
                switch(event.type)
                {
                    case UART_DATA:
                        len=uart_read_bytes(COM_UART_PORT, input_buffer, event.size, pdMS_TO_TICKS(10));
                        if(len>0)
                        {
                            input_buffer[len]='\0';
                            parse_incoming_json(input_buffer);
                        }
                        break;

                    case UART_FIFO_OVF:
                        ESP_LOGE("COM", "Input FIFO overflow");
                        uart_flush_input(COM_UART_PORT);
                        xQueueReset(uart_queue);
                        break;

                    default:
                        break;
                }
            }
        }

        //other tasks
        // update_gui=update_regulator || update_starter;

        if(update_regulator)
        {
            send_code=REG_NTCODE_UPDATE;
            xQueueSend(task_queue_list[REG_TASKID], &send_code , 0);
            update_regulator=false;
        }

        if(update_starter)
        {
            //here be some notification
            update_starter=false;
        }

        // if(update_gui)
        // {
        //     send_code=VIS_NTCODE_REDRAW_ALL_VALUES;
        //     xQueueSend(task_queue_list[VIS_TASKID], &send_code , 0);
        //     update_gui=false;
        // }
    }
}

void com_send_deffault_values()
{

}

void send_readings()
{
    uint8_t id;
    float hi, ti, he, te, mc, mv, mp;

    xSemaphoreTake(dev_id_mutex, portMAX_DELAY);
    id=dev_id;
    xSemaphoreGive(dev_id_mutex);

    //chamber RHT
    xSemaphoreTake(*RHT_int->get_mutex_ptr(), portMAX_DELAY);
    ti=RHT_int->get_T();
    hi=RHT_int->get_RH();
    xSemaphoreGive(*RHT_int->get_mutex_ptr());

    //external RHT
    xSemaphoreTake(*RHT_ext->get_mutex_ptr(), portMAX_DELAY);
    te=RHT_ext->get_T();
    he=RHT_ext->get_RH();
    xSemaphoreGive(*RHT_ext->get_mutex_ptr());

    //membrane
    xSemaphoreTake(*CURSEN->get_mutex_ptr(), portMAX_DELAY);
    mc=CURSEN->get_current();
    mv=CURSEN->get_voltage();
    mp=CURSEN->get_power();
    xSemaphoreGive(*CURSEN->get_mutex_ptr());

    int len=snprintf(output_buffer, COM_OUT_BUFSIZ,
        "{\""
        JSON_TYPE "\":\"" JT_SEN "\",\"" 
        JSON_ID "\":%d,\"" 
        JSON_SEN_RH_INT "\":%.3f,\"" 
        JSON_SEN_T_INT "\":%.3f,\"" 
        JSON_SEN_RH_EXT "\":%.3f,\"" 
        JSON_SEN_T_EXT "\":%.3f,\"" 
        JSON_SEN_CURR "\":%.3f,\"" 
        JSON_SEN_VOLT "\":%.3f,\"" 
        JSON_SEN_POWR "\":%.3f}\n",
        id, hi, ti, he, te, mc, mv, mp);
        
    if(len>0 && len<COM_OUT_BUFSIZ)
    {
        uart_write_bytes(COM_UART_PORT, output_buffer, len);
    }
    else ESP_LOGE("COM", "Json msg too large");
}

void handle_regulator_json(cJSON *root)
{
    cJSON *field=NULL;
    cJSON_ArrayForEach(field, root)
    {
        const char* key=field->string;

        if(key==NULL) continue;
        if(strcmp(key, JSON_TYPE)==0 || strcmp(key, JSON_ID)==0) continue;

        //SP
        if(strcmp(key, JSON_REG_SP)==0)
        {
            xSemaphoreTake(regulator.mutex, portMAX_DELAY);
            regulator.SP=(float)field->valuedouble;
            xSemaphoreGive(regulator.mutex);
            update_regulator=true;
        }
        else // Histeresis
        if(strcmp(key, JSON_REG_H)==0)
        {
            xSemaphoreTake(regulator.mutex, portMAX_DELAY);
            regulator.H=(float)field->valuedouble;
            xSemaphoreGive(regulator.mutex);
            update_regulator=true;
        }
        else //Regulator enable
        if(strcmp(key, JSON_REG_EN)==0)
        {
            xSemaphoreTake(regulator.mutex, portMAX_DELAY);
            if(strcmp(field->valuestring, JSON_ON)==0) regulator.enabled=true;
            else if(strcmp(field->valuestring, JSON_OFF)==0) regulator.enabled=false;
            xSemaphoreGive(regulator.mutex);
            update_regulator=true;
        }
    }
}

void send_regulator()
{
    xSemaphoreTake(dev_id_mutex, portMAX_DELAY);
    uint8_t id=dev_id;
    xSemaphoreGive(dev_id_mutex);


    xSemaphoreTake(regulator.mutex, portMAX_DELAY);
    float sp=regulator.SP;
    float h=regulator.H;
    bool temp=regulator.enabled;
    xSemaphoreGive(regulator.mutex);

    const char *en=temp ? JSON_ON: JSON_OFF;

    int len=snprintf(output_buffer, COM_OUT_BUFSIZ,
        "{\""
        JSON_TYPE "\":\"" JT_REG "\",\"" 
        JSON_ID "\":%d,\"" 
        JSON_REG_SP "\":%.3f,\"" 
        JSON_REG_H "\":%.3f,\"" 
        JSON_REG_EN "\":\"%s\"}\n",
        id, sp, h, en);
        
    if(len>0 && len<COM_OUT_BUFSIZ)
    {
        uart_write_bytes(COM_UART_PORT, output_buffer, len);
    }
    else ESP_LOGE("COM", "Json reg too large");
}

void handle_starter_json(cJSON *root)
{
    cJSON *field=NULL;
    cJSON_ArrayForEach(field, root)
    {
        const char* key=field->string;

        if(key==NULL) continue;
        if(strcmp(key, JSON_TYPE)==0 || strcmp(key, JSON_ID)==0) continue;

        //Starter enable
        if(strcmp(key, JSON_STA_EN)==0)
        {
            xSemaphoreTake(starter_en.mutex, portMAX_DELAY);
            if(strcmp(field->valuestring, JSON_ON)==0) starter_en.var=true;
            else if(strcmp(field->valuestring, JSON_OFF)==0) starter_en.var=false;
            xSemaphoreGive(starter_en.mutex);
            update_starter=true;
        }
    }
}

void send_starter()
{
    bool temp;
    const char *ls=NULL, *ss=NULL;

    xSemaphoreTake(dev_id_mutex, portMAX_DELAY);
    uint8_t id=dev_id;
    xSemaphoreGive(dev_id_mutex);

    xSemaphoreTake(starter_en.mutex, portMAX_DELAY);
    temp=starter_en.var;
    xSemaphoreGive(starter_en.mutex);

    ss=temp ? JSON_ON: JSON_OFF;
    ls=ls_temp[ls_index];
    ls_index=(ls_index+1)%4;

    int len=snprintf(output_buffer, COM_OUT_BUFSIZ,
        "{\""
        JSON_TYPE "\":\"" JT_STA "\",\"" 
        JSON_ID "\":%d,\"" 
        JSON_STA_EN "\":\"%s\",\"" 
        JSON_STA_LASER "\":\"%s\"}\n", 
        id, ss, ls);
        
    if(len>0 && len<COM_OUT_BUFSIZ)
    {
        uart_write_bytes(COM_UART_PORT, output_buffer, len);
    }
    else ESP_LOGE("COM", "Json sta too large");
}

void parse_incoming_json(char *json_string)
{
    cJSON *root=cJSON_Parse(json_string);
    if(root==NULL) return;

    cJSON *jt=cJSON_GetObjectItem(root, JSON_TYPE);
    if(cJSON_IsString(jt))
    {
        if(strcmp(jt->valuestring, JT_REG)==0)
        {
            handle_regulator_json(root);
            //send_regulator();
        }
        else if(strcmp(jt->valuestring, JT_STA)==0)
        {
            handle_starter_json(root);
            send_starter();
        }
    }
}

void com_init()
{
    uart_config_t uart_config
    {
        .baud_rate=COM_UART_BAUDRATE,
        .data_bits=UART_DATA_8_BITS,
        .parity   =UART_PARITY_DISABLE,
        .stop_bits=UART_STOP_BITS_1,
        .flow_ctrl=UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh=0,
        .source_clk=UART_SCLK_DEFAULT,
        .flags=
        {
            .allow_pd=0,
            .backup_before_sleep=0
        }
    };

    ESP_ERROR_CHECK(uart_driver_install(COM_UART_PORT, UART_RX_BUFFSIZ, UART_TX_BUFFSIZ, UART_QUEUE_DEPTH, &uart_queue, 0));
    ESP_ERROR_CHECK(uart_param_config(COM_UART_PORT, &uart_config));

    queue_set=xQueueCreateSet(COM_QUEUE_DEPTH+UART_QUEUE_DEPTH);
    if(queue_set==NULL)
    {
        ESP_LOGE("Setup", "Queue set creation failed\n");
        vTaskDelay(pdMS_TO_TICKS(100));
        exit(-1);
    }

    output_buffer=(char *)malloc(COM_OUT_BUFSIZ+1);
    input_buffer=(char*)malloc(COM_IN_BUFSIZ+1);
}