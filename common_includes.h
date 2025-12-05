#pragma once
#include <stdio.h>
#include <stdlib.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_system.h"
#include "esp_log.h" // makra do sygnalizacji LOGx

/*mine*/
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include <vector>
#include <string>
#include "freertos/semphr.h"
#include <cmath>
#include "nvs.h"
#include "nvs_flash.h"