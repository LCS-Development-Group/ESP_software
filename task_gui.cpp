#include "header.h"
#include "gui_class.h"

gui_controller *gui;
SemaphoreHandle_t gui_mutex;

void task_gui_main(void *args)
{
    xEventGroupWaitBits(main_event_group, TASK_START_SYNCBIT, pdFALSE, pdFALSE, portMAX_DELAY);
    ESP_LOGI("Gui", "task_gui started");
    
    vTaskDelay(portMAX_DELAY); //temporary
}

void gui_init()
{
    if(gui==nullptr)
    {
        gui=new gui_controller;
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
    root->add_field_to_page(new bool_io_field("State", FIELD_IN, &DEBUG_BOOL, &DEBUG_BOOL_MUT));
    root->add_field_to_page(new text_field("field 3"));
    // root->add_field_to_page(new float_io_field("float_in ", FIELD_IN, &f_var, "mm", 2, 3));
    root->add_field_to_page(new float_io_field("float_out", FIELD_OUT, &DEBUG_FLOAT, &DEBUG_FLOAT_MUT, "mA", 2, 3));
    page* subpage_1=root->add_new_page("link");

    /*subpage test*/
    subpage_1->add_field_to_page(new text_field("test"));
    // subpage_1->add_field_to_page(new bool_io_field("State_i", FIELD_IN, &debug_bool));
    // subpage_1->add_field_to_page(new bool_io_field("State_o", FIELD_OUT, &debug_bool));
}

//==================================================================================================================
// BASIC FIELD                                                                                             
//==================================================================================================================

basic_field::basic_field(t_field_type _field_type, std::string _name)
:field_type(_field_type), name(_name){}

t_field_type basic_field::get_field_type() const
{
    return field_type;
}

std::string basic_field::get_name() const
{
    return name;
}



//==================================================================================================================
// SUBPAGE LINK FIELD                                                                                               
//==================================================================================================================

page_link_field::page_link_field(std::string _name, page* _linked_page)
:basic_field(SUBPAGE_LINK, _name), linked_page(_linked_page){}

page_link_field::~page_link_field()
{
    if(linked_page!=nullptr) delete linked_page;
}

page* page_link_field::get_page_ptr(){return linked_page;}


//==================================================================================================================
// BOOL_IO FIELD                                                                                                    
//==================================================================================================================

bool_io_field::bool_io_field(std::string _name, t_field_io_type _io, bool* _var, SemaphoreHandle_t *_mutex)
:basic_field(BOOL_IO, _name), io(_io), var(_var), mutex(_mutex){}
t_field_io_type bool_io_field::get_io() const {return io;}
bool bool_io_field::get_val() const
{
    xSemaphoreTake(*mutex, portMAX_DELAY);
    bool copy=*var;
    xSemaphoreGive(*mutex);
    return copy;
}

void bool_io_field::switch_bool()
{
    xSemaphoreTake(*mutex, portMAX_DELAY);
    *var=!(*var);
    xSemaphoreGive(*mutex);
}

//==================================================================================================================
// FLOAT_IO FIELD                                                                                                   
//==================================================================================================================

float_io_field::float_io_field(std::string _name, t_field_io_type _io, float* _var, SemaphoreHandle_t *_mutex, std::string _unit, uint8_t _prec_pref, uint8_t _prec_pos)
:basic_field(FLOAT_IO, _name), io(_io), var(_var), mutex(_mutex), unit(_unit), prec_pref(_prec_pref), prec_pos(_prec_pos){}
t_field_io_type float_io_field::get_io() const {return io;}
float float_io_field::get_val() const
{
    xSemaphoreTake(*mutex, portMAX_DELAY);
    float copy=*var;
    xSemaphoreGive(*mutex);
    return copy;
}
std::string float_io_field::get_unit() const
{
    return unit;
}
uint8_t float_io_field::get_prec_pref() const {return prec_pref;}
uint8_t float_io_field::get_prec_pos() const {return prec_pos;}
uint8_t float_io_field::get_total_num_digits() const {return prec_pref+prec_pos;}
void float_io_field::set_val(float new_val)
{
    xSemaphoreTake(*mutex, portMAX_DELAY);
    *var=new_val;
    xSemaphoreGive(*mutex);
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
    sec_idx=-1;
    sec_lock=false;

    bool_io_field_ptr=nullptr;
    float_io_field_ptr=nullptr;
    link_field_ptr=nullptr;
}

gui_controller::~gui_controller()
{
    delete root;
}

void gui_controller::move_cursor_up()
{
    if(prim_lock)
    {
        float_io_field_ptr=dynamic_cast<float_io_field*>(current_page->get_field_ptr(prim_idx));
        if(sec_lock)//edycja liczby
        {

        }
        else
        {
            if(sec_idx<float_io_field_ptr->get_total_num_digits()-1) 
            {   
                sec_idx++;
                //redraw=1;
            }
        }
    }
    else
        {
        if(prim_idx>0) 
        {
            prim_idx--;
            //redraw=1;
        }
        else if(current_page->get_uppage_ptr()!=nullptr && prim_idx==0)
        {
            prim_idx=-1;
            //redraw=1;
        }
    }
}

void gui_controller::move_cursor_down()
{
    if(prim_lock)
    {
        float_io_field_ptr=dynamic_cast<float_io_field*>(current_page->get_field_ptr(prim_idx));
        if(sec_lock)//edycja liczby
        {

        }
        else
        {
            if(sec_idx>-1) 
            {   
                sec_idx--;
                // redraw=1;
            }
        }
    }
    else
    {
        if(prim_idx<current_page->get_numof_fields()-1) 
        {
            prim_idx++;
            //redraw=1;
        }
    }
}
void gui_controller::enter()
{
    if(prim_idx==-1)
    {//wracamy poziom wyżej
        if(current_page->get_uppage_ptr()==nullptr) /*std::cerr<<"no uppage yet \"Back\" exists"<<std::endl*/;
        else
        {
            jump_pages(current_page->get_uppage_ptr());
            //redraw=1;
        }
    }
    else
    {
        t_field_type field_type=current_page->get_field_ptr(prim_idx)->get_field_type();
        
    
        switch(field_type)
        {
        case TEXT:
            break;

        case SUBPAGE_LINK:
            link_field_ptr=dynamic_cast<page_link_field*>(current_page->get_field_ptr(prim_idx));
            jump_pages(link_field_ptr->get_page_ptr());
            //redraw=1;
            break;

        case BOOL_IO:
            bool_io_field_ptr=dynamic_cast<bool_io_field*>(current_page->get_field_ptr(prim_idx));
            if(bool_io_field_ptr->get_io()==FIELD_IN)
            {
                bool_io_field_ptr->switch_bool();
                //redraw=1;
            }
            break;

        case FLOAT_IO:
            float_io_field_ptr=dynamic_cast<float_io_field*>(current_page->get_field_ptr(prim_idx));
            if(float_io_field_ptr->get_io()==FIELD_IN)
            {
                if(prim_lock)
                {//edycja
                    //std::cout<<"CL";
                    if(sec_idx==-1) 
                    {//wyjście
                        prim_lock=false;
                    }
                    else
                    {//edycja cyfry
                        sec_lock=!sec_lock;
                    }
                    //redraw=1;
                }
                else prim_lock=true;
            }
            break;
        
        default:
            //std::cerr<<"Enter: field type with undefined behaviour"<<std::endl;
            break;
        }
    }
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

void gui_controller::jump_pages(page* newpage)
{
    current_page=newpage;
    if(current_page->get_uppage_ptr()==nullptr) prim_idx=0;
    else prim_idx=-1;
}

page* gui_controller::get_current_page() const {return current_page;};
bool gui_controller::get_prim_lock() const {return prim_lock;}
bool gui_controller::get_sec_lock() const {return sec_idx;}
int8_t gui_controller::get_prim_idx() const {return prim_idx;}
int8_t gui_controller::get_sec_idx() const {return sec_idx;}