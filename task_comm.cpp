#include "header.h"
#include <iomanip>
#include <sstream>
#include "driver/uart.h"

uint8_t *uart_buffer;
uint16_t uart_buffer_idx=COM_BUFF_SIZE-1;
float_mutex_t com_send_period;

void create_message();
void clear_buff();
t_DataPacket create_packet(uint8_t id, std::string value);
void write_packet_to_buff(t_DataPacket datapacket);

void task_comm_main(void *args)
{
    xEventGroupWaitBits(main_event_group, TASK_START_SYNCBIT, pdFALSE, pdFALSE, portMAX_DELAY);
    if(DEBUG_TASK_ANOUNCE) ESP_LOGI("COM", "task_comm started");

    uint32_t ntcode=0x00;
    while(true)
    {
        if(xTaskNotifyWaitIndexed(0, 0x00, 0xff, &ntcode, portMAX_DELAY)==pdPASS)
        {
            switch(ntcode)
            {
                case COM_NTCODE_SENDALL:
                    create_message();
                    uart_write_bytes(COM_UART_PORT, COM_UART_START_MSG, COM_UART_START_LEN);
                    uart_write_bytes(COM_UART_PORT, (const char *)uart_buffer, uart_buffer_idx);
                    break;

                default:
                    ESP_LOGW("COM", "Woken by unknown ntcode: %d", ntcode);
                    break;
            }
        }
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
    ESP_ERROR_CHECK(uart_driver_install(COM_UART_PORT, COM_BUFF_SIZE*2, 0, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_param_config(COM_UART_PORT, &uart_config));
}