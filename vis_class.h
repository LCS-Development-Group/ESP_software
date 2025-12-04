#pragma once
#include "common_includes.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_ili9341.h"
#include "graphics.h"
#include "gui_class.h"

#define LCD_MAX_LINES                   (240/VIS_FONT_H) //including page name
#define LCD_MAX_CHARS_PER_LINE          (320/VIS_FONT_W) //including margin
#define LCD_FIELD_VALUE_START           12 //where field value is displayed
#define LCD_L_MARGIN                    1   

class vis_controller
{
    esp_lcd_panel_handle_t* lcd_handle;

    uint16_t *fullscreen_bitmask;

    public:
    vis_controller(esp_lcd_panel_handle_t* _lcd_handle);
    ~vis_controller();

    void draw_page();
    void draw_current_value();
    void draw_editmode();
    void draw_all_values();
    void draw_select();
    void draw_bar();
    void start();

    private:
    void clear();
    void clear_field(uint8_t line);
    void draw_value(uint8_t line);
    void draw_names();
    void draw_bool_io_field(bool_io_field* bool_io_field_ptr, uint8_t line);
    void draw_float_io_field(float_io_field *float_io_field_ptr, uint8_t line);
    void draw_text(std::string text, uint8_t line, uint8_t pos);
    void draw_bar_at(uint8_t row, uint8_t col, uint8_t type);
};