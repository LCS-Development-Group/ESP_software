#include "../000_gui.h"
#include "../header.h"

void gui_controller_t::fill_gui()
{
    page_list.push_back(new gui_page_t("Readings", screen, editor_ptr, gui_init_page_test));
    page_list.push_back(new gui_page_t("Main", screen, editor_ptr, gui_init_page_readings));
    page_list.push_back(new gui_page_t("Regulation", screen, editor_ptr, gui_init_page_test));
    page_list.push_back(new gui_page_t("Servo control", screen, editor_ptr, gui_init_page_servos));
    page_list.push_back(new gui_page_t("Servo settings", screen, editor_ptr, gui_init_page_test));
    page_list.push_back(new gui_page_t("Starter", screen, editor_ptr, gui_init_page_test));
    page_list.push_back(new gui_page_t("Comms", screen, editor_ptr, gui_init_page_test));
    page_list.push_back(new gui_page_t("Display", screen, editor_ptr, gui_init_page_test));
    page_list.push_back(new gui_page_t("f", screen, editor_ptr, gui_init_page_test));
    page_list.push_back(new gui_page_t("g", screen, editor_ptr, gui_init_page_test));
    page_list.push_back(new gui_page_t("h", screen, editor_ptr, gui_init_page_test));
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

    field_ptr=new gui_sw_bool_field_t(nullptr, DEBUG_BOOL_MUT, &DEBUG_BOOL, screen, x, y, true);
    selectable->push_back(field_ptr);

    field_ptr=new gui_float_field_t(nullptr, DEBUG_FLOAT_MUT, &DEBUG_FLOAT, screen, 50, 100, GUI_COLOR_SW_ON, "%", "Debug float", 3);
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
    lv_obj_add_style(tile, gui_style_bg_tile, LV_STATE_DEFAULT);
    
    lv_obj_set_size(tile, 120, 2*GUI_FONT20_HEIGHT+3*GUI_TILE_OBJECT_PADDING+4*GUI_BACKPLATE_OBJECT_PADDING);
    lv_obj_align_to(tile, label, LV_ALIGN_OUT_BOTTOM_LEFT, -GUI_TILE_OBJECT_PADDING, GUI_LABEL_OBJ_PADDING);
    lv_obj_clear_flag(tile, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(tile, LV_SCROLLBAR_MODE_OFF);
    deco->push_back(tile);

    field_ptr=new gui_float_field_t(nullptr, *RHT_int->get_mutex_ptr(), RHT_int->get_RH_ptr(), tile, 
    0, 0, GUI_COLOR_RH_INT, "%", "Chamber RH", 2);
    unselectable->push_back(field_ptr);

    field_ptr=new gui_float_field_t(nullptr, *RHT_int->get_mutex_ptr(), RHT_int->get_T_ptr(), tile, 
    0, GUI_TILE_OBJECT_PADDING+GUI_FONT20_HEIGHT+2*GUI_BACKPLATE_OBJECT_PADDING, GUI_COLOR_T_INT, "°C", "Chamber Temp.", 2);
    selectable->push_back(field_ptr);
}

/*=======================================================================================================================================*/

void gui_init_page_servos(std::vector <gui_generic_field_t*>* selectable,
    std::vector <gui_generic_field_t*>* unselectable,
    std::vector <lv_obj_t *>* deco,
    lv_obj_t* screen)
{
    lv_obj_t *label, *tile;
    gui_generic_field_t* field_ptr;
    uint8_t y[]={80, 120, 160, 205};
    uint8_t x[]={0, 50, 130, 190, 250};
    //uint8_t ntcodes[]={ACT_NTCODE_UPDATE_SERV0, ACT_NTCODE_UPDATE_SERV1, ACT_NTCODE_UPDATE_SERV2, ACT_NTCODE_UPDATE_SERV3};
    uint8_t ntcodes[]={6, 7, 8, ACT_NTCODE_UPDATE_SERV3};//for safety - remove in board v2
    const char* text[]={"S0", "S1", "S2", "S3"};
    const char* POS0[]={"P_0_0", "P_0_1", "P_0_2", "P_0_3"};
    const char* POS1[]={"P_1_0", "P_1_1", "P_1_2", "P_1_3"};

    /*bg tile enabled*/
    tile=lv_obj_create(screen);
    lv_obj_add_style(tile, gui_style_bg_tile, LV_STATE_DEFAULT);
    lv_obj_set_size(tile, 
        2*GUI_TILE_OBJECT_PADDING+GUI_SW_WIDTH,
        2*GUI_TILE_OBJECT_PADDING+y[3]-y[0]+GUI_SW_HEIGHT);
    lv_obj_set_pos(tile, x[1]-GUI_TILE_OBJECT_PADDING, y[0]-GUI_TILE_OBJECT_PADDING);
    deco->push_back(tile);

    /*bg tile position*/
    tile=lv_obj_create(screen);
    lv_obj_add_style(tile, gui_style_bg_tile, LV_STATE_DEFAULT);
    lv_obj_set_size(tile, 
        2*GUI_TILE_OBJECT_PADDING+x[4]-x[2]+50, /*yeah, hardcoded but whatever*/
        2*GUI_TILE_OBJECT_PADDING+y[3]-y[0]+GUI_SW_HEIGHT);
    lv_obj_set_pos(tile, x[2]-GUI_TILE_OBJECT_PADDING, y[0]-GUI_TILE_OBJECT_PADDING);
    deco->push_back(tile);

    /*labels*/
    label=lv_label_create(screen);
    lv_obj_set_style_text_color(label, GUI_COLOR_TEXT, 0);
    lv_label_set_text(label, "Power"); 
    lv_obj_update_layout(label);
    lv_obj_set_pos(label, x[1]+GUI_SW_WIDTH/2-lv_obj_get_width(label)/2, y[0]-GUI_TILE_OBJECT_PADDING-GUI_LABEL_OBJ_PADDING-GUI_FONT14_HEIGHT);
    deco->push_back(label);

    label=lv_label_create(screen);
    lv_obj_set_style_text_color(label, GUI_COLOR_TEXT, 0);
    lv_label_set_text(label, "Positions"); 
    lv_obj_update_layout(label);
    lv_obj_set_pos(label, x[3]+GUI_SW_WIDTH/2-lv_obj_get_width(label)/2, y[0]-GUI_TILE_OBJECT_PADDING-GUI_LABEL_OBJ_PADDING-GUI_FONT14_HEIGHT);
    deco->push_back(label);
    
    for(uint8_t i=0; i<4; i++)
    {
        /*labels*/
        label=lv_label_create(screen);
        lv_obj_set_style_text_color(label, GUI_COLOR_TEXT, 0);
        lv_label_set_text(label, text[i]); 
        lv_obj_set_pos(label, x[0], y[i]); 
        deco->push_back(label);

        /*EN swithes*/
        field_ptr=new gui_sw_bool_field_t(
            new task_notify_pack_t(task_queue_list[ACT_TASKID], ntcodes[i]), 
            servos[i].mutex, &(servos[i].enabled), screen, x[1], y[i], true);
        selectable->push_back(field_ptr);

        /*POS0 labels*/
        label=lv_label_create(screen);
        lv_obj_set_style_text_color(label, GUI_COLOR_TEXT, 0);
        lv_label_set_text(label, POS0[i]); 
        lv_obj_set_pos(label, x[2], y[i]); 
        deco->push_back(label);

        /*pos swithes*/
        field_ptr=new gui_sw_bool_field_t(
            new task_notify_pack_t(task_queue_list[ACT_TASKID], ntcodes[i]), 
            servos[i].mutex, &(servos[i].position), screen, x[3], y[i], false);
        selectable->push_back(field_ptr);

        /*POS1 labels*/
        label=lv_label_create(screen);
        lv_obj_set_style_text_color(label, GUI_COLOR_TEXT, 0);
        lv_label_set_text(label, POS1[i]); 
        lv_obj_set_pos(label, x[4], y[i]); 
        deco->push_back(label);
    }
}

/*=======================================================================================================================================*/
/*=======================================================================================================================================*/
/*=======================================================================================================================================*/
/*=======================================================================================================================================*/
