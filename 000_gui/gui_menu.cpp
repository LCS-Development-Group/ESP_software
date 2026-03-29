#include "../000_gui.h"
#include "../header.h"

void gui_controller_t::fill_gui()
{
    page_list.push_back(new gui_page_t("Main", screen, gui_init_page_test));
    page_list.push_back(new gui_page_t("Readings", screen, gui_init_page_test));
    page_list.push_back(new gui_page_t("Regulation", screen, gui_init_page_test));
    page_list.push_back(new gui_page_t("Servos", screen, gui_init_page_test));
    page_list.push_back(new gui_page_t("Starter", screen, gui_init_page_test));
    page_list.push_back(new gui_page_t("Comms", screen, gui_init_page_test));
    page_list.push_back(new gui_page_t("Display", screen, gui_init_page_test));
    page_list.push_back(new gui_page_t("f", screen, gui_init_page_test));
    page_list.push_back(new gui_page_t("g", screen, gui_init_page_test));
    page_list.push_back(new gui_page_t("h", screen, gui_init_page_test));

    // if(page_list.size()>0)
    // {
    //     lv_screen_load(page_list[0].get_screen_ptr());
    // }
}

void gui_init_page_test(std::vector <gui_generic_field_t*>* selectable,
    std::vector <gui_generic_field_t*>* unselectable,
    std::vector <lv_obj_t *>* deco,
    lv_obj_t* screen)
{
    gui_generic_field_t* field_ptr=new gui_sw_field_t(nullptr, screen, &DEBUG_BOOL, DEBUG_BOOL_MUT, "test alpha", 20, 20);
    selectable->push_back(field_ptr);
}