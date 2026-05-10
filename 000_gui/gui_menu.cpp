#include "../000_gui.h"
#include "../header.h"

#define GUI_MENU_PAGEID_MAIN        0
#define GUI_MENU_PAGEID_REGULATOR   1
#define GUI_MENU_PAGEID_STARTER     2
#define GUI_MENU_PAGEID_SERV_CON    3
#define GUI_MENU_PAGEID_MISC        4
#define GUI_MENU_PAGEID_INFO        5
#define GUI_MENU_PAGEID_SERV_SET    6

void gui_controller_t::fill_gui()
{
    page_list.push_back(new gui_page_t("Main", screen, editor_ptr, gui_init_page_readings));
    page_list.push_back(new gui_page_t("Regulation", screen, editor_ptr, gui_init_page_regulation));
    page_list.push_back(new gui_page_t("Starter", screen, editor_ptr, gui_init_page_starter));
    page_list.push_back(new gui_page_t("Servo control", screen, editor_ptr, gui_init_page_servos));
    page_list.push_back(new gui_page_t("Settings", screen, editor_ptr, gui_init_page_misc_settings));
    page_list.push_back(new gui_page_t("Info", screen, editor_ptr, gui_init_page_placeholder));
    page_list.push_back(new gui_page_t("Servo positions", screen, editor_ptr, gui_init_page_servos_cfg));
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

    field_ptr=new gui_float_field_t(nullptr, DEBUG_FLOAT_MUT, &DEBUG_FLOAT, screen, 50, 100, GUI_COLOR_SW_ON, 1, "%", "Debug float", 3, 100, 0);
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

    uint8_t height=3*GUI_TILE_OBJECT_PADDING+4*GUI_BACKPLATE_OBJECT_PADDING+2*GUI_FONT14_HEIGHT;
    uint8_t width=90;
    /*Chamber RHT*/
    x=0; y=60;
    tile=lv_obj_create(screen);
    lv_obj_add_style(tile, gui_style_bg_tile, 0);
    lv_obj_set_size(tile,
    width,
    height);
    lv_obj_set_pos(tile, x, y);
    deco->push_back(tile);

    label=lv_label_create(screen);
    lv_obj_set_style_text_color(label, GUI_COLOR_TEXT, 0);
    lv_label_set_text(label, "Chamber");
    lv_obj_align_to(label, tile, LV_ALIGN_OUT_TOP_MID, 0, -GUI_LABEL_OBJ_PADDING);
    deco->push_back(label);

    field_ptr=new gui_float_field_t(
        nullptr, RHT_int->get_mutex(), RHT_int->get_RH_ptr(), screen, 
        x+GUI_TILE_OBJECT_PADDING,
        y+GUI_TILE_OBJECT_PADDING,
        GUI_COLOR_RH_INT, 0, "%", 2);
    unselectable->push_back(field_ptr);

    field_ptr=new gui_float_field_t(
        nullptr, RHT_int->get_mutex(), RHT_int->get_T_ptr(), screen, 
        x+GUI_TILE_OBJECT_PADDING,
        y+GUI_TILE_OBJECT_PADDING+3*GUI_BACKPLATE_OBJECT_PADDING+GUI_FONT20_HEIGHT,
        GUI_COLOR_T_INT, 0, "°C", 2);
    unselectable->push_back(field_ptr);

    /*mini Regulator*/
    x=width+GUI_TILE_OBJECT_PADDING;
    uint8_t width2=LCD_WIDTH-2*(width+GUI_TILE_OBJECT_PADDING);
    tile=lv_obj_create(screen);
    lv_obj_add_style(tile, gui_style_bg_tile, 0);
    lv_obj_set_size(tile,
    width2,
    height);
    lv_obj_set_pos(tile, x, y);
    deco->push_back(tile);

    label=lv_label_create(screen);
    lv_obj_set_style_text_color(label, GUI_COLOR_TEXT, 0);
    lv_label_set_text(label, "Regulation");
    lv_obj_align_to(label, tile, LV_ALIGN_OUT_TOP_MID, 0, -GUI_LABEL_OBJ_PADDING);
    deco->push_back(label);

    field_ptr=new gui_float_field_t(
        new task_notify_pack_t(task_queue_list[REG_TASKID], REG_NTCODE_UPDATE), 
        regulator.mutex, &(regulator.SP), screen, 
        x+width2-GUI_TILE_OBJECT_PADDING-65,
        y+GUI_TILE_OBJECT_PADDING,
        GUI_COLOR_SP, 0, "%", "Chamber RH setpoint", 2, REG_MAX_SP, REG_MIN_SP);

    selectable->push_back(field_ptr);
    label=lv_label_create(screen);
    lv_obj_set_style_text_color(label, GUI_COLOR_TEXT, 0);
    lv_label_set_text(label, "SP");
    lv_obj_set_pos(label, 
    x+GUI_TILE_OBJECT_PADDING,
    y+GUI_TILE_OBJECT_PADDING+GUI_BACKPLATE_OBJECT_PADDING);
    deco->push_back(label);

    field_ptr=new gui_sw_bool_field_t(
        new task_notify_pack_t(task_queue_list[REG_TASKID], REG_NTCODE_UPDATE),
        regulator.mutex, &(regulator.enabled), screen,
        x+width2-GUI_TILE_OBJECT_PADDING-GUI_SW_WIDTH,
        y+2*GUI_TILE_OBJECT_PADDING+3*GUI_BACKPLATE_OBJECT_PADDING+GUI_FONT14_HEIGHT,
        true);
    selectable->push_back(field_ptr);

    label=lv_label_create(screen);
    lv_obj_set_style_text_color(label, GUI_COLOR_TEXT, 0);
    lv_label_set_text(label, "Status");
    lv_obj_set_pos(label, 
    x+GUI_TILE_OBJECT_PADDING,
    y+2*GUI_TILE_OBJECT_PADDING+3*GUI_BACKPLATE_OBJECT_PADDING+GUI_FONT14_HEIGHT+(GUI_SW_HEIGHT-GUI_FONT14_HEIGHT)/2);
    deco->push_back(label);

    /*External RHT*/
    x=LCD_WIDTH-width;
    tile=lv_obj_create(screen);
    lv_obj_add_style(tile, gui_style_bg_tile, 0);
    lv_obj_set_size(tile,
    width,
    height);
    lv_obj_set_pos(tile, x, y);
    deco->push_back(tile);

    label=lv_label_create(screen);
    lv_obj_set_style_text_color(label, GUI_COLOR_TEXT, 0);
    lv_label_set_text(label, "Exterior");
    lv_obj_align_to(label, tile, LV_ALIGN_OUT_TOP_MID, 0, -GUI_LABEL_OBJ_PADDING);
    deco->push_back(label);

    field_ptr=new gui_float_field_t(
        nullptr, RHT_ext->get_mutex(), RHT_ext->get_RH_ptr(), screen, 
        x+GUI_TILE_OBJECT_PADDING,
        y+GUI_TILE_OBJECT_PADDING,
        GUI_COLOR_RH_EXT, 0, "%", 2);
    unselectable->push_back(field_ptr);

    field_ptr=new gui_float_field_t(
        nullptr, RHT_ext->get_mutex(), RHT_ext->get_T_ptr(), screen, 
        x+GUI_TILE_OBJECT_PADDING,
        y+GUI_TILE_OBJECT_PADDING+3*GUI_BACKPLATE_OBJECT_PADDING+GUI_FONT20_HEIGHT,
        GUI_COLOR_T_EXT, 0, "°C", 2);
    unselectable->push_back(field_ptr);

    /*laser status*/
    x=0; y+=height+GUI_TILE_OBJECT_PADDING+GUI_LABEL_OBJ_PADDING+GUI_FONT14_HEIGHT;
    width=155; height=70;
    tile=lv_obj_create(screen);
    lv_obj_add_style(tile, gui_style_bg_tile, 0);
    lv_obj_set_size(tile,
    width,
    height);
    lv_obj_set_pos(tile, x, y);
    deco->push_back(tile);

    label=lv_label_create(screen);
    lv_obj_set_style_text_color(label, GUI_COLOR_TEXT, 0);
    lv_label_set_text(label, "Laser Status");
    lv_obj_align_to(label, tile, LV_ALIGN_OUT_TOP_MID, 0, -GUI_LABEL_OBJ_PADDING);
    deco->push_back(label);

    /*mini starter*/
    x+=width+GUI_TILE_OBJECT_PADDING;
    tile=lv_obj_create(screen);
    lv_obj_add_style(tile, gui_style_bg_tile, 0);
    lv_obj_set_size(tile,
    width,
    height);
    lv_obj_set_pos(tile, x, y);
    deco->push_back(tile);

    label=lv_label_create(screen);
    lv_obj_set_style_text_color(label, GUI_COLOR_TEXT, 0);
    lv_label_set_text(label, "Starter");
    lv_obj_align_to(label, tile, LV_ALIGN_OUT_TOP_MID, 0, -GUI_LABEL_OBJ_PADDING);
    deco->push_back(label);

    /*link to regulator page*/
    field_ptr=new gui_jump_field_t(screen, GUI_MENU_PAGEID_REGULATOR);
    selectable->push_back(field_ptr);
    label=lv_label_create(screen);
    lv_label_set_text(label, "Regulator");
    lv_obj_set_style_text_color(label, GUI_COLOR_TEXT, 0);
    lv_obj_update_layout(label);
    lv_obj_align(label, LV_ALIGN_TOP_RIGHT, -30, 5);
}

