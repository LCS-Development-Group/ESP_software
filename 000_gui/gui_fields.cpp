#include "../000_gui.h"

//===============================================
// generic field
//===============================================

gui_generic_field_t::gui_generic_field_t(
    gui_field_type_t _field_type, 
    task_notify_pack_t *_ntpack, 
    SemaphoreHandle_t _mutex
):field_type(_field_type), ntpack(_ntpack), mutex(_mutex){}

//===============================================
// sw bool
//===============================================

gui_sw_bool_field_t::gui_sw_bool_field_t(
    task_notify_pack_t *_ntpack, 
    SemaphoreHandle_t _mutex, 
    bool *var_ptr,
    lv_obj_t* parrent,
    uint16_t offset_x,
    uint16_t offset_y,
    bool _color_states
):gui_generic_field_t(gui_field_type_t::SW_BOOL, _ntpack, _mutex), var(var_ptr), color_states(_color_states)
{
    indicator=lv_switch_create(parrent);
    
    lv_obj_set_pos(indicator, offset_x, offset_y);
    if(color_states)
    {
        lv_obj_set_style_bg_color(indicator, GUI_COLOR_SW_OFF, (uint32_t)LV_PART_MAIN|LV_STATE_DEFAULT);
        lv_obj_set_style_bg_color(indicator, GUI_COLOR_SW_ON, (uint32_t)LV_PART_MAIN|LV_STATE_CHECKED);
    }
    else lv_obj_set_style_bg_color(indicator, GUI_COLOR_SW_OFF, LV_PART_MAIN);
    
    lv_obj_set_style_bg_color(indicator, GUI_COLOR_SW_KNOB, LV_PART_KNOB);
    lv_obj_set_size(indicator, GUI_SW_WIDTH, GUI_SW_HEIGHT);

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
    send_ntcode();
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
    uint16_t offset_x,
    uint16_t offset_y,
    lv_color_t _def_color,
    const char *_unit,
    const char *_name,
    uint8_t _float_prec
):gui_generic_field_t(gui_field_type_t::FLOAT, _ntpack, _mutex), 
var(var_ptr), def_color(_def_color), unit_ptr(_unit), name(_name), float_prec(_float_prec)
{
    if(float_prec>GUI_FLOAT_PRECISION_MAX) float_prec=GUI_FLOAT_PRECISION_MAX;

    indicator=lv_label_create(parrent);
    lv_obj_set_style_text_font(indicator, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(indicator, def_color, 0);
    lv_label_set_text(indicator, GUI_FLOAT_FIELD_DEF_VAL);

    //bg
    lv_obj_set_style_bg_color(indicator, GUI_COLOR_TILE_IN, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(indicator, LV_OPA_COVER, 0);
    lv_obj_set_style_pad_all(indicator, GUI_BACKPLATE_OBJECT_PADDING, 0);
    lv_obj_set_style_radius(indicator, GUI_TILE_CORNER_RADIUS, 0);

    //fixed size
    lv_obj_update_layout(indicator);
    lv_obj_set_size(indicator, lv_obj_get_width(indicator), lv_obj_get_height(indicator));
    lv_obj_align(indicator, LV_ALIGN_TOP_LEFT, offset_x, offset_y);
    
    //unit
    lv_obj_update_layout(indicator);
    unit=lv_label_create(parrent);
    lv_obj_set_style_text_color(unit, GUI_COLOR_TEXT, 0);
    lv_label_set_text(unit, unit_ptr);
    lv_obj_align_to(unit, indicator, LV_ALIGN_OUT_RIGHT_MID, GUI_UNIT_OFFSET_PX, 0);
    this->update_state();
}

void gui_float_field_t::select_field()
{
    lv_obj_set_style_text_color(indicator, GUI_COLOR_TEXT, 0);
    lv_obj_set_style_bg_color(indicator, GUI_COLOR_SELECT, 0);
}

void gui_float_field_t::unselect_field()
{
    lv_obj_set_style_text_color(indicator, def_color, 0);
    lv_obj_set_style_bg_color(indicator, GUI_COLOR_TILE_IN, 0);
}

void gui_float_field_t::update_state()
{
    this->float_to_string();
    lv_label_set_text(indicator, gui_char_buf);
    lv_obj_update_layout(indicator);

    lv_obj_align_to(unit, indicator, LV_ALIGN_OUT_RIGHT_BOTTOM, GUI_UNIT_OFFSET_PX, 0);
}

void gui_float_field_t::float_to_string()
{
    snprintf(gui_char_buf, sizeof(gui_char_buf), "%.*f", float_prec, *var);
}

//===============================================
// back button
//===============================================

gui_back_field_t::gui_back_field_t(lv_obj_t* parrent)
:gui_generic_field_t(gui_field_type_t::BACK_BTN, nullptr, NULL)
{
    back_button=lv_label_create(parrent);
    lv_obj_add_style(back_button, gui_style_menu_def, LV_STATE_DEFAULT);
    lv_obj_add_style(back_button, gui_style_menu_sel, LV_STATE_CHECKED);
    lv_label_set_text(back_button, LV_SYMBOL_LEFT);
    lv_obj_set_pos(back_button, 0, 0);
}