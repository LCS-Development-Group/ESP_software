#include "header.h"
#include <iomanip>
#include <sstream>
#include "driver/uart.h"
#include "cJSON.h"

uint8_t *uart_buffer;
uint16_t uart_buffer_idx=COM_BUFF_SIZE-1;
float_mutex_t com_send_period;

#define COM_OUT_BUFSIZ 256
#define COM_IN_BUFSIZ 128
#define UART_TX_BUFFSIZ 512
#define UART_RX_BUFFSIZ 256

char *output_buffer;
char *input_buffer;

QueueSetHandle_t queue_set;
QueueHandle_t uart_queue;

bool update_regulator, update_starter, update_membrane;

//JSON codes
#define JSON_TYPE           "JT"
#define JSON_ID             "ID"

#define JT_SEN              "sen"
#define JSON_SEN_RH_INT     "HI"
#define JSON_SEN_T_INT      "TI"
#define JSON_SEN_RH_EXT     "HE"
#define JSON_SEN_T_EXT      "TE"
#define JSON_SEN_CURR       "MC"
#define JSON_SEN_VOLT       "MV"
#define JSON_SEN_POWR       "MP"

#define JT_REG              "reg"
#define JSON_REG_SP         "SP"
#define JSON_REG_H          "HI"
#define JSON_REG_EN         "EN"
#define JSON_REG_MEMB_EN    "ME"

#define JT_STA              "sta"
#define JSON_STA_STATUS     "SS"
#define JSON_STA_LASER      "LS"

#define JSON_ON             "ON"
#define JSON_OFF            "OFF"

#define UART_QUEUE_DEPTH    6

void send_readings();
void parse_incoming_json(char *json_string);


void create_message();
void clear_buff();
t_DataPacket create_packet(uint8_t id, std::string value);
void write_packet_to_buff(t_DataPacket datapacket);

void task_comm_main(void *args)
{
    xEventGroupWaitBits(main_event_group, TASK_START_SYNCBIT, pdFALSE, pdFALSE, portMAX_DELAY);
    if(DEBUG_TASK_ANOUNCE) ESP_LOGI("COM", "task_comm started");

    update_regulator=false;
    update_starter=false;
    update_membrane=false;

    xQueueAddToSet(task_queue_list[COM_TASKID], queue_set);
    xQueueAddToSet(uart_queue, queue_set);

    QueueSetMemberHandle_t activated_queue;
    uart_event_t event;
    uint8_t ntcode=0x00;
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
                    case COM_NTCODE_SENDALL:
                        // create_message();
                        // uart_write_bytes(COM_UART_PORT, COM_UART_START_MSG, COM_UART_START_LEN);
                        // uart_write_bytes(COM_UART_PORT, (const char *)uart_buffer, uart_buffer_idx);
                        if(!DEBUG_COM_DISABLE_READINGS) send_readings();
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
    }
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
        else 
        if(strcmp(key, JSON_REG_H)==0)
        {
            xSemaphoreTake(regulator.mutex, portMAX_DELAY);
            regulator.H=(float)field->valuedouble;
            xSemaphoreGive(regulator.mutex);
            update_regulator=true;
        }
        else 
        if(strcmp(key, JSON_REG_EN)==0)
        {
            xSemaphoreTake(regulator.mutex, portMAX_DELAY);
            if(strcmp(field->valuestring, JSON_ON)==0) regulator.enabled=true;
            else if(strcmp(field->valuestring, JSON_OFF)==0) regulator.enabled=false;
            xSemaphoreGive(regulator.mutex);
            update_regulator=true;
        }
        else 
        if(strcmp(key, JSON_REG_MEMB_EN)==0)
        {
            xSemaphoreTake(act_membrane.mutex, portMAX_DELAY);
            if(strcmp(field->valuestring, JSON_ON)==0) act_membrane.enabled=true;
            else if(strcmp(field->valuestring, JSON_OFF)==0) act_membrane.enabled=false;
            xSemaphoreGive(act_membrane.mutex);

            xSemaphoreTake(regulator.mutex, portMAX_DELAY);
            regulator.enabled=false;
            xSemaphoreGive(regulator.mutex);

            update_regulator=true;
            update_membrane=true;
        }
    }
}

void send_regulator()
{
    bool temp;
    const char *me=NULL, *en=NULL;

    xSemaphoreTake(dev_id_mutex, portMAX_DELAY);
    uint8_t id=dev_id;
    xSemaphoreGive(dev_id_mutex);


    xSemaphoreTake(regulator.mutex, portMAX_DELAY);
    float sp=regulator.SP;
    float h=regulator.H;
    temp=regulator.enabled;
    xSemaphoreGive(regulator.mutex);

    en=temp ? JSON_ON: JSON_OFF;

    xSemaphoreTake(act_membrane.mutex, portMAX_DELAY);
    temp=act_membrane.enabled;
    xSemaphoreGive(act_membrane.mutex);

    me=temp ? JSON_ON: JSON_OFF;

    int len=snprintf(output_buffer, COM_OUT_BUFSIZ,
        "{\""
        JSON_TYPE "\":\"" JT_REG "\",\"" 
        JSON_ID "\":%d,\"" 
        JSON_REG_SP "\":%.3f,\"" 
        JSON_REG_H "\":%.3f,\"" 
        JSON_REG_EN "\":\"%s\",\"" 
        JSON_REG_MEMB_EN "\":\"%s\"}\n",
        id, sp, h, en, me);
        
    if(len>0 && len<COM_OUT_BUFSIZ)
    {
        uart_write_bytes(COM_UART_PORT, output_buffer, len);
    }
    else ESP_LOGE("COM", "Json reg too large");
}

