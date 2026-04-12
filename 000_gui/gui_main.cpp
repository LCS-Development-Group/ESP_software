#include "../000_gui.h"
#include "../header.h"

lv_style_t *gui_style_menu_def;
lv_style_t *gui_style_menu_sel;
lv_style_t *gui_style_bg_tile;
void gui_setup_global_styles()
{
    //menu list default
    gui_style_menu_def=new lv_style_t;
    lv_style_init(gui_style_menu_def);
    lv_style_set_bg_color(gui_style_menu_def, GUI_COLOR_TILE_BG);
    lv_style_set_bg_opa(gui_style_menu_def, LV_OPA_COVER);
    lv_style_set_radius(gui_style_menu_def, GUI_TILE_CORNER_RADIUS);
    lv_style_set_pad_all(gui_style_menu_def, 6);
    lv_style_set_text_color(gui_style_menu_def, GUI_COLOR_TEXT);

    //menu list selected
    gui_style_menu_sel=new lv_style_t;
    lv_style_init(gui_style_menu_sel);
    lv_style_set_bg_color(gui_style_menu_sel, GUI_COLOR_SELECT);
    lv_style_set_bg_opa(gui_style_menu_sel, LV_OPA_COVER);
    lv_style_set_radius(gui_style_menu_sel, GUI_TILE_CORNER_RADIUS);
    lv_style_set_pad_all(gui_style_menu_sel, 6);
    lv_style_set_text_color(gui_style_menu_sel, GUI_COLOR_TEXT);

    //background tile
    gui_style_bg_tile=new lv_style_t;
    lv_style_init(gui_style_bg_tile);
    lv_style_set_pad_all(gui_style_bg_tile, GUI_TILE_OBJECT_PADDING);
    lv_style_set_radius(gui_style_bg_tile, GUI_TILE_CORNER_RADIUS);
    lv_style_set_bg_color(gui_style_bg_tile, GUI_COLOR_TILE_BG);
    lv_style_set_border_width(gui_style_bg_tile, 0);
}



gui_controller_t::gui_controller_t()
{
    //vars
    mutex=xSemaphoreCreateMutex();
    if(mutex==NULL) exit(-1);

    page_index=0;
    in_page=false;
    list_top=0;

    editor_ptr=new gui_editor_t;
    if(editor_ptr==nullptr) exit(-1);

    //background
    screen=lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen, GUI_COLOR_PAGE_BG, 0);

    //info headers
    uint8_t pos=(LCD_WIDTH-GUI_MENU_ENTRY_WIDTH_PX)/2+GUI_MENU_ENTRY_WIDTH_PX;
    header=lv_label_create(screen);
    lv_label_set_text(header, "Chamber " SETTING_CHAMBER_ID_STATIC);
    lv_obj_set_style_text_color(header, GUI_COLOR_TEXT, 0);
     lv_obj_update_layout(header);
    lv_obj_set_pos(header, pos-(lv_obj_get_width(header)/2), GUI_MENU_ENTRY_SPACING_PX);


    soft_ver=lv_label_create(screen);
    lv_label_set_text(soft_ver, "LCS " SETTING_SOFTWARE_VERSION);
    lv_obj_set_style_text_color(soft_ver, GUI_COLOR_TEXT, 0);
    lv_obj_set_align(soft_ver, LV_ALIGN_BOTTOM_RIGHT);


    //finally
    this->fill_gui();
    this->show_list();
    lv_screen_load(screen);
}

gui_controller_t::~gui_controller_t(){}

void gui_controller_t::cmd_next()
{
    if(in_page==true)
    {//on a page
        page_list[page_index]->cmd_next();
    }
    else
    {//in menu
        select_next_list_entry();
    }
}

void gui_controller_t::cmd_prev()
{
    if(in_page==true)
    {//on a page
        page_list[page_index]->cmd_prev();
    }
    else
    {//in menu
        select_prev_list_entry();
    }
}

void gui_controller_t::cmd_enter()
{
    if(in_page==true)
    {//on a page
        uint8_t code=page_list[page_index]->cmd_enter();
        if(code!=GUI_ENTER_NO_EVENT)
        {
            if(code==GUI_ENTER_BACK_EVENT) leave_page();
            else
            {
                leave_page();
                set_page_index(code);
                enter_page();
            }
        }
    }
    else
    {//in menu
        enter_page();
    }    
}

void gui_controller_t::cmd_back()
{
    if(in_page==true)
    {//on a page
        if(page_list[page_index]->cmd_back()==true) 
            leave_page();
    } 
}

void gui_controller_t::select_next_list_entry()
{
    if(page_index==(page_list.size()-1)) return;
    
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
    if(page_index==0) return;

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
    page_list[page_index]->load_page();
}

void gui_controller_t::leave_page()
{
    in_page=false;
    page_list[page_index]->unload_page();
    lv_screen_load(screen);
}

void gui_controller_t::cmd_update_page()
{
    page_list[page_index]->cmd_update_page();  
}