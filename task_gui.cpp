#include "header.h"
#include "gui_class.h"

gui_controller *gui;
SemaphoreHandle_t gui_mutex;

void task_gui_main(void *args)
{
    xEventGroupWaitBits(main_event_group, TASK_START_SYNCBIT, pdFALSE, pdFALSE, portMAX_DELAY);
    if(DEBUG_TASK_ANOUNCE) ESP_LOGI("GUI", "task_gui started");


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
            ESP_LOGE("GUI", "gui_controller creation failed");
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
    /*Menu (Root)*/
    page* page_sensors=root->add_new_page("Sensors", nullptr);
    page* page_regulator=root->add_new_page("Regulation", nullptr);
    page* page_membrane=root->add_new_page("Membrane", new t_notify_package(&task_handle_list[ACT_TASKID], ACT_NTCODE_UPDATE_MEMB));
    page* page_servos=root->add_new_page("Servos", nullptr);//no ntpack?
    page* page_comm=root->add_new_page("Comms", nullptr);
    //page* page_display=root->add_new_page("Display", nullptr);
    page* page_about=root->add_new_page("About", nullptr);

    /*Sensors*/
    //data from sensors, etc
    page_sensors->add_field_to_page(new text_field(""));//a way to make empty line
    page_sensors->add_field_to_page(new text_field("chamber:"));
    page_sensors->add_field_to_page(new float_io_field("Temp.", t_field_io_type::FIELD_OUT, RHT_int->get_T_ptr(), RHT_int->get_mutex_ptr(), " ^", 2, SEN_T_MAX_VAL, SEN_MIN_VAL));
    page_sensors->add_field_to_page(new float_io_field("Humidity", t_field_io_type::FIELD_OUT, RHT_int->get_RH_ptr(), RHT_int->get_mutex_ptr(), " %", 2, SEN_RH_MAX_VAL, SEN_MIN_VAL));
    page_sensors->add_field_to_page(new text_field(""));//a way to make empty line
    page_sensors->add_field_to_page(new text_field("external:"));
    page_sensors->add_field_to_page(new float_io_field("Temp.", t_field_io_type::FIELD_OUT, RHT_ext->get_T_ptr(), RHT_ext->get_mutex_ptr(), " ^", 2, SEN_T_MAX_VAL, SEN_MIN_VAL));
    page_sensors->add_field_to_page(new float_io_field("Humidity", t_field_io_type::FIELD_OUT, RHT_ext->get_RH_ptr(), RHT_ext->get_mutex_ptr(), " %", 2, SEN_RH_MAX_VAL, SEN_MIN_VAL));

    /*Membrane*/
    //mebrane info
    page_membrane->add_field_to_page(new bool_io_field("State", t_field_io_type::FIELD_OUT, &(act_membrane.enabled), &(act_membrane.mutex), "On ", "Off"));
    page_membrane->add_field_to_page(new float_io_field("Current", t_field_io_type::FIELD_OUT, CURSEN->get_current_ptr(), CURSEN->get_mutex_ptr(), " A", 3, SEN_CUR_MAX_VAL, SEN_MIN_VAL));
    page_membrane->add_field_to_page(new float_io_field("Power", t_field_io_type::FIELD_OUT, CURSEN->get_voltage_ptr(), CURSEN->get_mutex_ptr(), " W", 3, SEN_POW_MAX_VAL, SEN_MIN_VAL));
    page_membrane->add_field_to_page(new float_io_field("Voltage", t_field_io_type::FIELD_OUT, CURSEN->get_power_ptr(), CURSEN->get_mutex_ptr(), " V", 3, SEN_VOL_MAX_VAL, SEN_MIN_VAL));
    page_membrane->add_field_to_page(new bool_io_field("Manual_en", t_field_io_type::FIELD_IN, &(act_membrane.enabled), &(act_membrane.mutex), "On ", "Off"));
    page_membrane->add_field_to_page(new float_io_field("Chamber RH", t_field_io_type::FIELD_OUT, RHT_int->get_RH_ptr(), RHT_int->get_mutex_ptr(), " %", 2, SEN_RH_MAX_VAL, SEN_MIN_VAL));

    /*Regulation*/
    //detailed settings and info about regulation
    page_regulator->add_field_to_page(new bool_io_field("Enabled", t_field_io_type::FIELD_IN, &(regulator.enabled), &(regulator.mutex), "On ", "Off"));
    page_regulator->add_field_to_page(new float_io_field("SP", t_field_io_type::FIELD_IN, &(regulator.SP), &(regulator.mutex), " %", 2, SEN_RH_MAX_VAL, SEN_MIN_VAL));
    page_regulator->add_field_to_page(new float_io_field("H", t_field_io_type::FIELD_IN, &(regulator.H), &(regulator.mutex), " %", 2, SEN_RH_MAX_VAL, SEN_MIN_VAL));
    page_regulator->add_field_to_page(new text_field(" "));
    page_regulator->add_field_to_page(new text_field("DEBUG:"));
    page_regulator->add_field_to_page(new float_io_field("RH", t_field_io_type::FIELD_OUT, &(regulator.PV), &(regulator.mutex), " %", 2, SEN_RH_MAX_VAL, SEN_MIN_VAL));
    page_regulator->add_field_to_page(new float_io_field("Error", t_field_io_type::FIELD_OUT, &(regulator.E), &(regulator.mutex), " %", 2, SEN_RH_MAX_VAL, SEN_MIN_VAL));
    page_regulator->add_field_to_page(new bool_io_field("membrane", t_field_io_type::FIELD_OUT, &(regulator.CV), &(regulator.mutex), "On ", "Off"));


    /*servos*/
    //servomechanism control
    // page* page_serv0=page_servos->add_new_page("Serwo 0", new t_notify_package(task_handle_list[ACT_TASKID], ACT_NTCODE_UPDATE_SERV0));
    // page* page_serv1=page_servos->add_new_page("Serwo 1", new t_notify_package(task_handle_list[ACT_TASKID], ACT_NTCODE_UPDATE_SERV1));
    // page* page_serv2=page_servos->add_new_page("Serwo 2", new t_notify_package(task_handle_list[ACT_TASKID], ACT_NTCODE_UPDATE_SERV2));
    page* page_serv3=page_servos->add_new_page("Serwo 3", new t_notify_package(&task_handle_list[ACT_TASKID], ACT_NTCODE_UPDATE_SERV3));

    page_serv3->add_field_to_page(new bool_io_field("Enabled", t_field_io_type::FIELD_IN, &(servos[3].enabled), &(servos[3].mutex), "On ", "Off"));   
    page_serv3->add_field_to_page(new bool_io_field("Angle", t_field_io_type::FIELD_IN, &(servos[3].position), &(servos[3].mutex), "POS_1", "POS_0"));
    page_serv3->add_field_to_page(new float_io_field("POS_0", t_field_io_type::FIELD_IN, &(servos[3].angle[0]), &(servos[3].mutex), "^", 1, ACT_SERV_MAX_ANGLE_DEG, 0.0));
    page_serv3->add_field_to_page(new float_io_field("POS_1", t_field_io_type::FIELD_IN, &(servos[3].angle[1]), &(servos[3].mutex), "^", 1, ACT_SERV_MAX_ANGLE_DEG, 0.0));
    

    /*Comms*/
    page_comm->add_field_to_page(new float_io_field("PERIOD", t_field_io_type::FIELD_IN, &(com_send_period), &(com_send_mutex), " S", 0, COM_SEND_PERIOD_LOOPS_MAX, COM_SEND_PERIOD_LOOPS_MIN));

    /*Display*/
    //brightness, timeout(?)
    //page_display->add_field_to_page(new text_field("WIP"));
    

    /*About*/
    page_about->add_field_to_page(new text_field("Program by:"));
    page_about->add_field_to_page(new text_field("Karol Pach"));
    page_about->add_field_to_page(new text_field("Major version: 1.1"));
    page_about->add_field_to_page(new text_field("LCS 2025"));
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
    SemaphoreHandle_t *_var_mutex,
    std::string _true_text,
    std::string _false_text)
    :io_field<bool>(t_field_type::BOOL_IO , _name, _io, _var, _var_mutex), true_text(_true_text), false_text(_false_text){}

