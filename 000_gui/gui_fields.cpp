#include "../000_gui.h"
char gui_char_buf[16];



//===============================================
// io_field
//===============================================

gui_io_field_t::gui_io_field_t(
    gui_field_type_t _field_type, 
    task_notify_pack_t *_ntpack, 
    SemaphoreHandle_t _mutex, 
    lv_obj_t* parrent,
    uint16_t _pos_x,
    uint16_t _pos_y
):gui_generic_field_t(_field_type), ntpack(_ntpack), mutex(_mutex){}

//===============================================
// sw bool
//===============================================

gui_sw_bool_field_t::gui_sw_bool_field_t(
    task_notify_pack_t *_ntpack, 
    SemaphoreHandle_t _mutex, 
    bool *var_ptr,
    lv_obj_t* parrent,
    uint16_t _pos_x,
    uint16_t _pos_y
):gui_io_field_t(gui_field_type_t::SW_BOOL, _ntpack, _mutex, parrent, _pos_x, _pos_y)
{
    indicator=lv_switch_create(parrent);
    
    lv_obj_set_pos(indicator, _pos_x, _pos_y);
    lv_obj_set_style_bg_color(indicator, GUI_COLOR_SW_OFF, (uint32_t)LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(indicator, GUI_COLOR_SW_ON, (uint32_t)LV_PART_MAIN|LV_STATE_CHECKED);
    lv_obj_set_style_bg_color(indicator, GUI_COLOR_SW_KNOB, LV_PART_KNOB);

    if(*var) lv_obj_add_state(indicator, LV_STATE_CHECKED);
}

void gui_sw_bool_field_t::toggle()
{
    if(*var)
    {
        *var=false;
        lv_obj_remove_state(indicator, LV_STATE_CHECKED);
    }
    else
    {
        *var=true;
        lv_obj_add_state(indicator, LV_STATE_CHECKED);
    }
}
void gui_sw_bool_field_t::set_val(bool val)
{
    if(*var==val) return;
    else toggle();
}

void gui_sw_bool_field_t::select_field()
{
    lv_obj_set_style_bg_color(indicator, GUI_COLOR_SELECT, LV_PART_KNOB);
}
void gui_sw_bool_field_t::unselect_field()
{
    lv_obj_set_style_bg_color(indicator, GUI_COLOR_SW_KNOB, LV_PART_KNOB);
}

//===============================================
// float
//===============================================
gui_float_field_t::gui_float_field_t(
    task_notify_pack_t *_ntpack, 
    SemaphoreHandle_t _mutex, 
    float *var_ptr,
    lv_obj_t* parrent,
    uint16_t _pos_x,
    uint16_t _pos_y,
    lv_color_t _def_color,
    char _unit,
    uint8_t _float_prec
):gui_io_field_t(gui_field_type_t::FLOAT, _ntpack, _mutex, parrent, _pos_x, _pos_y), var(var_ptr), def_color(_def_color), float_prec(_float_prec)
{
    in_digit=false;
    digit_index=0;


    indicator=lv_label_create(parrent);
    lv_obj_set_style_text_color(indicator, def_color, 0);
    lv_obj_set_style_text_font(indicator, &lv_font_montserrat_28, 0);
    lv_obj_set_pos(indicator, _pos_x, _pos_y);
    
    unit=lv_label_create(parrent);
    lv_obj_set_style_text_color(unit, GUI_COLOR_TEXT, 0);
    char temp[]={_unit, '\0'};
    lv_label_set_text(unit, temp);

    this->update_state();
}

void gui_float_field_t::select_field()
{
    lv_obj_set_style_text_color(indicator, GUI_COLOR_SELECT, 0);
}

void gui_float_field_t::unselect_field()
{
    lv_obj_set_style_text_color(indicator, def_color, 0);
}

void gui_float_field_t::update_state()
{
    this->float_to_string(float_prec);
    lv_label_set_text(indicator, gui_char_buf);
    lv_obj_update_layout(indicator);

    lv_obj_align_to(unit, indicator, LV_ALIGN_OUT_RIGHT_BOTTOM, GUI_UNIT_OFFSET_PX, 0);
}

char* gui_float_field_t::float_to_string(uint8_t precision)
{
    if(precision>GUI_FLOAT_PRECISION_MAX) precision=GUI_FLOAT_PRECISION_MAX;
    snprintf(gui_char_buf, sizeof(gui_char_buf), "%.*f", precision, *var);
    return gui_char_buf;
}

//===============================================
// back button
//===============================================

gui_back_field_t::gui_back_field_t(lv_obj_t* parrent)
:gui_generic_field_t(gui_field_type_t::BACK_BTN)
{
    back_button=lv_label_create(parrent);
    lv_obj_add_style(back_button, list_style_def, LV_STATE_DEFAULT);
    lv_obj_add_style(back_button, list_style_sel, LV_STATE_CHECKED);
    lv_label_set_text(back_button, LV_SYMBOL_LEFT);
    lv_obj_set_pos(back_button, 0, 0);
}