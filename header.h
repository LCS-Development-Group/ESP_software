#pragma once
#include "common_includes.h"
#include "act_class.h"
#include "sen_class.h"
#include "I2C.h"
#include "000_gui.h"

#define SETTING_SOFTWARE_VERSION        "v2.0.0"
#define SETTING_CHAMBER_ID_STATIC       "0"
#define SETTING_GUI_START_FROM_PAGE     0// -1 for disabled; 0, 1, ... for page numbers

#define DEBUG_TASK_ANOUNCE          false
#define DEBUG_INVERT_LCD            true
#define DEBUG_NVS_ERASE_ON_INIT     false
#define DEBUG_COM_DISABLE_UART      true
#define DEBUG_COM_DISABLE_READINGS  false

extern bool DEBUG_BOOL;
extern SemaphoreHandle_t DEBUG_BOOL_MUT;

extern float DEBUG_FLOAT;
extern SemaphoreHandle_t DEBUG_FLOAT_MUT;

/*======================================================================================*/
/* GENERAL                                                                              */
/*======================================================================================*/
#define TASK_NUM 7
#define ENC_TASKID 0
#define GUI_TASKID 1
#define SEN_TASKID 2
#define ACT_TASKID 3
#define REG_TASKID 4
#define COM_TASKID 5
#define LCD_TASKID 6

#define ENC_QUEUE_DEPTH 2 //no point in more
#define GUI_QUEUE_DEPTH 5
#define SEN_QUEUE_DEPTH 2
#define ACT_QUEUE_DEPTH 3
#define REG_QUEUE_DEPTH 2
#define COM_QUEUE_DEPTH 2
#define LCD_QUEUE_DEPTH 5

#define TASK_START_SYNCBIT          (1<<23)
#define APP_MAIN_EVBIT              (1<<22)
#define SYSTEM_REBOOT_EVBIT         (1<<21)
#define MAIN_LOOP_DELAY_MS          1000//1s
#define MAIN_LOOP_MINIDELAY_MS      50
#define MAIN_LOOP_REDRAW_WAIT_MS    100

extern EventGroupHandle_t main_event_group;
extern TaskHandle_t task_handle_list[TASK_NUM];
extern QueueHandle_t task_queue_list[TASK_NUM];

#define DEFAULT_DEV_ID 0
extern uint8_t dev_id;
extern SemaphoreHandle_t dev_id_mutex;

/*======================================================================================*/
/* MISC                                                                                 */
/*======================================================================================*/
void misc_init();
void task_create_fail(uint8_t taskid);
void init_queue(uint8_t taskid, uint8_t depth);
void unstuck_i2c_bus(uint8_t port);//possibly deprecated
void system_gentle_reboot();
void handle_missing_sensors();

#define TP0_PIN     GPIO_EXP_NUM_B3
#define TP1_PIN     GPIO_EXP_NUM_B2
#define TP2_PIN     GPIO_EXP_NUM_B0
#define TP3_PIN     GPIO_EXP_NUM_A6
#define TP4_PIN     GPIO_EXP_NUM_A5
#define TP5_PIN     GPIO_EXP_NUM_A4
#define TP10_PIN    GPIO_NUM_36
#define TP11_PIN    GPIO_NUM_35
#define TP_DEF_LVL  0

struct float_mutex_t
{
    float var;
    SemaphoreHandle_t mutex;
};
struct bool_mutex_t
{
    bool var;
    SemaphoreHandle_t mutex;
};


/*======================================================================================*/
/* EXPANDER                                                                             */
/*======================================================================================*/
#define GPIO_EXP_NUM_A0 0
#define GPIO_EXP_NUM_A1 1
#define GPIO_EXP_NUM_A2 2
#define GPIO_EXP_NUM_A3 3
#define GPIO_EXP_NUM_A4 4
#define GPIO_EXP_NUM_A5 5
#define GPIO_EXP_NUM_A6 6
#define GPIO_EXP_NUM_A7 7
#define GPIO_EXP_NUM_B0 8
#define GPIO_EXP_NUM_B1 9
#define GPIO_EXP_NUM_B2 10
#define GPIO_EXP_NUM_B3 11
#define GPIO_EXP_NUM_B4 12
#define GPIO_EXP_NUM_B5 13
#define GPIO_EXP_NUM_B6 14
#define GPIO_EXP_NUM_B7 15

