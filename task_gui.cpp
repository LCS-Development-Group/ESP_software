#include "header.h"
#include "gui_class.h"

gui_controller *gui;
SemaphoreHandle_t gui_mutex;

void task_gui_main(void *args)
{
    xEventGroupWaitBits(main_event_group, TASK_START_SYNCBIT, pdFALSE, pdFALSE, portMAX_DELAY);
    ESP_LOGI("Gui", "task_gui started");


    uint32_t ntcode=0x00;
    uint8_t retcode=GUI_RETCODE_DEFAULT;
    while(true)
    {
        if(xTaskNotifyWaitIndexed(0, 0x00, 0xff, &ntcode, portMAX_DELAY)==pdPASS)
        {
            xSemaphoreTake(gui_mutex, portMAX_DELAY);
            switch(ntcode)
            {
                case GUI_NTCODE_CUR_NEG:
                    //ESP_LOGI("GUI", "UP");
                    retcode=gui->move_cursor_down();
                    
                    break;

                case GUI_NTCODE_CUR_POS:
                    //ESP_LOGI("GUI", "DOWN");
                    retcode=gui->move_cursor_up();
                    break;

                case GUI_NTCODE_CUR_ENT:
                    //ESP_LOGI("GUI", "ENTER");
                    retcode=gui->enter();
                    break;

                case GUI_NTCODE_CUR_BCK:
                    retcode=gui->go_back();
                    //ESP_LOGI("GUI", "BACK");
                    break;

                default:
                    ESP_LOGW("GUI", "Woken by unknown ntcode: %d", ntcode);
                    break;
            }
            xSemaphoreGive(gui_mutex);

            if(retcode!=GUI_RETCODE_DEFAULT)
            {
                xTaskNotifyIndexed(task_handle_list[VIS_TASKID], 0, retcode, eSetValueWithoutOverwrite);
                retcode=GUI_RETCODE_DEFAULT;
            }
        }
    }
}

void gui_init()
{
    if(gui==nullptr)
    {
        gui=new gui_controller;
        if(gui==nullptr)
        {
            ESP_LOGE("GUI", "vis_controller creation failed");
            exit(-1);
        }
        gui->fill_fields();
    }

    gui_mutex=xSemaphoreCreateMutex();
    if(gui_mutex==nullptr)
    {
        ESP_LOGE("GUI", "mutex creation failed");
        exit(-1);
    }
}

/*GUI _CLASS AND RELATED*/

void gui_controller::fill_fields()
{
    /*Menu (root)*/
    root->add_field_to_page(new text_field("test"));
    root->add_field_to_page(new float_io_field("f_i", FIELD_IN, &DEBUG_FLOAT, &DEBUG_FLOAT_MUT, "Ab", 3, 500.0, 0.0));
    root->add_field_to_page(new float_io_field("f2", FIELD_IN, &DEBUG_FLOAT_2, &DEBUG_FLOAT_2_MUT, "Ab", 2, 500.0, 0.0));
    root->add_field_to_page(new bool_io_field("State", FIELD_IN, &DEBUG_BOOL, &DEBUG_BOOL_MUT));
    //root->add_field_to_page(new bool_io_field("State", FIELD_IN, &DEBUG_BOOL, &DEBUG_BOOL_MUT));
    root->add_field_to_page(new text_field("field3"));
    // root->add_field_to_page(new float_io_field("float_in ", FIELD_IN, &f_var, "mm", 2, 3));
    root->add_field_to_page(new float_io_field("float_o", FIELD_OUT, &DEBUG_FLOAT, &DEBUG_FLOAT_MUT, "Ab", 3, 500.0, 0.0));
    page* subpage_1=root->add_new_page("link");

    /*subpage test*/
    subpage_1->add_field_to_page(new text_field("test"));
    subpage_1->add_field_to_page(new bool_io_field("State_1", FIELD_IN, &DEBUG_BOOL, &DEBUG_BOOL_MUT));
    subpage_1->add_field_to_page(new bool_io_field("State", FIELD_OUT, &DEBUG_BOOL, &DEBUG_BOOL_MUT));
    // subpage_1->add_field_to_page(new bool_io_field("State_i", FIELD_IN, &debug_bool));
    // subpage_1->add_field_to_page(new bool_io_field("State_o", FIELD_OUT, &debug_bool));

    prim_idx=find_next_editable(GUI_CURSOR_MAX_INDEX);//cursor starting position
}

