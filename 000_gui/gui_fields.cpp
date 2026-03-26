#include "../000_gui.h"

//===============================================
// generic
//===============================================

const lv_color_t GUI_LV_TEXT_COLOR          =lv_color_hex(0x9B9B9B);
const lv_color_t GUI_LV_FIELD_BG_COLOR      =lv_color_hex(0x1C1C1C);
const lv_color_t GUI_LV_PAGE_BG_COLOR       =lv_color_hex(0x000000);
const lv_color_t GUI_LV_SW_BG_ON            =lv_color_hex(0x1e90ff);
const lv_color_t GUI_LV_SW_BG_OFF           =lv_color_hex(0x666666);
const lv_color_t GUI_LV_SW_KNOB             =lv_color_hex(0xffffff);
#define LABEL_OBJ_PADDING   5

gui_generic_field_t::gui_generic_field_t(gui_field_type_t _field_type, task_notify_pack_t *_ntpack)
:field_type(_field_type), ntpack(_ntpack)
{
    mutex=xSemaphoreCreateMutex();
    if(mutex==NULL) exit(-1);
}
    
gui_generic_field_t::~gui_generic_field_t()
{
    if(ntpack!=nullptr) delete ntpack;
}

//===============================================
// sw bool
//===============================================

gui_sw_field_t::gui_sw_field_t(
    gui_field_type_t _field_type, 
    task_notify_pack_t *_ntpack,
    lv_obj_t* screen,
    bool *_state_var_ptr,
    std::string _text,
    uint16_t _pos_x,
    uint16_t _pos_y
):gui_generic_field_t(_field_type, _ntpack), state(_state_var_ptr), text(_text), pos_x(_pos_x), pos_y(_pos_y)
{
    if(state==nullptr) exit(-1);


    label=lv_label_create(screen);
    lv_label_set_text(label, text.c_str());
    lv_obj_set_style_text_color(label, GUI_LV_TEXT_COLOR, 0);
    lv_obj_set_pos(label, pos_x, pos_y);

    lv_obj_update_layout(label);//for size calculation

    sw=lv_switch_create(screen);
    lv_obj_set_pos(sw, pos_x, pos_y+lv_obj_get_height(label)+LABEL_OBJ_PADDING);

    lv_obj_set_style_bg_color(sw, GUI_LV_SW_BG_OFF, (uint32_t)LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(sw, GUI_LV_SW_BG_ON, (uint32_t)LV_PART_MAIN|LV_STATE_CHECKED);
    lv_obj_set_style_bg_color(sw, GUI_LV_SW_KNOB, LV_PART_KNOB);

    if(*state) lv_obj_add_state(sw, LV_STATE_CHECKED);

}

bool gui_sw_field_t::set_state(bool new_state)
{
    if(new_state!=*state)
    {
        *state=new_state;
        if(new_state) lv_obj_add_state(sw, LV_STATE_CHECKED);
        else lv_obj_remove_state(sw, LV_STATE_CHECKED);
        return true;
    }
    else return false;
}
bool gui_sw_field_t::switch_state()
{
    if(*state)
    {
        *state=false;
        lv_obj_remove_state(sw, LV_STATE_CHECKED);
    }
    else
    {
        *state=true;
        lv_obj_add_state(sw, LV_STATE_CHECKED);
    }
    return true;
}