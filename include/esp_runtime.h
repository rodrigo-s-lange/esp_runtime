#pragma once

#include <stdbool.h>

#include "esp_err.h"
#include "esp_at.h"
#include "esp_button.h"
#include "esp_can.h"
#include "esp_easyled.h"
#include "esp_ethernet.h"
#include "esp_gpio.h"
#include "esp_i2c_master.h"
#include "esp_keypad.h"
#include "esp_ota.h"
#include "esp_rs485.h"
#include "esp_scheduler.h"
#include "esp_spi_master.h"
#include "esp_st7789v2.h"
#include "esp_storage.h"
#include "esp_time.h"
#include "esp_webterm.h"
#include "esp_wifi_sta.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    ESP_RUNTIME_MODULE_BUTTON = 0,
    ESP_RUNTIME_MODULE_CAN,
    ESP_RUNTIME_MODULE_EASYLED,
    ESP_RUNTIME_MODULE_ETHERNET,
    ESP_RUNTIME_MODULE_GPIO,
    ESP_RUNTIME_MODULE_I2C_MASTER,
    ESP_RUNTIME_MODULE_KEYPAD,
    ESP_RUNTIME_MODULE_OTA,
    ESP_RUNTIME_MODULE_RS485,
    ESP_RUNTIME_MODULE_SCHEDULER,
    ESP_RUNTIME_MODULE_SPI_MASTER,
    ESP_RUNTIME_MODULE_ST7789V2,
    ESP_RUNTIME_MODULE_STORAGE,
    ESP_RUNTIME_MODULE_TIME,
    ESP_RUNTIME_MODULE_WEBTERM,
    ESP_RUNTIME_MODULE_WIFI_STA,
    ESP_RUNTIME_MODULE_COUNT,
} esp_runtime_module_t;

/**
 * @brief Runtime status snapshot for one managed module.
 */
typedef struct {
    bool initialized;
    bool enabled;
    bool log_enabled;
    bool at_enabled;
    bool supports_at;
    const char *name;
} esp_runtime_status_t;

/**
 * @brief Initialize runtime manager.
 *
 * @param log_enabled Enable internal runtime logs.
 * @param at_enabled  Register runtime AT commands (`AT+ESP`, `AT+ESP?`).
 */
esp_err_t esp_runtime_init(bool log_enabled, bool at_enabled);

/**
 * @brief Disable all active modules and deinitialize runtime manager.
 */
esp_err_t esp_runtime_deinit(void);

/**
 * @brief Report whether runtime manager is initialized.
 */
bool esp_runtime_is_initialized(void);

/**
 * @brief Enable one managed module.
 */
esp_err_t esp_runtime_enable(esp_runtime_module_t module, bool log_enabled, bool at_enabled);

/**
 * @brief Disable one managed module.
 */
esp_err_t esp_runtime_disable(esp_runtime_module_t module);

/**
 * @brief Report whether a given module is currently enabled.
 */
bool esp_runtime_module_is_enabled(esp_runtime_module_t module);

/**
 * @brief Fetch runtime status for one module.
 */
esp_err_t esp_runtime_get_status(esp_runtime_module_t module, esp_runtime_status_t *out_status);

/**
 * @brief Resolve a module enum by name.
 */
esp_err_t esp_runtime_find_module(const char *name, esp_runtime_module_t *out_module);

/**
 * @brief Return canonical module name for AT/status output.
 */
const char *esp_runtime_module_name(esp_runtime_module_t module);

#ifdef __cplusplus
}
#endif