//==================================================================================================================
// BASIC FIELD                                                                                             
//==================================================================================================================

basic_field::basic_field(t_field_type _field_type, std::string _name, t_field_io_type _io)
:field_type(_field_type), name(_name), io(_io){}

t_field_type basic_field::get_field_type() const{return field_type;}
std::string basic_field::get_name() const {return name;}
t_field_io_type basic_field::get_io() const {return io;}


//==================================================================================================================
// SUBPAGE LINK FIELD                                                                                               
//==================================================================================================================

page_link_field::page_link_field(std::string _name, page* _linked_page)
:basic_field(SUBPAGE_LINK, _name, t_field_io_type::FIELD_IN), linked_page(_linked_page){}

page_link_field::~page_link_field()
{
    if(linked_page!=nullptr) delete linked_page;
}
page* page_link_field::get_page_ptr(){return linked_page;}


//==================================================================================================================
// BOOL_IO FIELD                                                                                                    
//==================================================================================================================

bool_io_field::bool_io_field(
    std::string _name, 
    t_field_io_type _io, 
    bool *_var,
    SemaphoreHandle_t *_var_mutex)
    :io_field<bool>(t_field_type::BOOL_IO , _name, _io, _var, _var_mutex){}

void bool_io_field::switch_bool()
{
    xSemaphoreTake(*var_mutex, portMAX_DELAY);
    *var=!(*var);
    xSemaphoreGive(*var_mutex);
}

//==================================================================================================================
// FLOAT_IO FIELD                                                                                                   
//==================================================================================================================
float_io_field::float_io_field( 
    std::string _name, 
    t_field_io_type _io, 
    float *_var,
    SemaphoreHandle_t *_var_mutex,
    /*derived class arguments*/
    std::string _unit,
    uint8_t _prec,
    float _var_max,
    float _var_min)
    :io_field<float>(t_field_type::FLOAT_IO, _name, _io, _var, _var_mutex),
    unit(_unit), prec(_prec), var_max(_var_max), var_min(_var_min)
{
    point_pos=0;
    update_point_pos();
}

std::string float_io_field::get_unit() const {return unit;}

uint8_t float_io_field::get_prec() const {return prec;}

void float_io_field::set_val(float new_val)
{
    xSemaphoreTake(*var_mutex, portMAX_DELAY);
    *var=new_val;
    xSemaphoreGive(*var_mutex);
}

uint8_t float_io_field::get_point_pos() const {return point_pos;}
uint8_t float_io_field::get_numof_digits() const
{
    return point_pos+prec;
}

void float_io_field::update_point_pos()
{
    xSemaphoreTake(*var_mutex, portMAX_DELAY);
    uint16_t float_var=(uint16_t)*var;
    xSemaphoreGive(*var_mutex);
    
    if(float_var==0) 
    {
        point_pos=1;
    }
    else
    {
        uint8_t order=0;
        while(float_var!=0)
        {
            float_var/=10;
            order++;
        }
        point_pos=order;
    }
}

bool float_io_field::increment(int8_t power)
{
    float temp=std::powf(10.0, power), new_var;
    bool retcode;
    xSemaphoreTake(*var_mutex, portMAX_DELAY);
    new_var=*var+temp;
    if(new_var>var_max) retcode=GUI_FLOAT_XXCREMENT_NOCHANGE;
    else if(new_var<var_min) retcode=GUI_FLOAT_XXCREMENT_NOCHANGE;
    else
    {
        retcode=GUI_FLOAT_XXCREMENT_CHANGE;
        *var=new_var;
    }
    xSemaphoreGive(*var_mutex);
    return retcode;
}
bool float_io_field::decrement(int8_t power)
{
    float temp=std::powf(10.0, power), new_var;
    bool retcode;
    xSemaphoreTake(*var_mutex, portMAX_DELAY);
    new_var=*var-temp;
    if(new_var>var_max) retcode=GUI_FLOAT_XXCREMENT_NOCHANGE;
    else if(new_var<var_min) retcode=GUI_FLOAT_XXCREMENT_NOCHANGE;
    else
    {
        retcode=GUI_FLOAT_XXCREMENT_CHANGE;
        *var=new_var;
    }
    xSemaphoreGive(*var_mutex);
    return retcode;
}