/*=======================================================================================================================================*/

void gui_init_page_regulation(std::vector <gui_generic_field_t*>* selectable,
    std::vector <gui_generic_field_t*>* unselectable,
    std::vector <lv_obj_t *>* deco,
    lv_obj_t* screen)
{
    lv_obj_t *label, *tile;
    gui_generic_field_t* field_ptr;
    uint8_t x, y;

    uint8_t height=2*GUI_TILE_OBJECT_PADDING+3*(2*GUI_BACKPLATE_OBJECT_PADDING+GUI_FONT14_HEIGHT)+2*GUI_BACKPLATE_OBJECT_PADDING;
    uint8_t width=155;
    int32_t xlabel[2]={GUI_TILE_OBJECT_PADDING, 2*GUI_TILE_OBJECT_PADDING+width};
    int32_t xfield[2]={width-90+GUI_TILE_OBJECT_PADDING, LCD_WIDTH-90+GUI_TILE_OBJECT_PADDING};

    /*Settings===========================================================================================*/
    x=0; y=60;
    tile=lv_obj_create(screen);
    lv_obj_add_style(tile, gui_style_bg_tile, 0);
    lv_obj_set_size(tile, width, height);
    lv_obj_set_pos(tile, x, y);
    deco->push_back(tile);

    label=lv_label_create(screen);
    lv_obj_set_style_text_color(label, GUI_COLOR_TEXT, 0);
    lv_label_set_text(label, "Settings");
    lv_obj_align_to(label, tile, LV_ALIGN_OUT_TOP_MID, 0, -GUI_LABEL_OBJ_PADDING);
    deco->push_back(label);

    {
    //sp
    uint32_t y3=y+GUI_TILE_OBJECT_PADDING;
    field_ptr=new gui_float_field_t(
        new task_notify_pack_t(task_queue_list[REG_TASKID], REG_NTCODE_UPDATE), 
        regulator.mutex, &(regulator.SP), screen, 
        xfield[0],
        y3,
        GUI_COLOR_SP, 0, "%", "Chamber RH setpoint", 2, REG_MAX_SP, REG_MIN_SP);
    selectable->push_back(field_ptr);
    label=lv_label_create(screen);
    lv_obj_set_style_text_color(label, GUI_COLOR_TEXT, 0);
    lv_label_set_text(label, "Set RH");
    lv_obj_set_pos(label, xlabel[0], y3+GUI_BACKPLATE_OBJECT_PADDING);
    deco->push_back(label);

    //enabled
    y3+=(3*GUI_BACKPLATE_OBJECT_PADDING+GUI_FONT14_HEIGHT);
    field_ptr=new gui_sw_bool_field_t(
        new task_notify_pack_t(task_queue_list[REG_TASKID], REG_NTCODE_UPDATE),
        regulator.mutex, &(regulator.enabled), screen,
        xfield[0],
        y3+2,
        true);
    selectable->push_back(field_ptr);
    label=lv_label_create(screen);
    lv_obj_set_style_text_color(label, GUI_COLOR_TEXT, 0);
    lv_label_set_text(label, "Enabled");
    lv_obj_set_pos(label, xlabel[0], y3+GUI_BACKPLATE_OBJECT_PADDING);
    deco->push_back(label);

    //Histeresis
    y3+=(3*GUI_BACKPLATE_OBJECT_PADDING+GUI_FONT14_HEIGHT);
    field_ptr=new gui_float_field_t(
        new task_notify_pack_t(task_queue_list[REG_TASKID], REG_NTCODE_UPDATE), 
        regulator.mutex, &(regulator.H), screen, 
        xfield[0],
        y3,
        GUI_COLOR_TEXT, 0, "%", "Regulation histeresis", 2, REG_MAX_H, REG_MIN_H);
    selectable->push_back(field_ptr);
    label=lv_label_create(screen);
    lv_obj_set_style_text_color(label, GUI_COLOR_TEXT, 0);
    lv_label_set_text(label, "Hister.");
    lv_obj_set_pos(label, xlabel[0], y3+GUI_BACKPLATE_OBJECT_PADDING);
    deco->push_back(label);
    }

    /*RH/error===========================================================================================*/
    uint16_t height2=2*GUI_TILE_OBJECT_PADDING+2*GUI_BACKPLATE_OBJECT_PADDING+GUI_FONT14_HEIGHT;
    uint16_t width2=LCD_WIDTH;
    x=0; uint8_t y2=y+height+GUI_LABEL_SPACE;
    tile=lv_obj_create(screen);
    lv_obj_add_style(tile, gui_style_bg_tile, 0);
    lv_obj_set_size(tile, width2, height2);
    lv_obj_set_pos(tile, x, y2);
    deco->push_back(tile);

    label=lv_label_create(screen);
    lv_obj_set_style_text_color(label, GUI_COLOR_TEXT, 0);
    lv_label_set_text(label, "Chamber");
    lv_obj_align_to(label, tile, LV_ALIGN_OUT_TOP_MID, 0, -GUI_LABEL_OBJ_PADDING);
    deco->push_back(label);

    //RH
    field_ptr=new gui_float_field_t(
        nullptr, RHT_int->get_mutex(), RHT_int->get_RH_ptr(), screen, 
        xfield[0],
        y2+GUI_TILE_OBJECT_PADDING,
        GUI_COLOR_RH_INT, 0, "%", 2);
    unselectable->push_back(field_ptr);

    label=lv_label_create(screen);
    lv_obj_set_style_text_color(label, GUI_COLOR_TEXT, 0);
    lv_label_set_text(label, "RH");
    lv_obj_set_pos(label, 
    xlabel[0],
    y2+GUI_TILE_OBJECT_PADDING+GUI_BACKPLATE_OBJECT_PADDING);
    deco->push_back(label);

    //ERROR
    field_ptr=new gui_float_field_t(
        nullptr, regulator.mutex, &(regulator.E), screen, 
        xfield[1],
        y2+GUI_TILE_OBJECT_PADDING,
        GUI_COLOR_TEXT, 0, "%", 2);
    unselectable->push_back(field_ptr);

    label=lv_label_create(screen);
    lv_obj_set_style_text_color(label, GUI_COLOR_TEXT, 0);
    lv_label_set_text(label, "Error");
    lv_obj_set_pos(label, 
    xlabel[1],
    y2+GUI_TILE_OBJECT_PADDING+GUI_BACKPLATE_OBJECT_PADDING);
    deco->push_back(label);

    /*Membrane===========================================================================================*/
    x=width+GUI_TILE_OBJECT_PADDING;
    tile=lv_obj_create(screen);
    lv_obj_add_style(tile, gui_style_bg_tile, 0);
    lv_obj_set_size(tile, width, height);
    lv_obj_set_pos(tile, x, y);
    deco->push_back(tile);

    label=lv_label_create(screen);
    lv_obj_set_style_text_color(label, GUI_COLOR_TEXT, 0);
    lv_label_set_text(label, "Membrane");
    lv_obj_align_to(label, tile, LV_ALIGN_OUT_TOP_MID, 0, -GUI_LABEL_OBJ_PADDING);
    deco->push_back(label);

    const char* name[]={"Current", "Power", "Voltage"};
    const char* unit[]={"A", "W", "V"};
    float *var_ptr[]={CURSEN->get_current_ptr(),  CURSEN->get_power_ptr(),  CURSEN->get_voltage_ptr()};
    uint32_t y3=y+GUI_TILE_OBJECT_PADDING;

    for(uint8_t i=0; i<3; i++)
    {
        field_ptr=new gui_float_field_t(
        nullptr, CURSEN->get_mutex(), var_ptr[i], screen, 
        xfield[1],
        y3,
        GUI_COLOR_TEXT, 0, unit[i], 3);
        unselectable->push_back(field_ptr);

        label=lv_label_create(screen);
        lv_obj_set_style_text_color(label, GUI_COLOR_TEXT, 0);
        lv_label_set_text(label, name[i]);
        lv_obj_set_pos(label, 
        xlabel[1],
        y3+GUI_BACKPLATE_OBJECT_PADDING);
        deco->push_back(label);

        y3+=3*GUI_BACKPLATE_OBJECT_PADDING+GUI_FONT14_HEIGHT;
    }
}