#define EXP_FREQ_HZ         400000
#define EXP_ADDR            0x20
#define EXP_PORT            I2C1_PORT

extern mcp23x17_t exp_dev;
extern uint16_t exp_mask;
void exp_init();
void exp_set_pin(uint8_t pin, bool lvl);


/*======================================================================================*/
/* Actuator                                                                             */
/*======================================================================================*/
void task_actuator_main(void *args);

//membrane
extern t_basic_actuator act_membrane;
#define ACT_MEMB_EN_PIN         GPIO_NUM_42
#define ACT_MEMB_EN_LVL         1
#define ACT_MEMB_DIS_LVL        !(ACT_MEMB_EN_LVL)

//servos
#define ACT_SERV_NUMOF          4
extern servo_config_t serv_cfg;
extern t_servo servos[ACT_SERV_NUMOF];

#define ACT_SERV0_EN_PIN        GPIO_EXP_NUM_B4
#define ACT_SERV1_EN_PIN        GPIO_EXP_NUM_B5
#define ACT_SERV2_EN_PIN        GPIO_EXP_NUM_B6
// #define ACT_SERV3_EN_PIN        GPIO_EXP_NUM_B7
#define ACT_SERV3_EN_PIN        GPIO_EXP_NUM_B2 //temp - TP1

#define ACT_SERV0_PWM_PIN       GPIO_NUM_4
#define ACT_SERV1_PWM_PIN       GPIO_NUM_5
#define ACT_SERV2_PWM_PIN       GPIO_NUM_6
#define ACT_SERV3_PWM_PIN       GPIO_NUM_7

#define ACT_SERV_EN_LVL         1
#define ACT_SERV_DIS_LVL        !(ACT_SERV_EN_LVL)
#define ACT_SERV_FREQ_HZ        50
#define ACT_SERV_MIN_ANGLE_DEG  0
#define ACT_SERV_MAX_ANGLE_DEG  180
#define ACT_SERV_MIN_WIDTH_US   500
#define ACT_SERV_MAX_WIDTH_US   2500
#define ACT_SERV_POS0_DEF       0
#define ACT_SERV_POS1_DEF       ACT_SERV_MAX_ANGLE_DEG

//Stepper motors
#define ACT_STEP_EN_PIN         GPIO_EXP_NUM_A7
#define ACT_STEP_DIR_PIN        GPIO_EXP_NUM_B1

#define ACT_STEP_EN_LVL         0
#define ACT_STEP_DIS_LVL        !(ACT_STEP_EN_LVL)
#define ACT_STEP_DIR_DEF_LVL    0

#define ACT_NTCODE_UPDATE_SERV0             0
#define ACT_NTCODE_UPDATE_SERV1             1
#define ACT_NTCODE_UPDATE_SERV2             2
#define ACT_NTCODE_UPDATE_SERV3             3
#define ACT_NTCODE_UPDATE_MEMB              4
#define ACT_NTCODE_UPDATE_MEMB_NO_COM       5

void act_init();

/*======================================================================================*/
/* Regulator                                                                            */
/*======================================================================================*/
void task_regulator_main(void *args);
void reg_init();
struct t_reg_var
{
    bool enabled;
    float H;
    float SP;
    float PV;
    float E;
    bool CV;
    SemaphoreHandle_t mutex;
};
extern t_reg_var regulator;
#define REG_DEF_VAL     0.0
#define REG_DEF_BOOL    false

#define REG_NTCODE_UPDATE           0
#define REG_NTCODE_UPDATE_NO_COM    1

/*======================================================================================*/
/* Sensor                                                                               */
/*======================================================================================*/
void task_sensor_main(void *args);
void sen_init();

