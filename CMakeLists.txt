idf_component_register(SRCS  "main.cpp" 
"task_encoder.cpp" 
"task_visual.cpp" 
"task_gui.cpp" 
"task_lcd.cpp" 
"task_sensor.cpp" 
"task_regulator.cpp" 
"task_actuator.cpp" 
"task_comm.cpp" 
"expander.cpp" 
"nvs_stuff.cpp" 
"misc.cpp"
"000_gui/gui_fields.cpp"
"000_gui/gui_main.cpp"
"000_gui/gui_pages.cpp"
"000_gui/gui_menu.cpp"

REQUIRES 
spi_flash 
esp_driver_gpio 
esp_driver_pcnt 
esp_driver_spi 
esp_lcd_ili9341 
nvs_flash driver 
json
INCLUDE_DIRS "")