/*=======================================================================================================================================*/
void gui_init_page_starter(std::vector <gui_generic_field_t*>* selectable,
    std::vector <gui_generic_field_t*>* unselectable,
    std::vector <lv_obj_t *>* deco,
    lv_obj_t* screen)
{
    lv_obj_t* object;
    gui_generic_field_t* field;

    field=new gui_trigger_field_t(
        new task_notify_pack_t(task_queue_list[ACT_TASKID], ACT_NTCODE_STEPPER_MANUAL_POS), 
        screen, 20, 60, "+10");
    selectable->push_back(field);

    field=new gui_trigger_field_t(
        new task_notify_pack_t(task_queue_list[ACT_TASKID], ACT_NTCODE_STEPPER_MANUAL_NEG), 
        screen, 20, 90, "-10");
    selectable->push_back(field);


    object=lv_label_create(screen);
    lv_label_set_text(object, "test");
    lv_obj_set_style_text_color(object, GUI_COLOR_TEXT, 0);
    lv_obj_align(object, LV_ALIGN_BOTTOM_MID, 0, 0);
    deco->push_back(object);
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

    field_ptr=new gui_jump_field_t(screen, GUI_MENU_PAGEID_SERV_SET);
    selectable->push_back(field_ptr);
    label=lv_label_create(screen);
    lv_label_set_text(label, "Edit");
    lv_obj_set_style_text_color(label, GUI_COLOR_TEXT, 0);
    lv_obj_update_layout(label);
    lv_obj_align(label, LV_ALIGN_TOP_RIGHT, -30, 5);
}

