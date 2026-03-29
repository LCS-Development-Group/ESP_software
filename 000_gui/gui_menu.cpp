#include "../000_gui.h"
#include "../header.h"

void gui_controller_t::fill_gui()
{
    page_list.push_back(new gui_page_t("Main", screen, gui_init_page_readings));
    page_list.push_back(new gui_page_t("Readings", screen, gui_init_page_test));
    page_list.push_back(new gui_page_t("Regulation", screen, gui_init_page_test));
    page_list.push_back(new gui_page_t("Servos", screen, gui_init_page_test));
    page_list.push_back(new gui_page_t("Starter", screen, gui_init_page_test));
    page_list.push_back(new gui_page_t("Comms", screen, gui_init_page_test));
    page_list.push_back(new gui_page_t("Display", screen, gui_init_page_test));
    page_list.push_back(new gui_page_t("f", screen, gui_init_page_test));
    page_list.push_back(new gui_page_t("g", screen, gui_init_page_test));
    page_list.push_back(new gui_page_t("h", screen, gui_init_page_test));
}

/*=======================================================================================================================================*/

void gui_init_page_test(std::vector <gui_generic_field_t*>* selectable,
    std::vector <gui_generic_field_t*>* unselectable,
    std::vector <lv_obj_t *>* deco,
    lv_obj_t* screen)
{
    lv_obj_t *object;
    uint16_t x,y;
    gui_generic_field_t* field_ptr;
    
    //switch
    x=50; y=50; object=lv_label_create(screen);
    lv_label_set_text(object, "test switch");
    lv_obj_set_style_text_color(object, GUI_COLOR_TEXT, 0);
    lv_obj_update_layout(object);
    lv_obj_set_pos(object, x, y-lv_obj_get_height(object)-GUI_LABEL_OBJ_PADDING);
    deco->push_back(object);

    field_ptr=new gui_sw_bool_field_t(nullptr, DEBUG_BOOL_MUT, &DEBUG_BOOL, screen, x, y);
    selectable->push_back(field_ptr);


    field_ptr=new gui_float_field_t(nullptr, DEBUG_FLOAT_MUT, &DEBUG_FLOAT, screen, 50, 100, GUI_COLOR_SW_ON, '%', 3);
    selectable->push_back(field_ptr);
}

/*=======================================================================================================================================*/
void gui_init_page_readings(std::vector <gui_generic_field_t*>* selectable,
    std::vector <gui_generic_field_t*>* unselectable,
    std::vector <lv_obj_t *>* deco,
    lv_obj_t* screen)
{
    lv_obj_t *label, *tile;
    gui_generic_field_t* field_ptr;
    uint8_t x, y;
    

    /*Chamber RHT*/
    x=20; y=40;
    label=lv_label_create(screen);
    lv_obj_set_style_text_color(label, GUI_COLOR_TEXT, 0);
    lv_label_set_text(label, "Chamber");
    lv_obj_set_pos(label, x, y);
    lv_obj_update_layout(label);
    deco->push_back(label);

    tile=lv_obj_create(screen);
    lv_obj_set_style_bg_color(tile, GUI_COLOR_TILE_BG, 0);
    lv_obj_set_style_radius(tile, GUI_TILE_CORNER_RADIUS, 0);
    lv_obj_set_style_border_width(tile, 0, 0);
    lv_obj_set_size(tile, 120, 56+3*GUI_TILE_OBJECT_PADDING);//2*font24+3*padding (incl. newline)
    lv_obj_align_to(tile, label, LV_ALIGN_OUT_BOTTOM_LEFT, -GUI_TILE_OBJECT_PADDING, GUI_LABEL_OBJ_PADDING);
    deco->push_back(tile);

    field_ptr=new gui_float_field_t(nullptr, *RHT_int->get_mutex_ptr(), RHT_int->get_RH_ptr(), screen, 
    x, y+GUI_TILE_OBJECT_PADDING+GUI_LABEL_OBJ_PADDING+14, GUI_COLOR_RH_INT, '%', 3);
    unselectable->push_back(field_ptr);

    field_ptr=new gui_float_field_t(nullptr, *RHT_int->get_mutex_ptr(), RHT_int->get_T_ptr(), screen, 
    x, y+2*GUI_TILE_OBJECT_PADDING+GUI_LABEL_OBJ_PADDING+28+14, GUI_COLOR_T_INT, 'C', 3);
    unselectable->push_back(field_ptr);
}
/*=======================================================================================================================================*/
/*=======================================================================================================================================*/
/*=======================================================================================================================================*/
/*=======================================================================================================================================*/
/*=======================================================================================================================================*/
