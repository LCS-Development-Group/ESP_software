idf_component_register(SRCS "main.c" "task_encoder.c" "task_visual.c" "task_controller.c"
                       REQUIRES spi_flash esp_driver_gpio esp_driver_pcnt esp_driver_spi esp_lcd_ili9341
                       INCLUDE_DIRS "")
