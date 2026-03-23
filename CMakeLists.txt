idf_component_register(
    SRCS "esp_runtime.c"
    INCLUDE_DIRS "include"
    REQUIRES esp_at esp_button esp_can esp_easyled esp_ethernet esp_gpio esp_i2c_master esp_keypad esp_ota esp_pca9685 esp_rs485 esp_scheduler esp_spi_master esp_stepper esp_st7789v2 esp_storage esp_time esp_wifi_sta
)
