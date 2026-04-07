#pragma once
#include "common_includes.h"
/*class definitions in 000_gui/ directory*/

/*global values (colors, sizes, etc)*/
inline lv_color_t GUI_COLOR_TEXT        =lv_color_hex(0xE1E1E1);
inline lv_color_t GUI_COLOR_SELECT      =lv_color_hex(0x8B0000);//DarkRed
inline lv_color_t GUI_COLOR_PAGE_BG     =lv_color_hex(0x000000);//Black
inline lv_color_t GUI_COLOR_TILE_BG     =lv_color_hex(0x1c1c1c);
inline lv_color_t GUI_COLOR_TILE_IN     =lv_color_hex(0x333333);

inline lv_color_t GUI_COLOR_RH_INT      =lv_color_hex(0x1e90ff);//DodgerBlue
inline lv_color_t GUI_COLOR_T_INT       =lv_color_hex(0xffa500);//Orange
inline lv_color_t GUI_COLOR_RH_EXT      =lv_color_hex(0x6a5acd);//SlateBlue
inline lv_color_t GUI_COLOR_T_EXT       =lv_color_hex(0xb22222);//fireBrick
inline lv_color_t GUI_COLOR_SP          =lv_color_hex(0x32cd32);//LimeGreen

inline lv_color_t GUI_COLOR_SW_KNOB     =lv_color_hex(0xE1E1E1);
inline lv_color_t GUI_COLOR_SW_ON       =lv_color_hex(0x1e90ff);//DodgerBlue
inline lv_color_t GUI_COLOR_SW_OFF      =lv_color_hex(0x666666);

#define GUI_FONT28_HEIGHT                   28
#define GUI_FONT24_HEIGHT                   24
#define GUI_FONT20_HEIGHT                   20
#define GUI_FONT14_HEIGHT                   14

#define GUI_LABEL_OBJ_PADDING               5
#define GUI_TILE_OBJECT_PADDING             10
#define GUI_BACKPLATE_OBJECT_PADDING        5
#define GUI_TILE_CORNER_RADIUS              8
#define GUI_SW_WIDTH                        40
#define GUI_SW_HEIGHT                       20
#define GUI_FLOAT_FIELD_DEF_VAL             "-88.88"

#define GUI_FLOAT_PRECISION_MAX             3
#define GUI_UNIT_OFFSET_PX                  8

#define GUI_MENU_ENTRY_NUM                  6
#define GUI_MENU_ENTRY_SPACING_PX           40
#define GUI_MENU_ENTRY_WIDTH_PX             120

#define GUI_EDIT_AFTER_DOT                  3
#define GUI_EDIT_BEFORE_DOT                 3
#define GUI_EDIT_INDICATORS_Y               100
#define GUI_EDIT_INDICATOR_MON28_W          (2*GUI_BACKPLATE_OBJECT_PADDING+18)
#define GUI_EDIT_INDICATOR_MON28_H          (2*GUI_BACKPLATE_OBJECT_PADDING+28)
#define GUI_EDIT_TOTAL_DIGITS               (GUI_EDIT_BEFORE_DOT+GUI_EDIT_AFTER_DOT)
#define GUI_EDIT_BACK_IDX                   255

extern lv_style_t *gui_style_menu_def;
extern lv_style_t *gui_style_menu_sel;
extern lv_style_t *gui_style_bg_tile;
void gui_setup_global_styles();
extern char gui_char_buf[16];//this bad

enum gui_field_type_t{SW_BOOL, SW_POS, FLOAT, INT16, TEXT, BACK_BTN};
struct task_notify_pack_t
{
    QueueHandle_t task_queue;
    uint8_t ntcode;
    task_notify_pack_t(
        QueueHandle_t _task_queue, uint8_t _ntcode)
        :task_queue(_task_queue), ntcode(_ntcode){}
};

//===============================================
// Generic Field
//===============================================

class gui_generic_field_t
{
protected:
    gui_field_type_t field_type;
    lv_obj_t *indicator;

    task_notify_pack_t *ntpack;
    SemaphoreHandle_t mutex;

public:
    gui_generic_field_t(
        gui_field_type_t _field_type, 
        task_notify_pack_t *_ntpack, 
        SemaphoreHandle_t _mutex);

    virtual ~gui_generic_field_t(){}

    //getters
    gui_field_type_t get_field_type() const {return field_type;}
    
    virtual void select_field()=0;
    virtual void unselect_field()=0;

    task_notify_pack_t* get_ntpack_ptr() const {return ntpack;}
    uint8_t get_ntpack_ntcode() const {return ntpack->ntcode;}
    QueueHandle_t get_ntpack_task_queue() const {return ntpack->task_queue;}

    SemaphoreHandle_t get_mutex() const {return mutex;}
    virtual void update_state(){};
    void send_ntcode()
    {
        if(ntpack==nullptr) return;
        uint8_t sendval=ntpack->ntcode;
        xQueueSend(ntpack->task_queue, &sendval, 0);
    }
};


class gui_sw_bool_field_t: public gui_generic_field_t
{
    bool *var;
    bool color_states;

public:
    gui_sw_bool_field_t(
        task_notify_pack_t *_ntpack, 
        SemaphoreHandle_t _mutex, 
        bool *var_ptr,
        lv_obj_t* parrent,
        uint16_t offset_x,
        uint16_t offset_y,
        bool _color_states
    );

    bool get_var() const {return *var;}
    bool* get_var_ptr() {return var;}

