#pragma once
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"
#include "freertos/queue.h"

#include "esp_chip_info.h"
#include "esp_system.h"
#include "esp_log.h" // makra do sygnalizacji LOGx
#include "esp_timer.h"

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <cmath>

#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "nvs.h"
#include "nvs_flash.h"
#include <sht3x.h>
#include <mcp23x17.h>
#include <ina219.h>

#include "lvgl.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_ili9341.h"