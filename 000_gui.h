#pragma once
#include "common_includes.h"

/*class definitions in 000_gui/ directory*/

/*global values (colors, sizes, etc)*/
inline lv_color_t GUI_LV_TEXT_COLOR         =lv_color_hex(0x9B9B9B);
inline lv_color_t GUI_LV_FIELD_BG_COLOR     =lv_color_hex(0x1C1C1C);
inline lv_color_t GUI_LV_SELECT             =lv_color_hex(0x8B0000);
inline lv_color_t GUI_LV_PAGE_BG_COLOR      =lv_color_hex(0x000000);
inline lv_color_t GUI_LV_SW_BG_ON           =lv_color_hex(0x1e90ff);
inline lv_color_t GUI_LV_SW_BG_OFF          =lv_color_hex(0x666666);
inline lv_color_t GUI_LV_SW_KNOB            =lv_color_hex(0xffffff);
#define LABEL_OBJ_PADDING                   10

#define GUI_MENU_ENTRY_NUM                  6
#define GUI_MENU_ENTRY_SPACING_PX           40
#define GUI_MENU_ENTRY_WIDTH_PX             160

extern lv_style_t *list_style_def;
extern lv_style_t *list_style_sel;

enum gui_field_type_t{SW_BOOL, SW_POS, FLOAT_OUT, FLOAT_IN, INT16, TEXT, BACK_BTN};
struct task_notify_pack_t
{
    uint8_t task_id;
    uint8_t ntcode;
    task_notify_pack_t(
        uint8_t _task_id,uint8_t _ntcode)
        :task_id(_task_id), ntcode(_ntcode){}
};

//===============================================
// Generic Field
//===============================================

class gui_generic_field_t
{
protected:
    gui_field_type_t field_type;
    task_notify_pack_t *ntpack;
    SemaphoreHandle_t mutex;

public:
     gui_generic_field_t(gui_field_type_t _field_type, task_notify_pack_t *_ntpack, SemaphoreHandle_t _mutex);
    virtual ~gui_generic_field_t();

    //getters
    gui_field_type_t get_field_type() const {return field_type;}
    task_notify_pack_t* get_ntpack_ptr() const {return ntpack;}
    SemaphoreHandle_t get_mutex() const {return mutex;}

    virtual void select_field()=0;
    virtual void unselect_field()=0;
};

class gui_sw_field_t :public gui_generic_field_t
{
    lv_obj_t *sw;
    lv_obj_t *label;
    lv_obj_t *bg;
    bool *state;
    std::string text;

    uint16_t pos_x;
    uint16_t pos_y;

public:
    gui_sw_field_t(
        task_notify_pack_t *_ntpack,
        lv_obj_t* screen,
        bool *_state_var_ptr,
        SemaphoreHandle_t _mutex,
        std::string _text,
        uint16_t _pos_x,
        uint16_t _pos_y
    );

    //setters
    bool set_state(bool new_state);
    bool switch_state();
    void select_field() override {lv_obj_set_style_bg_color(sw, GUI_LV_SELECT, LV_PART_KNOB);}//think how to best show that
    void unselect_field() override {lv_obj_set_style_bg_color(sw, GUI_LV_SW_KNOB, LV_PART_KNOB);}

    //getters - mutex taken externally
    bool get_state() const {return *state;};
    bool *get_state_var_ptr() {return state;}
};

class gui_back_field_t: public gui_generic_field_t
{
    lv_obj_t *back_button;
public:
    gui_back_field_t(lv_obj_t* screen);
    void select_field() override {lv_obj_add_state(back_button, LV_STATE_CHECKED);}
    void unselect_field() override {lv_obj_remove_state(back_button, LV_STATE_CHECKED);}
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

    public:
    gui_page_t(
        std::string _name,
        lv_obj_t* main_screen,
        void (*init_func)(
            std::vector <gui_generic_field_t*>*,
            std::vector <gui_generic_field_t*>*,
            std::vector <lv_obj_t *>*,
            lv_obj_t*
    ));
    ~gui_page_t();


//getters
    lv_obj_t* get_screen_ptr() {return screen;}
    lv_obj_t* get_menu_label_ptr() {return menu_label;}
    std::string get_name() const {return name;}
    uint8_t get_numof_selectable() const {return selectable.size();}

    gui_generic_field_t *get_selectable_field(uint8_t idx);
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

    uint8_t field_index;
    bool in_field;

    lv_obj_t *screen;
    uint8_t list_top;

public:
    gui_controller_t();
    ~gui_controller_t();


    void cmd_next();
    void cmd_prev();
    void cmd_enter();
    void cmd_back();


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