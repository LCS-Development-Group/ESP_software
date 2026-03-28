#include "../000_gui.h"

gui_page_t::gui_page_t(
std::string _name,
lv_obj_t* main_screen,
void (*init_func)(
    std::vector <gui_generic_field_t*>*,
    std::vector <gui_generic_field_t*>*,
    std::vector <lv_obj_t *>*,
    lv_obj_t*
)):name(_name)
{
    screen=lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen, GUI_LV_PAGE_BG_COLOR, 0);

    //main menu label
    menu_label=lv_label_create(main_screen);
    lv_obj_add_style(menu_label, list_style_def, LV_STATE_DEFAULT);
    lv_obj_add_style(menu_label, list_style_sel, LV_STATE_CHECKED);
    lv_label_set_text(menu_label, name.c_str());
    lv_obj_add_flag(menu_label, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_width(menu_label, GUI_MENU_ENTRY_WIDTH_PX);

    //name label
    name_label=lv_label_create(screen);
    lv_label_set_text(name_label, name.c_str());
    lv_obj_set_style_text_color(name_label, GUI_LV_TEXT_COLOR, 0);
    lv_obj_set_align(name_label, LV_ALIGN_TOP_MID);

    //back button
    selectable.push_back(new gui_back_field_t(screen));

    init_func(
        &selectable,
        &unselectable,
        &deco,
        screen
    );
}

gui_generic_field_t* gui_page_t::get_selectable_field(uint8_t idx)
{
    if(idx>=selectable.size()) return selectable[0];
    else return selectable[idx];
}

gui_page_t::~gui_page_t()
{
    // for(uint8_t i=0; i<selectable.size(); i++)
    //     if(selectable[i]!=nullptr) delete selectable[i];

    // for(uint8_t i=0; i<unselectable.size(); i++)
    //     if(unselectable[i]!=nullptr) delete unselectable[i];

    // for(uint8_t i=0; i<deco.size(); i++)
    //     if(deco[i]!=nullptr) delete deco[i];
}