/*SHT35 internal*/
extern t_RHT_sensor *RHT_int;
#define SEN_RHT_INT_ADDR                0x44
#define SEN_RHT_INT_PORT                I2C0_PORT
#define SEN_RHT_INT_PROCEED_CONNFAIL    true //readings critical to the system (lack thereof = not much works)

/*SHT35 external*/
extern t_RHT_sensor *RHT_ext;
#define SEN_RHT_EXT_ADDR                0x44
#define SEN_RHT_EXT_PORT                I2C1_PORT
#define SEN_RHT_EXT_PROCEED_CONNFAIL    true

/*INA219*/
extern t_INA_sensor* CURSEN;
#define SEN_CURSEN_ADDR                 0x40
#define SEN_CURSEN_PORT                 I2C1_PORT
#define SEN_CURSEN_PROCEED_CONNFAIL     true

#define SEN_MISSING_PROCEED_DELAY_S     10

//sensor settings
#define SEN_CURSEN_BUS_VOLT_RANGE   INA219_BUS_RANGE_16V
#define SEN_CURSEN_GAIN             INA219_GAIN_1//+-40mV
#define SEN_CURSEN_U_RES            INA219_RES_12BIT_4S //12bit time for adc, average over 4 samples - about 2.13ms total
#define SEN_CURSEN_I_RES            INA219_RES_12BIT_16S //12bit time for adc, average over 16 samples - about 8.51ms total
#define SEN_CURSEN_WAIT_MS          15
#define SEN_CURSEN_MODE             INA219_MODE_TRIG_SHUNT_BUS //both shunt and bus measured once when triggered
#define SEN_CURSEN_RSHUNT_mOHM      4.0f

#define SEN_MIN_VAL         0.0
#define SEN_RH_MAX_VAL      100.0
#define SEN_T_MAX_VAL       100.0
#define SEN_CUR_MAX_VAL     10.0
#define SEN_VOL_MAX_VAL     5.0
#define SEN_POW_MAX_VAL     30.0

/*ntcodes*/
#define SEN_NTCODE_UPDATE_ALL       0
#define SEN_NTCODE_UPDATE_MEMB      1
#define SEN_NTCODE_UPDATE_RHT_INT   2
#define SEN_NTCODE_UPDATE_RHT_EXT   3

/*======================================================================================*/
/* Comm                                                                                 */
/*======================================================================================*/
void task_comm_main(void *args);
void com_init();

#define COM_UART_BAUDRATE       115200
#define COM_UART_PORT           UART_NUM_0
extern uint8_t *uart_buffer;
extern uint16_t uart_buffer_idx;

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

#define JT_STA              "sta"
#define JSON_STA_EN         "SS"
#define JSON_STA_LASER      "LS"

#define JSON_ON             "ON"
#define JSON_OFF            "OFF"

#define UART_QUEUE_DEPTH    6

#define COM_NTCODE_SEND_SEN 0
#define COM_NTCODE_SEND_REG 1
#define COM_NTCODE_SEND_STA 2
#define COM_NTCODE_SENDALL  4

/*======================================================================================*/
/* Starter                                                                              */
/*======================================================================================*/

extern bool_mutex_t starter_en;
void starter_init(); //in misc.cpp for now

/*======================================================================================*/
/* LCD                                                                                  */
/*======================================================================================*/
void task_lcd_main(void *args);

void lcd_init();
struct lcd_settings_t{
    SemaphoreHandle_t mutex;
    bool ss_enabled;
    bool ss_state;
    float ss_delay;
    float brightness;
};
extern lcd_settings_t lcd_settings;
extern esp_lcd_panel_handle_t lcd_handle;
extern esp_lcd_panel_io_handle_t lcd_io_handle;
bool lcd_flushed_isr(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx);

#define LCD_HOST                SPI2_HOST
#define LCD_CLOCK_HZ            (10 * 1000 * 1000) //10MHz
#define LCD_DIN_PIN             GPIO_NUM_10 //din
#define LCD_CLK_PIN             GPIO_NUM_9 //clk
#define LCD_CS_PIN              GPIO_NUM_8 //cs
#define LCD_DC_PIN              GPIO_NUM_18 //dc
#define LCD_RST_PIN             GPIO_NUM_17 //rst
#define LCD_HRES                240 
#define LCD_VRES                320
#define LCD_NUM_PX              (LCD_HRES*LCD_VRES)
#define LCD_BITS_PX             16
#define LCD_CMD_BITS            8
#define LCD_PARAM_BITS          8

