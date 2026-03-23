# esp_runtime

Runtime manager for project components.

## Purpose

- Centralize includes for the main application.
- Provide a uniform enable/disable/status layer for runtime-managed libs.
- Expose a single AT control surface for module lifecycle.

## Included headers

`esp_runtime.h` re-exports:

- `esp_at.h`
- `esp_button.h`
- `esp_can.h`
- `esp_easyled.h`
- `esp_ethernet.h`
- `esp_gpio.h`
- `esp_i2c_master.h`
- `esp_keypad.h`
- `esp_time.h`
- `esp_ota.h`
- `esp_pca9685.h`
- `esp_rs485.h`
- `esp_scheduler.h`
- `esp_spi_master.h`
- `esp_stepper.h`
- `esp_st7789v2.h`
- `esp_storage.h`
- `esp_wifi_sta.h`

## API

- `esp_runtime_init(log_enabled, at_enabled)`
- `esp_runtime_deinit()`
- `esp_runtime_is_initialized()`
- `esp_runtime_enable(module, log_enabled, at_enabled)`
- `esp_runtime_disable(module)`
- `esp_runtime_module_is_enabled(module)`
- `esp_runtime_get_status(module, &status)`
- `esp_runtime_find_module(name, &module)`
- `esp_runtime_module_name(module)`

## Managed modules

- `BUTTON`
- `CAN`
- `EASYLED`
- `ETHERNET`
- `GPIO`
- `I2C_MASTER`
- `KEYPAD`
- `OTA`
- `PCA9685`
- `RS485`
- `SCHEDULER`
- `SPI_MASTER`
- `STEPPER`
- `ST7789V2`
- `STORAGE`
- `TIME`
- `WIFI_STA`

## Notes

- `esp_at` is not runtime-managed. It is the control backbone and should be initialized before `esp_runtime` when AT control is required.
- Modules that do not support AT today reject `ENABLE,...,TRUE` with `ESP_ERR_NOT_SUPPORTED`.
- Runtime disable depends on each library having a real `deinit()` and unregistering its AT commands.

