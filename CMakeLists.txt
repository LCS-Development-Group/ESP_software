idf_component_register(SRCS "main.c" "task_encoder.c" "task_test.c"
                       REQUIRES spi_flash esp_driver_gpio esp_driver_pcnt
                       INCLUDE_DIRS "")
