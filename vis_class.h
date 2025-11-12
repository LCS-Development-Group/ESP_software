#pragma once
#include "common_includes.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_ili9341.h"
#include "graphics.h"

class vis_controller
{

    public:
    vis_controller(esp_lcd_panel_handle_t* _lcd_handle);
};