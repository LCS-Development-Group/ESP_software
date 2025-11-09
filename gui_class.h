#pragma once
#include "common_includes.h"

enum t_field_type{TEXT, SUBPAGE_LINK, ONOFF_SWITCH, FLOAT_IO, BOOL_IO};
enum t_field_io_type{FIELD_IN, FIELD_OUT};

class basic_field
{
    protected:
    t_field_type field_type;
    std::string name;

    public:
    basic_field(t_field_type _field_type, std::string _name);
    virtual ~basic_field(){}

    t_field_type get_field_type() const;
    std::string get_name() const;
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
    text_field(std::string _name):basic_field(TEXT, _name){}
};

class page_link_field :public basic_field
{   
    page* linked_page;
    public:
    
    page_link_field(std::string _name, page* _linked_page);
    ~page_link_field();
    page* get_page_ptr();
};


class bool_io_field: public basic_field
{
    t_field_io_type io;
    bool *var;//pointer to assioted variable
    SemaphoreHandle_t *mutex;

    public:
    bool_io_field(std::string _name, t_field_io_type _io, bool* _var, SemaphoreHandle_t *_mutex);
    t_field_io_type get_io() const;
    bool get_val() const; //value at call
    void switch_bool();
};

class float_io_field: public basic_field
{
    t_field_io_type io;
    float *var;//pointer to assioted variable
    SemaphoreHandle_t *mutex; //value at call
    std::string unit;
    int8_t prec_pref, prec_pos;

    public:
    float_io_field(std::string _name, t_field_io_type _io, float* _var, SemaphoreHandle_t *_mutex, std::string _unit, uint8_t _prec_pref, uint8_t _prec_pos);
    
    t_field_io_type get_io() const;
    float get_val() const;
    std::string get_unit() const;
    uint8_t get_prec_pref() const;
    uint8_t get_prec_pos() const;
    uint8_t get_total_num_digits() const;

    void set_val(float new_val);
};

class gui_controller
{
    page *root;
    page *current_page;

    //cursors
    int8_t prim_idx;
    bool prim_lock;
    int8_t sec_idx;
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
    void display_curr_list();
    void move_cursor_up();
    void move_cursor_down();
    void enter();

    page* get_current_page() const;
    bool get_prim_lock() const;
    bool get_sec_lock() const;
    int8_t get_prim_idx() const;
    int8_t get_sec_idx() const;
    
private:
    void jump_pages(page* newpage);
};