/*=======================================================================================================================================*/

void gui_init_page_servos_cfg(std::vector <gui_generic_field_t*>* selectable,
    std::vector <gui_generic_field_t*>* unselectable,
    std::vector <lv_obj_t *>* deco,
    lv_obj_t* screen)
{
    lv_obj_t *label, *tile;
    gui_generic_field_t* field_ptr;
    uint8_t ntcodes[]={ACT_NTCODE_UPDATE_SERV0, ACT_NTCODE_UPDATE_SERV1, ACT_NTCODE_UPDATE_SERV2, ACT_NTCODE_UPDATE_SERV3};
    const char* servo_labels[]={"Servo 0", "Servo 1", "Servo 2", "Servo 3"};
    
    const char* names[]={"Servo 0 position 0", "Servo 1 position 0", "Servo 1 position 0", "Servo 1 position 0", "Servo 0 position 1", "Servo 1 position 1", "Servo 1 position 1", "Servo 1 position 1"};
    const char* short_names[]={"Pos. 0", "Pos. 0", "Pos. 0", "Pos. 0", "Pos. 1", "Pos. 1", "Pos. 1", "Pos. 1"};

    uint8_t field_x[]={70, 230};
    uint8_t panel_x[]={0, 160};
    uint8_t panel_y[]={50, 160};


    for(uint8_t row=0; row<2; row++)
    for(uint8_t col=0; col<2; col++)
    {
        /*backplates*/
        tile=lv_obj_create(screen);
        lv_obj_add_style(tile, gui_style_bg_tile, LV_STATE_DEFAULT);
        lv_obj_set_size(tile, 
            150,
            3*GUI_TILE_OBJECT_PADDING+2*GUI_FONT14_HEIGHT+4*GUI_BACKPLATE_OBJECT_PADDING);
        lv_obj_set_pos(tile, panel_x[col], panel_y[row]);
        deco->push_back(tile);

        lv_obj_update_layout(tile);

        /*name label*/
        label=lv_label_create(screen);
        lv_obj_set_style_text_color(label, GUI_COLOR_TEXT, 0);
        lv_label_set_text(label, servo_labels[2*row+col]); 
        lv_obj_update_layout(label);
        lv_obj_align_to(label, tile, LV_ALIGN_OUT_TOP_MID, 0, -GUI_LABEL_OBJ_PADDING);
        deco->push_back(label);

        /*angle 0 field*/
        field_ptr=new gui_float_field_t(
            new task_notify_pack_t(task_queue_list[ACT_TASKID], ntcodes[2*row+col]), 
            servos[row+col].mutex, &(servos[2*row+col].angle[0]), screen, 
            field_x[col]+GUI_TILE_OBJECT_PADDING, 
            panel_y[row]+GUI_TILE_OBJECT_PADDING, 
            GUI_COLOR_TEXT, 0, "°", names[2*row+col], 1, ACT_SERV_MAX_ANGLE_DEG, ACT_SERV_MIN_ANGLE_DEG);
        selectable->push_back(field_ptr);

        /*angle 0 label*/
        label=lv_label_create(screen);
        lv_obj_set_style_text_color(label, GUI_COLOR_TEXT, 0);
        lv_label_set_text(label, short_names[2*row+col]); 
        lv_obj_update_layout(label);
        lv_obj_align_to(label, tile, LV_ALIGN_TOP_LEFT, 0, GUI_BACKPLATE_OBJECT_PADDING);
        deco->push_back(label);

        /*angle 1 field*/
        field_ptr=new gui_float_field_t(
            new task_notify_pack_t(task_queue_list[ACT_TASKID], ntcodes[2*row+col]), 
            servos[row+col].mutex, &(servos[2*row+col].angle[1]), screen, 
            field_x[col]+GUI_TILE_OBJECT_PADDING, 
            panel_y[row]+2*GUI_TILE_OBJECT_PADDING+GUI_FONT14_HEIGHT+2*GUI_BACKPLATE_OBJECT_PADDING, 
            GUI_COLOR_TEXT, 0, "°", names[2*row+col+4], 1, ACT_SERV_MAX_ANGLE_DEG, ACT_SERV_MIN_ANGLE_DEG);
        selectable->push_back(field_ptr);

        /*angle 1 label*/
        label=lv_label_create(screen);
        lv_obj_set_style_text_color(label, GUI_COLOR_TEXT, 0);
        lv_label_set_text(label, short_names[2*row+col+4]); 
        lv_obj_update_layout(label);
        lv_obj_align_to(label, tile, LV_ALIGN_TOP_LEFT, 0, GUI_TILE_OBJECT_PADDING+3*GUI_BACKPLATE_OBJECT_PADDING+GUI_FONT14_HEIGHT);
        deco->push_back(label);
    }

    field_ptr=new gui_jump_field_t(screen, GUI_MENU_PAGEID_SERV_CON);
    selectable->push_back(field_ptr);
    label=lv_label_create(screen);
    lv_label_set_text(label, "Control");
    lv_obj_set_style_text_color(label, GUI_COLOR_TEXT, 0);
    lv_obj_update_layout(label);
    lv_obj_align(label, LV_ALIGN_TOP_RIGHT, -30, 5);
}