//==================================================================================================================
// Controller                                                                                                  
//==================================================================================================================

gui_controller::gui_controller()
{
    root=new page("MENU", nullptr);
    current_page=root;
    prim_idx=0;
    prim_lock=false;
    prev_prim_idx=GUI_CURSOR_MAX_INDEX;

    sec_idx=GUI_CURSOR_MAX_INDEX;
    sec_lock=false;
    prev_sec_idx=GUI_CURSOR_MAX_INDEX;

    bool_io_field_ptr=nullptr;
    float_io_field_ptr=nullptr;
    link_field_ptr=nullptr;
}

gui_controller::~gui_controller()
{
    delete root;
}

uint8_t gui_controller::move_cursor_up()
{
    if(prim_lock==true)
    {
        if(current_page->get_field_ptr(prim_idx)->get_field_type()==t_field_type::FLOAT_IO)//to be certain
        {
            float_io_field_ptr=cast_to_float_io(current_page->get_field_ptr(prim_idx));//i hope this is in fact a float_io_bool
            if(sec_lock==true)
            {//field varaible edition
                float_io_field_ptr->update_point_pos();

                int8_t power=float_io_field_ptr->get_point_pos()-sec_idx;
                if(sec_idx<float_io_field_ptr->get_point_pos()) power--;

                if(float_io_field_ptr->increment(power)==GUI_FLOAT_XXCREMENT_CHANGE) return GUI_RETCODE_REDRAW_VALUE;
                else return GUI_RETCODE_DEFAULT;
            }
            else
            {//move secondary cursor right
                if(sec_idx<float_io_field_ptr->get_numof_digits())//temp
                {
                    
                    if(sec_idx==float_io_field_ptr->get_point_pos()-1)//decimal point
                    {
                        if(float_io_field_ptr->get_prec()!=0)//there is something after it
                        {
                            prev_sec_idx=sec_idx;
                            sec_idx+=2;
                            return GUI_RETCODE_REDRAW_BAR;
                        }
                        else return GUI_RETCODE_DEFAULT;
                    }
                    prev_sec_idx=sec_idx;
                    sec_idx++;
                    return GUI_RETCODE_REDRAW_BAR;
                }
            }
        }
    }
    else
    {//move primary cursor up (decrement)
        if(prim_idx>0)
        {
            uint8_t prev=find_prev_editable(prim_idx);
            if(prev!=GUI_CURSOR_MAX_INDEX)
            {
                prev_prim_idx=prim_idx;
                prim_idx=prev;
                return GUI_RETCODE_REDRAW_SELECT;
            }
            else return GUI_RETCODE_DEFAULT;
        }
    }
    return GUI_RETCODE_DEFAULT;//for safety
}

