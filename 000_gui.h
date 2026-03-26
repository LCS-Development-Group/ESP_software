#pragma once
#include "common_includes.h"

/*class definitions in 000_gui/ directory*/

/*global values (colors, sizes, etc) in gui_fields.cpp*/

enum gui_field_type_t{GUI_SW_BOOL, GUI_SW_POS, FLOAT_OUT, FLOAT_IN, INT16, TEXT};
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
     gui_generic_field_t(gui_field_type_t _field_type, task_notify_pack_t *_ntpack);
    ~gui_generic_field_t();

    //getters
    gui_field_type_t get_field_type() const {return field_type;}
    task_notify_pack_t* get_ntpack_ptr() const {return ntpack;}
    SemaphoreHandle_t get_mutex() const {return mutex;}
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
        gui_field_type_t _field_type, 
        task_notify_pack_t *_ntpack,
        lv_obj_t* screen,
        bool *_state_var_ptr,
        std::string _text,
        uint16_t _pos_x,
        uint16_t _pos_y
    );
    ~gui_sw_field_t();

    //setters
    bool set_state(bool new_state);
    bool switch_state();

    //getters - mutex taken externally
    bool get_state() const {return *state;};
    bool *get_state_var_ptr() {return state;}
};

//===============================================
// Page
//===============================================

class gui_page_t
{


    public:
    gui_page_t();
    ~gui_page_t();
};

//===============================================
// Controller
//===============================================

class gui_controller_t
{

};