//widths (after rotation)
#define LCD_WIDTH               LCD_VRES
#define LCD_HEIGHT              LCD_HRES

#define LCD_BL_PIN              GPIO_NUM_16 //bl
#define LCD_BL_LEDC_CHANNEL     LEDC_CHANNEL_4
#define LCD_BL_LEDC_TIMER       LEDC_TIMER_1
#define LCD_BL_LEDC_MODE        LEDC_LOW_SPEED_MODE
#define LCD_BL_FREQ_HZ          1000
#define LCD_BL_DUTY_ON          1023
#define LCD_BL_DUTY_OFF         0

#define LCD_DEF_BRIGHT          50
#define LCD_MAX_BRIGHT          100
#define LCD_MIN_BRIGHT          5

#define LCD_SS_DEF_DELAY_S      5
#define LCD_SS_MIN_DELAY_S      3
#define LCD_SS_MAX_DELAY_S      120
#define LCD_SS_DEF_ENABLED      true

/*Notification Codes*/
#define LCD_NTCODE_UPDATE_SETTINGS          0
#define LCD_NTCODE_ACTIVATE_SCREENSAVER     1
#define LCD_NTCODE_DEACTIVATE_SCREENSAVER   2

/*======================================================================================*/
/* UI ROTATIONAL ENCODER                                                                */
/*======================================================================================*/
void task_encoder_main(void *args);
#define ENC_CLK_PIN                 GPIO_NUM_13
#define ENC_DT_PIN                  GPIO_NUM_14
#define ENC_SW_PIN                  GPIO_NUM_12
#define ENC_PCNT_HIGH               2      //after reaching that many steps a one "big step" is registered
#define ENC_PCNT_LOW                (-1)*ENC_PCNT_HIGH
void enc_gpio_init();
void enc_pnct_init();

/*Notification Codes*/
#define ENC_NTCODE_ROT              0
#define ENC_NTCODE_SW_CLICK         1
#define ENC_NTCODE_SW_PRESSED       2

/*======================================================================================*/
/* GUI                                                                                  */
/*======================================================================================*/
void task_gui_main(void *args);

extern gui_controller_t *gui;

void gui_init();

/*Notification Codes*/
#define GUI_NTCODE_CUR_POS      0
#define GUI_NTCODE_CUR_NEG      1
#define GUI_NTCODE_CUR_ENT      2
#define GUI_NTCODE_CUR_BCK      3
#define GUI_NTCODE_UPDATE_PAGE  4

/*======================================================================================*/
/* NVS                                                                                  */
/*======================================================================================*/
//non volatile storage
#define NVS_SPACE_NAME              "NVS_SPACE"
#define NVS_SAVE_PERIOD_LOOPS       60
#define NVS_FLOAT_COMPARE_EPSILON   0.001f

extern nvs_handle_t nvs;
void init_nvs();
float nvs_get_float(const char* key, float def);
bool nvs_get_bool(const char* key, bool def);
void nvs_save_values();

/*keys defined in nvs.cpp*/
extern const char* NVS_SERVOS[ACT_SERV_NUMOF][2];
extern const char* NVS_REG_H;
extern const char* NVS_REG_SP;
extern const char* NVS_COM_PERIOD;
extern const char* NVS_GUI_SS_DELAY;
extern const char* NVS_GUI_SS_EN;

/*======================================================================================*/
/* LVGL                                                                                 */
/*======================================================================================*/
#define LVGL_COLOR_DEPTH          16  // RGB565
#define LVGL_USE_ANIMATION        0
#define LVGL_USE_LABEL            1   // Only enable what you need
#define LVGL_DRAW_SW_COMPLEX      0

#define LCD_BUF_LINES            40
extern lv_color_t lvgl_buffer[LCD_WIDTH*LCD_BUF_LINES];