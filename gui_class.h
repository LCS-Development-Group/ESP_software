#pragma once
#include "common_includes.h"

enum t_field_type{TEXT, SUBPAGE_LINK, FLOAT_IO, BOOL_IO};
enum t_field_io_type{FIELD_IN, FIELD_OUT};

#define GUI_RETCODE_REDRAW_ALL              VIS_NTCODE_REDRAW_ALL
#define GUI_RETCODE_REDRAW_SELECT           VIS_NTCODE_REDRAW_SELECT
#define GUI_RETCODE_REDRAW_BAR              VIS_NTCODE_REDRAW_BAR
#define GUI_RETCODE_REDRAW_VALUE            VIS_NTCODE_REDRAW_VALUE
#define GUI_RETCODE_REDRAW_ALL_VALUES       VIS_NTCODE_REDRAW_ALL_VALUES
#define GUI_RETCODE_DEFAULT 255

class basic_field
{
    protected:
    t_field_type field_type;
    std::string name;
    t_field_io_type io;

    public:
    basic_field(t_field_type _field_type, std::string _name, t_field_io_type _io);
    virtual ~basic_field(){}

    t_field_type get_field_type() const;
    std::string get_name() const;
    t_field_io_type get_io() const;
};

class page
{
    std::vector <basic_field*> page_list;
    std::string name;
    page *uppage;

    public:
    
    page(std::string _name, page* _uppage);
    ~page();
    void add_field_to_page(basic_field* new_field);
    page *add_new_page(std::string _name);

    uint8_t get_numof_fields() const;
    basic_field* get_field_ptr(int index) const;
    page* get_uppage_ptr() const;
    std::string get_page_name() const;
};

class text_field: public basic_field
{
    public:
    text_field(std::string _name):basic_field(TEXT, _name, t_field_io_type::FIELD_OUT){}
};

class page_link_field :public basic_field
{   
    page* linked_page;
    public:
    
    page_link_field(std::string _name, page* _linked_page);
    ~page_link_field();
    page* get_page_ptr();
};

template <typename var_type>
class io_field : public basic_field
{
    protected:
    var_type *var;                  //field variable
    SemaphoreHandle_t *var_mutex;   //variable mutex

    public:
    io_field(
        t_field_type _field_type, 
        std::string _name, 
        t_field_io_type _io, 
        var_type *_var,
        SemaphoreHandle_t *_var_mutex)
        :basic_field(_field_type, _name, _io),
        var(_var), var_mutex(_var_mutex)
    {
        if(var==nullptr || var_mutex==nullptr)
        {
            ESP_LOGE("GUI", "io_field initialized with nullptr pointer");
            exit(-1);
        }
    }
    virtual ~io_field(){}

    var_type get_val() const
    {
        var_type copy;
        xSemaphoreTake(*var_mutex, portMAX_DELAY);
        copy=*var;
        xSemaphoreGive(*var_mutex);
        return copy;
    }

    var_type* get_var_ptr() const {return var;}
    SemaphoreHandle_t *get_var_mutex() const {return var_mutex;}
};


class bool_io_field: public io_field<bool>
{
    public:
    bool_io_field(
        std::string _name, 
        t_field_io_type _io, 
        bool *_var,
        SemaphoreHandle_t *_var_mutex);

    void switch_bool();
};

class float_io_field: public io_field<float>
{
    
    std::string unit;
    
    uint8_t prec;

    public:
    float_io_field(
        std::string _name, 
        t_field_io_type _io, 
        float *_var,
        SemaphoreHandle_t *_var_mutex,
        /*derived class arguments*/
        std::string _unit,
        uint8_t _prec);

    std::string get_unit() const;
    uint8_t get_prec() const;

    void set_val(float new_val);
};

class gui_controller
{
    page *root;
    page *current_page;

    //cursors
    uint8_t prim_idx, prev_prim_idx;
    bool prim_lock;
    uint8_t sec_idx, prev_sec_idx;
    bool sec_lock;

    //temp. variables (to avoid repeated allocations)
    bool_io_field* bool_io_field_ptr;
    float_io_field* float_io_field_ptr;
    page_link_field* link_field_ptr;

    int temp_int, temp_int2;
    float *float_ptr; 

public:
    gui_controller();
    ~gui_controller();
    void fill_fields();
    uint8_t move_cursor_up();
    uint8_t move_cursor_down();
    uint8_t enter();
    uint8_t go_back();

    page* get_current_page() const;
    bool get_prim_lock() const;
    bool get_sec_lock() const;
    uint8_t get_prim_idx() const;
    uint8_t get_sec_idx() const;
    uint8_t get_prev_prim_idx() const;
    uint8_t get_prev_sec_idx() const;

    bool_io_field* cast_to_bool_io(basic_field* field) const;
    float_io_field* cast_to_float_io(basic_field* field) const;
    page_link_field* cast_to_page_link(basic_field* field) const;

    bool check_if_displayed(bool* _var) const;
    bool check_if_displayed(float* _var) const;
private:
    bool check_if_displayed_excluding(bool* _var, uint8_t excluded) const;
    bool check_if_displayed_excluding(float* _var, uint8_t excluded) const;

    void jump_pages(page* newpage);
    uint8_t find_next_editable(uint8_t current) const;
    uint8_t find_prev_editable(uint8_t current) const;
};