void bool_io_field::switch_bool()
{
    xSemaphoreTake(*var_mutex, portMAX_DELAY);
    *var=!(*var);
    xSemaphoreGive(*var_mutex);
}
std::string bool_io_field::get_true_text() const {return true_text;}
std::string bool_io_field::get_false_text() const {return false_text;}

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
    root=new page("MENU", nullptr, nullptr);
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
                    gui->get_current_page()->notify_associated_task();
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
            gui->get_current_page()->notify_associated_task();
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

page::page(std::string _name, page* _uppage, t_notify_package *_ntpack)
:name(_name), uppage(_uppage), ntpack(_ntpack){}

page::~page()
{
    if(ntpack!=nullptr) delete ntpack;

    for(int i=0; i<page_list.size(); i++)
    if(page_list[i]!=nullptr) delete page_list[i];
}

void page::add_field_to_page(basic_field* new_field)
{
    page_list.push_back(new_field);
}

page* page::add_new_page(std::string _name, t_notify_package *_ntpack)
{
    page* newpage=new page(_name, this, _ntpack);
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
t_notify_package *page::get_ntpack() const {return ntpack;}
void page::notify_associated_task()
{
    if(ntpack!=nullptr)
    {
        xTaskNotifyIndexed(*(ntpack->task_to_notify), 0, ntpack->ntcode, eSetValueWithoutOverwrite);
    }
}