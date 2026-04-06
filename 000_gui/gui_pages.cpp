#include "../000_gui.h"

gui_page_t::gui_page_t(
std::string _name,
lv_obj_t* main_screen,
gui_editor_t *_editor_ptr,
void (*init_func)(
    std::vector <gui_generic_field_t*>*,
    std::vector <gui_generic_field_t*>*,
    std::vector <lv_obj_t *>*,
    lv_obj_t*
)):name(_name), editor_ptr(_editor_ptr)
{
    field_index=0;
    in_field=false;

    screen=lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen, GUI_COLOR_PAGE_BG, 0);

    //main menu label
    menu_label=lv_label_create(main_screen);
    lv_obj_add_style(menu_label, gui_style_menu_def, LV_STATE_DEFAULT);
    lv_obj_add_style(menu_label, gui_style_menu_sel, LV_STATE_CHECKED);
    lv_label_set_text(menu_label, name.c_str());
    lv_obj_add_flag(menu_label, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_width(menu_label, GUI_MENU_ENTRY_WIDTH_PX);

    //name label
    name_label=lv_label_create(screen);
    lv_label_set_text(name_label, name.c_str());
    lv_obj_set_style_text_color(name_label, GUI_COLOR_TEXT, 0);
    //lv_obj_set_align(name_label, LV_ALIGN_TOP_MID);
    lv_obj_set_pos(name_label, 30, 5);

    //back button
    selectable.push_back(new gui_back_field_t(screen));

    init_func(
        &selectable,
        &unselectable,
        &deco,
        screen
    );
}

void gui_page_t::cmd_next()
{
    if(in_field==false)//jumping between fields
    {
        if(field_index<selectable.size()-1)
        {
            selectable[field_index]->unselect_field();
            field_index++;
            selectable[field_index]->select_field();
        }
    }
    else
    {
        editor_ptr->cmd_next();
    }
}

void gui_page_t::cmd_prev()
{
    if(in_field==false)//jumping between fields
    {
        if(field_index>0)
        {
                selectable[field_index]->unselect_field();
                field_index--;
                selectable[field_index]->select_field();
        }
    }
    else
    {
        editor_ptr->cmd_prev();
    }
}

bool gui_page_t::cmd_enter()
{
    if(in_field==false)//jumping between fields
    {
        switch(selectable[field_index]->get_field_type())
        {
            case gui_field_type_t::SW_BOOL:{
                gui_sw_bool_field_t* temp=static_cast<gui_sw_bool_field_t*>(selectable[field_index]);
                xSemaphoreTake(temp->get_mutex(), portMAX_DELAY);
                temp->toggle();
                xSemaphoreGive(temp->get_mutex());
                break;}

            case gui_field_type_t::BACK_BTN:
                selectable[field_index]->unselect_field();
                return true;
                break;

            case gui_field_type_t::FLOAT:{
                gui_float_field_t *temp=static_cast<gui_float_field_t*>(selectable[field_index]);    
                editor_ptr->start_edit(temp);
                in_field=true;
                break;}

            case gui_field_type_t::INT16:       break;
            case gui_field_type_t::SW_POS:      break;
            case gui_field_type_t::TEXT:        break;
            default:
                break;
        }
    }    
    else
    {
        //editor
        if(editor_ptr->cmd_enter())//true - enter resulted in leaving
        {
            in_field=false;
            editor_ptr->save_edit();
            lv_screen_load(screen);
        }
    }
    return false;
}

bool gui_page_t::cmd_back()
{
    if(in_field==false)
    {
        unload_page();
        return true;
    }
    else
    {
        in_field=false;
        editor_ptr->save_edit();
        lv_screen_load(screen);
        return false;
    }
}

void gui_page_t::cmd_update_page()
{
    if(in_field==true) return;

    uint8_t i_max;
    gui_generic_field_t *field_ptr;
    SemaphoreHandle_t _mutex;

    i_max=selectable.size();
    for(uint8_t i=0; i<i_max; i++)
    {
        field_ptr=selectable[i];
        if(field_ptr==nullptr) continue;
        _mutex=field_ptr->get_mutex();
        if(_mutex!=NULL) 
        {
            //if field has no mutex then it is prolly not updatable - assumption
            xSemaphoreTake(_mutex, portMAX_DELAY);
            field_ptr->update_state();
            xSemaphoreGive(_mutex);
        }
    }

    i_max=unselectable.size();
    for(uint8_t i=0; i<i_max; i++)
    {
        field_ptr=unselectable[i];
        if(field_ptr==nullptr) continue;
        _mutex=field_ptr->get_mutex();
        if(_mutex!=NULL) 
        {
            //if field has no mutex then it is prolly not updatable - assumption
            xSemaphoreTake(_mutex, portMAX_DELAY);
            field_ptr->update_state();
            xSemaphoreGive(_mutex);
        }
    }
}

void gui_page_t::load_page()
{
    lv_screen_load(screen);
    field_index=0;
    selectable[field_index]->select_field();
}

void gui_page_t::unload_page()
{
    selectable[field_index]->unselect_field();
}


gui_generic_field_t* gui_page_t::get_selectable_field(uint8_t idx)
{
    if(idx>=selectable.size()) return selectable[0];
    else return selectable[idx];
}

gui_generic_field_t *gui_page_t::get_unselectable_field(uint8_t idx)
{
    if(idx>=unselectable.size()) return unselectable[0];
    else return unselectable[idx];
}