#include "../000_gui.h"

gui_editor_t::gui_editor_t()
{

    /*Screen*/
    screen=lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen, GUI_COLOR_PAGE_BG, 0);

    /*styles*/
    indicator_style=new lv_style_t;
    lv_style_init(indicator_style);
    indicator_style_sel=new lv_style_t;
    lv_style_init(indicator_style_sel);
    indicator_style_lock=new lv_style_t;
    lv_style_init(indicator_style_lock);

    lv_style_set_text_font(indicator_style, &lv_font_montserrat_28);
    lv_style_set_text_color(indicator_style, GUI_COLOR_TEXT);
    lv_style_set_bg_color(indicator_style, GUI_COLOR_TILE_IN);
    lv_style_set_bg_opa(indicator_style, LV_OPA_COVER);
    lv_style_set_pad_all(indicator_style, GUI_BACKPLATE_OBJECT_PADDING);
    lv_style_set_radius(indicator_style, GUI_TILE_CORNER_RADIUS);

    lv_style_set_bg_color(indicator_style_sel, GUI_COLOR_SELECT);

    lv_style_set_bg_color(indicator_style_lock, GUI_COLOR_TEXT);
    lv_style_set_text_color(indicator_style_lock, GUI_COLOR_SELECT);

    /*backplate*/
    tile=lv_obj_create(screen);
    lv_obj_add_style(tile, gui_style_bg_tile, LV_STATE_DEFAULT);
    lv_obj_set_size(tile, 
        GUI_EDIT_TOTAL_DIGITS*(GUI_EDIT_INDICATOR_MON28_W+GUI_BACKPLATE_OBJECT_PADDING)+2*GUI_BACKPLATE_OBJECT_PADDING+3*GUI_TILE_OBJECT_PADDING, 
        GUI_EDIT_INDICATOR_MON28_H+2*GUI_TILE_OBJECT_PADDING
    );
    lv_obj_align(tile, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(tile, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(tile, LV_SCROLLBAR_MODE_OFF);

    /*Dot*/
    dot=lv_label_create(tile);
    lv_obj_set_style_text_font(dot, &lv_font_montserrat_28, 0);
    lv_obj_set_style_text_color(dot, GUI_COLOR_TEXT, 0);
    lv_label_set_text(dot, ".");
    lv_obj_align(dot, LV_ALIGN_CENTER, -(GUI_TILE_OBJECT_PADDING+GUI_BACKPLATE_OBJECT_PADDING)/2, 0);

    /*Unit*/
    unit=lv_label_create(tile);
    lv_obj_set_style_text_color(unit, GUI_COLOR_TEXT, 0);

    /*Name*/
    name=lv_label_create(screen);
    lv_obj_set_style_text_color(name, GUI_COLOR_TEXT, 0);

    lv_obj_t* indicator;
    uint8_t x=0;
    for(uint8_t i=0; i<GUI_EDIT_BEFORE_DOT; i++)
    {
        indicator=lv_label_create(tile);
        lv_obj_add_style(indicator, indicator_style, LV_STATE_DEFAULT);
        lv_obj_add_style(indicator, indicator_style_sel, LV_STATE_CHECKED);
        lv_obj_add_style(indicator, indicator_style_lock, LV_STATE_USER_1);
        lv_obj_set_size(indicator, GUI_EDIT_INDICATOR_MON28_W, GUI_EDIT_INDICATOR_MON28_H);
        lv_label_set_text(indicator, "0");
        lv_obj_set_pos(indicator, x, 0);
        x+=(GUI_EDIT_INDICATOR_MON28_W+GUI_BACKPLATE_OBJECT_PADDING);
        indicators.push_back(indicator);
        digits.push_back(0);
    }
    x+=GUI_TILE_OBJECT_PADDING;//splitting into two coz of this line
    for(uint8_t i=0; i<GUI_EDIT_AFTER_DOT; i++)
    {
        indicator=lv_label_create(tile);
        lv_obj_add_style(indicator, indicator_style, LV_STATE_DEFAULT);
        lv_obj_add_style(indicator, indicator_style_sel, LV_STATE_CHECKED);
        lv_obj_add_style(indicator, indicator_style_lock, LV_STATE_USER_1);
        lv_obj_set_size(indicator, GUI_EDIT_INDICATOR_MON28_W, GUI_EDIT_INDICATOR_MON28_H);
        lv_label_set_text(indicator, "0");
        lv_obj_set_pos(indicator, x, 0);
        x+=(GUI_EDIT_INDICATOR_MON28_W+GUI_BACKPLATE_OBJECT_PADDING);
        indicators.push_back(indicator);
        digits.push_back(0);
    }

    /*back button*/
    back_button=new gui_back_field_t(screen);

    /*min*/
    min=lv_label_create(screen);
    lv_obj_set_style_text_color(min, GUI_COLOR_TEXT, 0);
    lv_obj_align_to(min, tile, LV_ALIGN_OUT_BOTTOM_LEFT, GUI_TILE_OBJECT_PADDING, GUI_TILE_OBJECT_PADDING);

    /*max*/
    max=lv_label_create(screen);
    lv_obj_set_style_text_color(max, GUI_COLOR_TEXT, 0);
    lv_obj_align_to(max, tile, LV_ALIGN_OUT_BOTTOM_LEFT, GUI_TILE_OBJECT_PADDING, 2*GUI_TILE_OBJECT_PADDING+GUI_FONT14_HEIGHT);
}

void gui_editor_t::start_edit(gui_float_field_t *_field_ptr)
{
    field_ptr=_field_ptr;
    if(field_ptr==nullptr) return;
    SemaphoreHandle_t mutex=field_ptr->get_mutex();
    if(mutex!=NULL)
    {
        xSemaphoreTake(mutex, portMAX_DELAY);
        edited_value=field_ptr->get_var();
        xSemaphoreGive(mutex);
    }
    else edited_value=field_ptr->get_var();

    lv_label_set_text(name, field_ptr->get_name());
    lv_obj_align_to(name, tile, LV_ALIGN_OUT_TOP_MID, 0, -GUI_LABEL_OBJ_PADDING);

    lv_label_set_text(unit, field_ptr->get_unit_ptr());
    lv_obj_align_to(unit, indicators[indicators.size()-1], LV_ALIGN_OUT_RIGHT_BOTTOM, GUI_BACKPLATE_OBJECT_PADDING, -GUI_BACKPLATE_OBJECT_PADDING);

    prim_idx=GUI_EDIT_BACK_IDX;
    back_button->select_field();
    in_digit=false;   

    decompose();
    update_digits();
    //ESP_LOGI("EDT", "IN: %f", edited_value);

    snprintf(buf, sizeof(buf), "Min: %.*f", field_ptr->get_float_prec(), field_ptr->get_min());
    lv_label_set_text(min, buf);

    snprintf(buf, sizeof(buf), "Max: %.*f", field_ptr->get_float_prec(), field_ptr->get_max());
    lv_label_set_text(max, buf);

    lv_screen_load(screen);
}
void gui_editor_t::save_edit()
{
    if(prim_idx!=255)
    {
        lv_obj_remove_state(indicators[prim_idx], LV_STATE_CHECKED);
        lv_obj_remove_state(indicators[prim_idx], LV_STATE_USER_1);
    }
    recompose();
    //ESP_LOGI("EDT", "OUT: %f", edited_value);

    if(edited_value>field_ptr->get_max()) edited_value=field_ptr->get_max();
    else if(edited_value<field_ptr->get_min()) edited_value=field_ptr->get_min();
    
    SemaphoreHandle_t mutex=field_ptr->get_mutex();
    if(mutex!=NULL)
    {
        xSemaphoreTake(mutex, portMAX_DELAY);
        field_ptr->set_var(edited_value);
        xSemaphoreGive(mutex);
    }
    else field_ptr->set_var(edited_value);

    field_ptr->send_ntcode();
}

void gui_editor_t::cmd_next()
{
    if(in_digit)
    {
        if(digits[prim_idx]<9)
        {
            digits[prim_idx]++;
            char buf[2];
            itoa(digits[prim_idx], buf, 10);
            lv_label_set_text(indicators[prim_idx], buf);
        }
        else if(digits[prim_idx]==9 && prim_idx>0 && digits[prim_idx-1]<9)
        {
            digits[prim_idx]=0;
            char buf[2];
            itoa(digits[prim_idx], buf, 10);
            lv_label_set_text(indicators[prim_idx], buf);

            digits[prim_idx-1]++;
            itoa(digits[prim_idx-1], buf, 10);
            lv_label_set_text(indicators[prim_idx-1], buf);
        }
    }
    else
    {
        if(prim_idx==GUI_EDIT_TOTAL_DIGITS-1) return;

        if(prim_idx==GUI_EDIT_BACK_IDX) back_button->unselect_field();
        else lv_obj_remove_state(indicators[prim_idx], LV_STATE_CHECKED);
        prim_idx++;
        lv_obj_add_state(indicators[prim_idx], LV_STATE_CHECKED);
    }
}
void gui_editor_t::cmd_prev()
{
    if(in_digit)
    {
        if(digits[prim_idx]>0)
        {
            digits[prim_idx]--;
            char buf[2];
            itoa(digits[prim_idx], buf, 10);
            lv_label_set_text(indicators[prim_idx], buf);
        }
        else if(digits[prim_idx]==0 && prim_idx>0 && digits[prim_idx-1]>0)
        {
            digits[prim_idx]=9;
            char buf[2];
            itoa(digits[prim_idx], buf, 10);
            lv_label_set_text(indicators[prim_idx], buf);

            digits[prim_idx-1]--;
            itoa(digits[prim_idx-1], buf, 10);
            lv_label_set_text(indicators[prim_idx-1], buf);
        }
    }
    else
    {
        if(prim_idx==GUI_EDIT_BACK_IDX) return;
        lv_obj_remove_state(indicators[prim_idx], LV_STATE_CHECKED);
        prim_idx--;

        if(prim_idx==GUI_EDIT_BACK_IDX) back_button->select_field();
        else lv_obj_add_state(indicators[prim_idx], LV_STATE_CHECKED);
    }
}

bool gui_editor_t::cmd_enter()
{
    if(in_digit)
    {
        //leave digit
        in_digit=false;
        lv_obj_remove_state(indicators[prim_idx], LV_STATE_USER_1);
    }
    else
    {
        if(prim_idx==255) return true; //leave;
        //enter digit
        in_digit=true;
        lv_obj_add_state(indicators[prim_idx], LV_STATE_USER_1);
    }
    return false;
}

void gui_editor_t::decompose()
{
    int temp=(int)(edited_value*pow(10, GUI_EDIT_AFTER_DOT)+0.5f);
    for(int8_t i=GUI_EDIT_TOTAL_DIGITS-1; i>=0; i--)
    {
        digits[i]=temp%10;
        temp/=10;
    }
}

void gui_editor_t::recompose()
{
    edited_value=0.0f;
    for(uint8_t i=0; i<GUI_EDIT_TOTAL_DIGITS; i++)
        edited_value=(edited_value*10.0f)+digits[i];

    edited_value/=pow(10, GUI_EDIT_AFTER_DOT);
}

void gui_editor_t::update_digits()
{
    char buf[2];
    for(uint8_t i=0; i<GUI_EDIT_TOTAL_DIGITS; i++)
    {
        itoa(digits[i], buf, 10);
        lv_label_set_text(indicators[i], buf);
    }
}