    void toggle();
    void set_val(bool val);

    void select_field() override;
    void unselect_field() override;
    void update_state() override {}// ? why is it an empty override?
};

class gui_float_field_t: public gui_generic_field_t
{
    char gui_char_buf[16];
    float *var;

    lv_color_t def_color;
    lv_obj_t *unit;
    const char *unit_ptr;
    const char *name;
    uint8_t float_prec;

public:
    gui_float_field_t(
        task_notify_pack_t *_ntpack, 
        SemaphoreHandle_t _mutex, 
        float *var_ptr,
        lv_obj_t* parrent,
        uint16_t offset_x,
        uint16_t offset_y,
        lv_color_t _def_color,
        const char *_unit,
        const char *_name,
        uint8_t _float_prec
    );

    float get_var() const {return *var;}
    float* get_var_ptr() {return var;}


    void select_field() override;
    void unselect_field() override;
    void update_state() override;
    const char *get_unit_ptr() {return unit_ptr;}
    const char *get_name() {return name;}
private:
    void float_to_string();
};

class gui_back_field_t: public gui_generic_field_t
{
    lv_obj_t *back_button;
public:
    gui_back_field_t(lv_obj_t* parrent);
    void select_field() override {lv_obj_add_state(back_button, LV_STATE_CHECKED);}
    void unselect_field() override {lv_obj_remove_state(back_button, LV_STATE_CHECKED);}
};

//===============================================
// Editor
//===============================================

class gui_editor_t
{
    gui_float_field_t *field_ptr;
    float edited_value;
    std::vector <uint8_t> digits;
    uint8_t prim_idx, sec_idx;
    bool in_digit;

    lv_obj_t *screen;
    lv_style_t *indicator_style;
    lv_style_t *indicator_style_sel;
    lv_style_t *indicator_style_lock;
    std::vector <lv_obj_t*> indicators; 
    lv_obj_t *tile;
    lv_obj_t *dot;
    lv_obj_t *unit;
    lv_obj_t *name;
    gui_back_field_t *back_button;

public:
    gui_editor_t();

    void start_edit(gui_float_field_t *_field_ptr);
    void save_edit();
    
    void cmd_next();
    void cmd_prev();
    bool cmd_enter(); //true - resulted in leaving editor

private:

};

//===============================================
// Page
//===============================================

class gui_page_t
{
    std::vector <gui_generic_field_t*> selectable;
    std::vector <gui_generic_field_t*> unselectable;
    std::vector <lv_obj_t *> deco;

    lv_obj_t *screen;
    lv_obj_t *menu_label;
    lv_obj_t *name_label;
    std::string name;

    uint8_t field_index;
    bool in_field;

    gui_editor_t *editor_ptr;

    public:
    gui_page_t(
        std::string _name,
        lv_obj_t* main_screen,
        gui_editor_t *_editor_ptr,
        void (*init_func)(
            std::vector <gui_generic_field_t*>*,
            std::vector <gui_generic_field_t*>*,
            std::vector <lv_obj_t *>*,
            lv_obj_t*
    ));
    ~gui_page_t(){}

    void unload_page();
    void load_page();

    void cmd_next();
    void cmd_prev();
    bool cmd_enter();
    bool cmd_back();
    void cmd_update_page();

//getters
    lv_obj_t* get_screen_ptr() {return screen;}
    lv_obj_t* get_menu_label_ptr() {return menu_label;}
    std::string get_name() const {return name;}

    uint8_t get_numof_selectable() const {return selectable.size();}
    gui_generic_field_t *get_selectable_field(uint8_t idx);

    uint8_t get_numof_unselectable() const {return unselectable.size();}
    gui_generic_field_t *get_unselectable_field(uint8_t idx);
};


//===============================================
// Controller
//===============================================

class gui_controller_t
{
    SemaphoreHandle_t mutex;
    std::vector <gui_page_t*> page_list;

    uint8_t page_index;
    bool in_page;

    lv_obj_t *screen;
    uint8_t list_top;
    lv_obj_t *header;
    lv_obj_t *soft_ver;

    gui_editor_t *editor_ptr;

public:
    gui_controller_t();
    ~gui_controller_t();


    void cmd_next();
    void cmd_prev();
    void cmd_enter();
    void cmd_back();

    void cmd_update_page();

//getters
    SemaphoreHandle_t get_mutex() const {return mutex;}
    uint8_t get_page_index() const {return page_index;}

private:
    void show_list();
    void select_next_list_entry();
    void select_prev_list_entry();

    void fill_gui();
    void enter_page();
    void leave_page();
};


//menu page init functions definitions
void gui_init_page_test(std::vector <gui_generic_field_t*>* selectable,
    std::vector <gui_generic_field_t*>* unselectable,
    std::vector <lv_obj_t *>* deco,
    lv_obj_t* screen);

void gui_init_page_readings(std::vector <gui_generic_field_t*>* selectable,
    std::vector <gui_generic_field_t*>* unselectable,
    std::vector <lv_obj_t *>* deco,
    lv_obj_t* screen);

void gui_init_page_servos(std::vector <gui_generic_field_t*>* selectable,
    std::vector <gui_generic_field_t*>* unselectable,
    std::vector <lv_obj_t *>* deco,
    lv_obj_t* screen);

void gui_init_page_placeholder(std::vector <gui_generic_field_t*>* selectable,
    std::vector <gui_generic_field_t*>* unselectable,
    std::vector <lv_obj_t *>* deco,
    lv_obj_t* screen);