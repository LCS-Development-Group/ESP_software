idf_component_register(SRCS  "main.cpp" "task_encoder.cpp" "task_visual.cpp" "task_gui.cpp" "task_sensor.cpp" "task_regulator.cpp" "task_actuator.cpp" "task_comm.cpp" "font.cpp" "expander.cpp"
                       REQUIRES spi_flash esp_driver_gpio esp_driver_pcnt esp_driver_spi esp_lcd_ili9341
                       INCLUDE_DIRS "")
