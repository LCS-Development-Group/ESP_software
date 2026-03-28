#include "../000_gui.h"

gui_controller_t::gui_controller_t()
{
    //vars
    mutex=xSemaphoreCreateMutex();
    if(mutex==NULL) exit(-1);

    page_index=0;
    in_page=false;
    list_top=0;

    field_index=0;
    in_field=false;

    //background
    screen=lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen, GUI_LV_PAGE_BG_COLOR, 0);

    //finally
    this->fill_gui();
    this->show_list();
    lv_screen_load(screen);
}

gui_controller_t::~gui_controller_t(){}

void gui_controller_t::cmd_next()
{
    if(in_page==false)
    {
        if(page_index<(page_list.size()-1))
            select_next_list_entry();
    }
    else//in a page
    {
        if(in_field==false)//jumping between fields
        {
            if(field_index<(page_list[page_index]->get_numof_selectable()-1))
            {
                page_list[page_index]->get_selectable_field(field_index)->unselect_field();
                field_index++;
                page_list[page_index]->get_selectable_field(field_index)->select_field();
            }
        }
        else
        {

        }
    }
}

void gui_controller_t::cmd_prev()
{
    if(in_page==false)
    {
        if(page_index>0)
            select_prev_list_entry();
    }
    else//in a page
    {
        if(in_field==false)//jumping between fields
        {
            if(field_index>0)
            {
                page_list[page_index]->get_selectable_field(field_index)->unselect_field();
                field_index--;
                page_list[page_index]->get_selectable_field(field_index)->select_field();
            }
        }
        else
        {
            
        }
    }
}
void gui_controller_t::cmd_enter()
{
    if(in_page==false)
    {
        enter_page();
    }
    else
    {
        if(in_field==false)//jumping between fields
        {
            gui_generic_field_t* field=page_list[page_index]->get_selectable_field(field_index);
            switch(field->get_field_type())
            {
                case gui_field_type_t::SW_BOOL:{
                    gui_sw_field_t* temp=static_cast<gui_sw_field_t*>(field);
                    xSemaphoreTake(temp->get_mutex(), portMAX_DELAY);
                    temp->switch_state();
                    xSemaphoreGive(temp->get_mutex());
                    break;}

                case gui_field_type_t::BACK_BTN:
                    leave_page();
                    break;

                case gui_field_type_t::FLOAT_IN:    break;
                case gui_field_type_t::FLOAT_OUT:   break;
                case gui_field_type_t::INT16:       break;
                case gui_field_type_t::SW_POS:      break;
                case gui_field_type_t::TEXT:        break;
                default:
                    break;
            }
        }    
        else
        {
            
        }
    }
}
void gui_controller_t::cmd_back()
{
    if(in_page==false)
    {
        
    }
    else
    {
        leave_page();
    }
}

void gui_controller_t::select_next_list_entry()
{
    lv_obj_remove_state(page_list[page_index]->get_menu_label_ptr(), LV_STATE_CHECKED);
    page_index++;
    lv_obj_add_state(page_list[page_index]->get_menu_label_ptr(), LV_STATE_CHECKED);

    //if movinf beyond visible range
    if(page_index>=(list_top+GUI_MENU_ENTRY_NUM))
    {
        lv_obj_remove_flag(page_list[page_index]->get_menu_label_ptr(), LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(page_list[list_top]->get_menu_label_ptr(), LV_OBJ_FLAG_HIDDEN);
        list_top++;

        for(uint8_t i=0; i<GUI_MENU_ENTRY_NUM; i++)
        {
            if((list_top+i)>=page_list.size()) return;
            lv_obj_set_pos(page_list[list_top+i]->get_menu_label_ptr(), 0, i*GUI_MENU_ENTRY_SPACING_PX);
        }
    }
}
void gui_controller_t::select_prev_list_entry()
{
    lv_obj_remove_state(page_list[page_index]->get_menu_label_ptr(), LV_STATE_CHECKED);
    page_index--;
    lv_obj_add_state(page_list[page_index]->get_menu_label_ptr(), LV_STATE_CHECKED);

    //if movinf beyond visible range
    if(page_index<list_top)
    {
        lv_obj_remove_flag(page_list[page_index]->get_menu_label_ptr(), LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(page_list[list_top+GUI_MENU_ENTRY_NUM-1]->get_menu_label_ptr(), LV_OBJ_FLAG_HIDDEN);
        list_top--;

        for(uint8_t i=0; i<GUI_MENU_ENTRY_NUM; i++)
        {
            if((list_top+i)>=page_list.size()) return;
            lv_obj_set_pos(page_list[list_top+i]->get_menu_label_ptr(), 0, i*GUI_MENU_ENTRY_SPACING_PX);
        }
    }
}

void gui_controller_t::show_list()
{
    lv_obj_add_state(page_list[page_index]->get_menu_label_ptr(), LV_STATE_CHECKED);
    for(uint8_t i=list_top; i<list_top+GUI_MENU_ENTRY_NUM; i++)
    {
        if(i>=page_list.size()) return;
        lv_obj_remove_flag(page_list[i]->get_menu_label_ptr(), LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_pos(page_list[i]->get_menu_label_ptr(), 0, i*GUI_MENU_ENTRY_SPACING_PX);
    }
}

void gui_controller_t::enter_page()
{
    in_page=true;
    lv_screen_load(page_list[page_index]->get_screen_ptr());
    in_field=false;
    field_index=0;
}

void gui_controller_t::leave_page()
{
    in_page=false;
    lv_screen_load(screen);
}