void handle_starter_json(cJSON *root)
{

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
            send_regulator();
        }
        else if(strcmp(jt->valuestring, JT_STA)==0)
            handle_starter_json(root);
    }
}




t_DataPacket create_packet(uint8_t id, std::string value)
{
    t_DataPacket output;
    output.id=id;
    uint8_t dot_pos=value.find('.');
    
    if(dot_pos==std::string::npos) dot_pos=value.size()-1;

    for(uint8_t idx=0; idx<COM_INT_PART_LEN; idx++)
    {
        // if(dot_pos<=idx) output.int_part[idx]='0';
        // else output.int_part[idx]=value.at(dot_pos-idx-1);
        if((dot_pos+idx)<COM_INT_PART_LEN) output.int_part[idx]='0';
        else output.int_part[idx]=value.at(dot_pos-COM_INT_PART_LEN+idx);
    }
    for(uint8_t idx=0; idx<COM_FRC_PART_LEN; idx++)
    {
        if(dot_pos+idx>=value.size()) output.frc_part[idx]='0';
        else output.frc_part[idx]=value.at(dot_pos+idx+1);
    }
    
    return output;
}

void write_packet_to_buff(t_DataPacket datapacket)
{
    uart_buffer[uart_buffer_idx++]=datapacket.id;
    for(uint8_t idx=0; idx<COM_INT_PART_LEN; idx++)
        uart_buffer[uart_buffer_idx++]=datapacket.int_part[idx];

    uart_buffer[uart_buffer_idx++]='.';

    for(uint8_t idx=0; idx<COM_FRC_PART_LEN; idx++)
        uart_buffer[uart_buffer_idx++]=datapacket.frc_part[idx];

    
    uart_buffer[uart_buffer_idx]='\0';
}

std::string float_to_string(float val)
{
    std::stringstream ss;
    ss<<std::fixed<<std::setprecision(COM_FRC_PART_LEN)<<val;
    return ss.str();
}

void create_message()
{
    clear_buff();

    //chamber RHT
    xSemaphoreTake(*RHT_int->get_mutex_ptr(), portMAX_DELAY);
    write_packet_to_buff(create_packet(COM_T_INT_ID, float_to_string(RHT_int->get_T())));
    write_packet_to_buff(create_packet(COM_RH_INT_ID, float_to_string(RHT_int->get_RH())));
    xSemaphoreGive(*RHT_int->get_mutex_ptr());

    //external RHT
    xSemaphoreTake(*RHT_ext->get_mutex_ptr(), portMAX_DELAY);
    write_packet_to_buff(create_packet(COM_T_EXT_ID, float_to_string(RHT_ext->get_T())));
    write_packet_to_buff(create_packet(COM_RH_EXT_ID, float_to_string(RHT_ext->get_RH())));
    xSemaphoreGive(*RHT_ext->get_mutex_ptr());

    //regulator
    xSemaphoreTake(regulator.mutex, portMAX_DELAY);
    write_packet_to_buff(create_packet(COM_REG_SP, float_to_string(regulator.SP)));
    write_packet_to_buff(create_packet(COM_REG_H, float_to_string(regulator.H)));
    write_packet_to_buff(create_packet(COM_MEMB_STAT_ID, float_to_string(regulator.CV)));
    xSemaphoreGive(regulator.mutex);

    //membrane
    xSemaphoreTake(*CURSEN->get_mutex_ptr(), portMAX_DELAY);
    write_packet_to_buff(create_packet(COM_MEMB_CUR_ID, float_to_string(CURSEN->get_current())));
    write_packet_to_buff(create_packet(COM_MEMB_VOL_ID, float_to_string(CURSEN->get_voltage())));
    write_packet_to_buff(create_packet(COM_MEMB_POW_ID, float_to_string(CURSEN->get_power())));
    xSemaphoreGive(*CURSEN->get_mutex_ptr());
}

void clear_buff()
{
    for(uint16_t i=0; i<=uart_buffer_idx; i++) uart_buffer[i]=0;
    uart_buffer_idx=0;
    uart_buffer[uart_buffer_idx]='\0';
}

void com_init()
{
    uart_buffer=(uint8_t *)malloc(COM_BUFF_SIZE);
    output_buffer=(char *)malloc(COM_OUT_BUFSIZ);
    clear_buff();

    com_send_period.mutex=xSemaphoreCreateMutex();
    if(com_send_period.mutex==nullptr)
    {
        ESP_LOGE("GUI", "mutex creation failed");
        exit(-1);
    }
    com_send_period.var=nvs_get_float(NVS_COM_PERIOD, COM_SEND_PERIOD_LOOPS_MIN);

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


    //uart queue
    uart_queue=xQueueCreate(UART_QUEUE_DEPTH, sizeof(uint8_t));
    if(uart_queue==NULL)
    {
        ESP_LOGE("Setup", "UART queue creation failed\n");
        vTaskDelay(pdMS_TO_TICKS(100));
        exit(-1);
    }

    ESP_ERROR_CHECK(uart_driver_install(COM_UART_PORT, UART_RX_BUFFSIZ, UART_TX_BUFFSIZ, UART_QUEUE_DEPTH, &uart_queue, 0));
    ESP_ERROR_CHECK(uart_param_config(COM_UART_PORT, &uart_config));

    queue_set=xQueueCreateSet(COM_QUEUE_DEPTH+UART_QUEUE_DEPTH);
    if(queue_set==NULL)
    {
        ESP_LOGE("Setup", "Queue set creation failed\n");
        vTaskDelay(pdMS_TO_TICKS(100));
        exit(-1);
    }

    input_buffer=(char*)malloc(COM_IN_BUFSIZ+1);
}