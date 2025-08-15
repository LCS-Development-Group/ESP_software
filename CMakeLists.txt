idf_component_register(SRCS "visual.c" "control.c" "main.c" "task_encoder.c"
                       REQUIRES spi_flash esp_driver_gpio esp_driver_pcnt
                       INCLUDE_DIRS "")