uint8_t gui_controller::move_cursor_down()
{
    if(prim_lock==true)
    {
        if(current_page->get_field_ptr(prim_idx)->get_field_type()==t_field_type::FLOAT_IO)//to be certain
        {
            float_io_field_ptr=cast_to_float_io(current_page->get_field_ptr(prim_idx));
            if(sec_lock==true)
            {//field varaible edition
                uint8_t test=float_io_field_ptr->get_point_pos();
                float_io_field_ptr->update_point_pos();

                int8_t power=float_io_field_ptr->get_point_pos()-sec_idx;
                if(sec_idx<float_io_field_ptr->get_point_pos()) power--;

                if(float_io_field_ptr->decrement(power)==GUI_FLOAT_XXCREMENT_CHANGE) return GUI_RETCODE_REDRAW_VALUE;
                else return GUI_RETCODE_DEFAULT;
            }
            else
            {//move secondary cursor left
                if(sec_idx>0) 
                {
                    
                    if(sec_idx==float_io_field_ptr->get_point_pos()+1)//decimal point
                    {
                        prev_sec_idx=sec_idx;
                        sec_idx-=2;
                        return GUI_RETCODE_REDRAW_BAR;
                    }
                    prev_sec_idx=sec_idx;
                    sec_idx--;
                    return GUI_RETCODE_REDRAW_BAR;
                }
            }
        }
    }
    else
    {//move primary cursor down (increment)
        if(prim_idx<(current_page->get_numof_fields()-1))
        {
            uint8_t next=find_next_editable(prim_idx);
            if(next!=GUI_CURSOR_MAX_INDEX)
            {
                prev_prim_idx=prim_idx;
                prim_idx=next;
                return GUI_RETCODE_REDRAW_SELECT;
            }
            else return GUI_RETCODE_DEFAULT;
        }
    }
    return GUI_RETCODE_DEFAULT;//for safety
}

uint8_t gui_controller::enter()
{
    if(prim_idx==GUI_CURSOR_MAX_INDEX) return GUI_RETCODE_DEFAULT;

    if(prim_lock==true)
    {//switching digit edition
        sec_lock=!sec_lock;
        return GUI_RETCODE_REDRAW_BAR;
    }
    else
    {//entering field
        switch(current_page->get_field_ptr(prim_idx)->get_field_type())
        {
            case t_field_type::TEXT:
                return GUI_RETCODE_DEFAULT;

            case t_field_type::SUBPAGE_LINK:
                link_field_ptr=cast_to_page_link(current_page->get_field_ptr(prim_idx));
                if(link_field_ptr->get_page_ptr()!=nullptr)
                {
                    jump_pages(link_field_ptr->get_page_ptr());
                    return GUI_RETCODE_REDRAW_ALL;
                }
                break;

            case t_field_type::BOOL_IO:
                if(current_page->get_field_ptr(prim_idx)->get_io()==t_field_io_type::FIELD_OUT)
                {//output - editting disallowed
                    return GUI_RETCODE_DEFAULT;
                }
                else
                {//input - editting allowed (bool so no entering to secondary cursor level)
                    bool_io_field_ptr=cast_to_bool_io(current_page->get_field_ptr(prim_idx));
                    bool_io_field_ptr->switch_bool();
                    if(check_if_displayed_excluding(bool_io_field_ptr->get_var_ptr(), prim_idx)==true) return GUI_RETCODE_REDRAW_ALL_VALUES;
                    else return GUI_RETCODE_REDRAW_VALUE;
                }
                break;

            case t_field_type::FLOAT_IO:
                if(current_page->get_field_ptr(prim_idx)->get_io()==t_field_io_type::FIELD_OUT)
                {//output - editting disallowed
                    return GUI_RETCODE_DEFAULT;
                }
                else
                {//input - editting allowed
                    prim_lock=true;
                    sec_idx=0;
                    prev_sec_idx=GUI_CURSOR_MAX_INDEX;
                    return GUI_RETCODE_REDRAW_VALUE_EDITMODE;
                }
                break;
        }
    }

    return GUI_RETCODE_DEFAULT;//for safety
}

uint8_t gui_controller::go_back()
{
    if(prim_lock==true)
    {
        if(sec_lock==true)
        {//leaving digit edition
            //redraw specific field
            
        }
        else
        {//leaving field editing
            prim_lock=false;
            prev_sec_idx=sec_idx;
            sec_idx=GUI_CURSOR_MAX_INDEX;
            return GUI_RETCODE_REDRAW_ALL_VALUES;
        }
    }
    else
    {
        if(current_page->get_uppage_ptr()!=nullptr)
        {//return to previous page
            jump_pages(current_page->get_uppage_ptr());
            return GUI_RETCODE_REDRAW_ALL;
        }
    }
    return GUI_RETCODE_DEFAULT;//for safety
}