/*=======================================================================================================================================*/

void gui_init_page_info(std::vector <gui_generic_field_t*>* selectable,
    std::vector <gui_generic_field_t*>* unselectable,
    std::vector <lv_obj_t *>* deco,
    lv_obj_t* screen)
{
    
}

/*=======================================================================================================================================*/

void gui_init_page_misc_settings(std::vector <gui_generic_field_t*>* selectable,
    std::vector <gui_generic_field_t*>* unselectable,
    std::vector <lv_obj_t *>* deco,
    lv_obj_t* screen)
{
    lv_obj_t *label, *tile;
    uint8_t y, x, xfield, tile_height, tile_width;
    gui_generic_field_t* field_ptr;

    /*=======Screensaver======*/
    y=70; x=0; xfield=105; tile_width=175;
    tile_height=3*GUI_TILE_OBJECT_PADDING+2*GUI_FONT14_HEIGHT+4*GUI_BACKPLATE_OBJECT_PADDING;
    uint8_t yfield[]={
        GUI_TILE_OBJECT_PADDING, 
        2*GUI_TILE_OBJECT_PADDING+2*GUI_BACKPLATE_OBJECT_PADDING+GUI_FONT14_HEIGHT};

    tile=lv_obj_create(screen);
    lv_obj_add_style(tile, gui_style_bg_tile, LV_STATE_DEFAULT);
    lv_obj_set_size(tile, 
        tile_width,
        tile_height);
    lv_obj_set_pos(tile, x, y);
    deco->push_back(tile);

    label=lv_label_create(screen);
    lv_obj_set_style_text_color(label, GUI_COLOR_TEXT, 0);
    lv_label_set_text(label, "Screensaver");
    lv_obj_align_to(label, tile, LV_ALIGN_OUT_TOP_MID, 0, -GUI_LABEL_OBJ_PADDING);
    deco->push_back(label);

    field_ptr=new gui_float_field_t(
        new task_notify_pack_t(task_queue_list[LCD_TASKID], LCD_NTCODE_UPDATE_SETTINGS), lcd_settings.mutex, &(lcd_settings.ss_delay),
        screen, xfield, y+yfield[0], GUI_COLOR_TEXT, 0, "s", "Screensaver delay", 0, LCD_SS_MAX_DELAY_S, LCD_SS_MIN_DELAY_S);
    selectable->push_back(field_ptr);

    label=lv_label_create(screen);
    lv_obj_set_style_text_color(label, GUI_COLOR_TEXT, 0);
    lv_label_set_text(label, "Delay");
    lv_obj_set_pos(label, GUI_TILE_OBJECT_PADDING, y+yfield[0]+GUI_BACKPLATE_OBJECT_PADDING);
    deco->push_back(label);

    field_ptr=new gui_sw_bool_field_t(
        new task_notify_pack_t(task_queue_list[LCD_TASKID], LCD_NTCODE_UPDATE_SETTINGS), lcd_settings.mutex, &(lcd_settings.ss_enabled),
        screen, xfield+GUI_TILE_OBJECT_PADDING, y+yfield[1], true);
    selectable->push_back(field_ptr);

    label=lv_label_create(screen);
    lv_obj_set_style_text_color(label, GUI_COLOR_TEXT, 0);
    lv_label_set_text(label, "Enabled");
    lv_obj_set_pos(label, GUI_TILE_OBJECT_PADDING, y+yfield[1]+GUI_BACKPLATE_OBJECT_PADDING);
    deco->push_back(label);

    /*=======Brightness======*/
    y+=tile_height+GUI_FONT14_HEIGHT+GUI_LABEL_OBJ_PADDING+2*GUI_TILE_OBJECT_PADDING; 

    tile=lv_obj_create(screen);
    lv_obj_add_style(tile, gui_style_bg_tile, LV_STATE_DEFAULT);
    lv_obj_set_size(tile, 
        tile_width,
        2*GUI_TILE_OBJECT_PADDING+GUI_FONT14_HEIGHT+2*GUI_BACKPLATE_OBJECT_PADDING);
    lv_obj_set_pos(tile, x, y);
    deco->push_back(tile);

    label=lv_label_create(screen);
    lv_obj_set_style_text_color(label, GUI_COLOR_TEXT, 0);
    lv_label_set_text(label, "Display");
    lv_obj_align_to(label, tile, LV_ALIGN_OUT_TOP_MID, 0, -GUI_LABEL_OBJ_PADDING);
    deco->push_back(label);

    field_ptr=new gui_float_field_t(
        new task_notify_pack_t(task_queue_list[LCD_TASKID], LCD_NTCODE_UPDATE_SETTINGS), lcd_settings.mutex, &(lcd_settings.brightness),
        screen, xfield, y+yfield[0], GUI_COLOR_TEXT, 0, "%", "Display Brightness", 0, LCD_MAX_BRIGHT, LCD_MIN_BRIGHT);
    selectable->push_back(field_ptr);

    label=lv_label_create(screen);
    lv_obj_set_style_text_color(label, GUI_COLOR_TEXT, 0);
    lv_label_set_text(label, "Brightness");
    lv_obj_set_pos(label, GUI_TILE_OBJECT_PADDING, y+yfield[0]+GUI_BACKPLATE_OBJECT_PADDING);
    deco->push_back(label);



    /*Debug - manual membrane*/
    field_ptr=new gui_sw_bool_field_t(
        new task_notify_pack_t(task_queue_list[ACT_TASKID], ACT_NTCODE_UPDATE_MEMB_NO_COM),
        act_membrane.mutex, &(act_membrane.enabled), screen,
        LCD_WIDTH-GUI_SW_WIDTH-GUI_TILE_OBJECT_PADDING,
        GUI_TILE_OBJECT_PADDING,
        true);
    selectable->push_back(field_ptr);
}

/*=======================================================================================================================================*/

void gui_init_page_placeholder(std::vector <gui_generic_field_t*>* selectable,
    std::vector <gui_generic_field_t*>* unselectable,
    std::vector <lv_obj_t *>* deco,
    lv_obj_t* screen)
{
    lv_obj_t *object;
    object=lv_label_create(screen);
    lv_label_set_text(object, "Placeholder");
    lv_obj_set_style_text_color(object, GUI_COLOR_TEXT, 0);
    lv_obj_align(object, LV_ALIGN_CENTER, 0, 0);
    deco->push_back(object);
}