void gui_controller::jump_pages(page* newpage)
{
    current_page=newpage;
    prim_idx=find_next_editable(GUI_CURSOR_MAX_INDEX);
    prev_prim_idx=GUI_CURSOR_MAX_INDEX;
}

page* gui_controller::get_current_page() const {return current_page;};
bool gui_controller::get_prim_lock() const {return prim_lock;}
bool gui_controller::get_sec_lock() const {return sec_lock;}
uint8_t gui_controller::get_prim_idx() const {return prim_idx;}
uint8_t gui_controller::get_sec_idx() const {return sec_idx;}
uint8_t gui_controller::get_prev_prim_idx() const {return prev_prim_idx;}
uint8_t gui_controller::get_prev_sec_idx() const {return prev_sec_idx;}
uint8_t gui_controller::find_next_editable(uint8_t current) const
{
    for(uint8_t idx=current+1; idx<current_page->get_numof_fields(); idx++)
        if(current_page->get_field_ptr(idx)->get_io()==t_field_io_type::FIELD_IN) return idx;
    return GUI_CURSOR_MAX_INDEX;
}
uint8_t  gui_controller::find_prev_editable(uint8_t current) const
{
    if(current==GUI_CURSOR_MAX_INDEX) return GUI_CURSOR_MAX_INDEX;
    for(uint8_t idx=current-1;; idx--)
    {
        if(current_page->get_field_ptr(idx)->get_io()==t_field_io_type::FIELD_IN) return idx;
        if(idx==0) break;//compiler had problem with uint8_t condition being always true (I it's mean correct) even though overflow is a thing
    }
    return GUI_CURSOR_MAX_INDEX;
}

bool_io_field* gui_controller::cast_to_bool_io(basic_field* field) const
{return static_cast<bool_io_field*>(field);}
float_io_field* gui_controller::cast_to_float_io(basic_field* field) const
{return static_cast<float_io_field*>(field);}
page_link_field* gui_controller::cast_to_page_link(basic_field* field) const
{return static_cast<page_link_field*>(field);}

bool gui_controller::check_if_displayed(bool* _var) const {return check_if_displayed_excluding(_var, GUI_CURSOR_MAX_INDEX);}
bool gui_controller::check_if_displayed(float* _var) const {return check_if_displayed_excluding(_var, GUI_CURSOR_MAX_INDEX);}

bool gui_controller::check_if_displayed_excluding(bool* _var, uint8_t excluded) const
{
    for(uint8_t idx=0; idx<current_page->get_numof_fields(); idx++)
    {
        if(idx==excluded) continue;
        if(cast_to_bool_io(current_page->get_field_ptr(idx))->get_var_ptr()==_var) return true;
    }
    return false;
}
bool gui_controller::check_if_displayed_excluding(float* _var, uint8_t excluded) const
{
    for(uint8_t idx=0; idx<current_page->get_numof_fields(); idx++)
    {
        if(idx==excluded) continue;
        if(cast_to_float_io(current_page->get_field_ptr(idx))->get_var_ptr()==_var) return true;
    }
    return false;
}

//==================================================================================================================
// Page                                                                                                  
//==================================================================================================================

page::page(std::string _name, page* _uppage)
:name(_name), uppage(_uppage){}

page::~page()
{
    for(int i=0; i<page_list.size(); i++)
    if(page_list[i]!=nullptr) delete page_list[i];
}

void page::add_field_to_page(basic_field* new_field)
{
    page_list.push_back(new_field);
}

page* page::add_new_page(std::string _name)
{
    page* newpage=new page(_name, this);
    page_list.push_back(new page_link_field(_name, newpage));
    return newpage;
}

uint8_t page::get_numof_fields() const {return page_list.size();}

basic_field* page::get_field_ptr(int index) const
{
    if(index>=page_list.size()) return nullptr;
    else return page_list[index];
}

page* page::get_uppage_ptr() const {return uppage;}

std::string page::get_page_name() const {return name;}