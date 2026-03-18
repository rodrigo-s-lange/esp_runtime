#include <ctype.h>
#include <string.h>

#include "esp_log.h"
#include "esp_runtime.h"

static const char *TAG = "esp_runtime";

typedef esp_err_t (*esp_runtime_init_fn_t)(bool log_enabled, bool at_enabled);
typedef esp_err_t (*esp_runtime_deinit_fn_t)(void);
typedef bool (*esp_runtime_is_initialized_fn_t)(void);

typedef struct {
    const char *name;
    bool supports_at;
    esp_runtime_init_fn_t init_fn;
    esp_runtime_deinit_fn_t deinit_fn;
    esp_runtime_is_initialized_fn_t is_initialized_fn;
} esp_runtime_module_desc_t;

typedef struct {
    bool initialized;
    bool log_enabled;
    bool at_enabled;
    bool at_registered;
    bool module_log_enabled[ESP_RUNTIME_MODULE_COUNT];
    bool module_at_enabled[ESP_RUNTIME_MODULE_COUNT];
} esp_runtime_state_t;

static esp_runtime_state_t s_runtime = {0};

#define RT_LOGI(...) do { if (s_runtime.log_enabled) ESP_LOGI(TAG, __VA_ARGS__); } while (0)
#define RT_LOGW(...) do { if (s_runtime.log_enabled) ESP_LOGW(TAG, __VA_ARGS__); } while (0)
#define RT_LOGE(...) do { if (s_runtime.log_enabled) ESP_LOGE(TAG, __VA_ARGS__); } while (0)

static const esp_runtime_module_desc_t s_modules[ESP_RUNTIME_MODULE_COUNT] = {
    [ESP_RUNTIME_MODULE_BUTTON] = { "BUTTON", true, esp_button_init, esp_button_deinit, esp_button_is_initialized },
    [ESP_RUNTIME_MODULE_CAN] = { "CAN", false, esp_can_init, esp_can_deinit, esp_can_is_initialized },
    [ESP_RUNTIME_MODULE_EASYLED] = { "EASYLED", true, esp_easyled_init, esp_easyled_deinit, esp_easyled_is_initialized },
    [ESP_RUNTIME_MODULE_ETHERNET] = { "ETHERNET", true, esp_ethernet_init, esp_ethernet_deinit, esp_ethernet_is_initialized },
    [ESP_RUNTIME_MODULE_GPIO] = { "GPIO", true, esp_gpio_init, esp_gpio_deinit, esp_gpio_is_initialized },
    [ESP_RUNTIME_MODULE_I2C_MASTER] = { "I2C_MASTER", false, esp_i2c_master_init, esp_i2c_master_deinit, esp_i2c_master_is_initialized },
    [ESP_RUNTIME_MODULE_KEYPAD] = { "KEYPAD", false, esp_keypad_init, esp_keypad_deinit, esp_keypad_is_initialized },
    [ESP_RUNTIME_MODULE_OTA] = { "OTA", true, esp_ota_init, esp_ota_deinit, esp_ota_is_initialized },
    [ESP_RUNTIME_MODULE_RS485] = { "RS485", false, esp_rs485_init, esp_rs485_deinit, esp_rs485_is_initialized },
    [ESP_RUNTIME_MODULE_SCHEDULER] = { "SCHEDULER", false, esp_scheduler_init, esp_scheduler_deinit, esp_scheduler_is_initialized },
    [ESP_RUNTIME_MODULE_SPI_MASTER] = { "SPI_MASTER", false, esp_spi_master_init, esp_spi_master_deinit, esp_spi_master_is_initialized },
    [ESP_RUNTIME_MODULE_ST7789V2] = { "ST7789V2", true, esp_st7789v2_init, esp_st7789v2_deinit, esp_st7789v2_is_initialized },
    [ESP_RUNTIME_MODULE_STORAGE] = { "STORAGE", true, esp_storage_init, esp_storage_deinit, esp_storage_is_initialized },
    [ESP_RUNTIME_MODULE_TIME] = { "TIME", true, esp_time_init, esp_time_deinit, esp_time_is_initialized },
    [ESP_RUNTIME_MODULE_WEBTERM] = { "WEBTERM", true, esp_webterm_init, esp_webterm_deinit, esp_webterm_is_initialized },
    [ESP_RUNTIME_MODULE_WIFI_STA] = { "WIFI_STA", true, esp_wifi_sta_init, esp_wifi_sta_deinit, esp_wifi_sta_is_initialized },
};

static bool ci_equals(const char *a, const char *b)
{
    if (a == NULL || b == NULL) return false;
    while (*a != '\0' && *b != '\0') {
        if (toupper((unsigned char)*a) != toupper((unsigned char)*b)) {
            return false;
        }
        a++;
        b++;
    }
    return *a == '\0' && *b == '\0';
}

static char *trim_ws(char *s)
{
    if (s == NULL) return NULL;
    while (*s != '\0' && isspace((unsigned char)*s)) s++;
    size_t n = strlen(s);
    while (n > 0 && isspace((unsigned char)s[n - 1])) {
        s[n - 1] = '\0';
        n--;
    }
    return s;
}

static esp_err_t parse_bool_token(const char *value, bool *out)
{
    if (value == NULL || out == NULL) return ESP_ERR_INVALID_ARG;
    if (ci_equals(value, "1") || ci_equals(value, "TRUE") || ci_equals(value, "ON") || ci_equals(value, "ENABLE") || ci_equals(value, "ENABLED")) {
        *out = true;
        return ESP_OK;
    }
    if (ci_equals(value, "0") || ci_equals(value, "FALSE") || ci_equals(value, "OFF") || ci_equals(value, "DISABLE") || ci_equals(value, "DISABLED")) {
        *out = false;
        return ESP_OK;
    }
    return ESP_ERR_INVALID_ARG;
}

static esp_err_t register_at_commands(void);
static void unregister_at_commands(void);
static void handle_at_runtime_query(const char *param);
static void handle_at_runtime(const char *param);

static esp_err_t register_at_commands(void)
{
    if (s_runtime.at_registered) return ESP_OK;

    esp_err_t err = esp_at_register_cmd_example("AT+ESP?", handle_at_runtime_query, "AT+ESP?");
    if (err != ESP_OK) return err;
    err = esp_at_register_cmd_example("AT+ESP", handle_at_runtime, "AT+ESP=BUTTON,ENABLE,TRUE,FALSE");
    if (err != ESP_OK) {
        (void)esp_at_unregister_cmd("AT+ESP?");
        return err;
    }
    err = esp_at_set_help_visible("AT+ESP", false);
    if (err != ESP_OK) {
        (void)esp_at_unregister_cmd("AT+ESP");
        (void)esp_at_unregister_cmd("AT+ESP?");
        return err;
    }

    s_runtime.at_registered = true;
    return ESP_OK;
}

static void unregister_at_commands(void)
{
    if (!s_runtime.at_registered) return;
    (void)esp_at_unregister_cmd("AT+ESP?");
    (void)esp_at_unregister_cmd("AT+ESP");
    s_runtime.at_registered = false;
}

esp_err_t esp_runtime_init(bool log_enabled, bool at_enabled)
{
    if (s_runtime.initialized) return ESP_ERR_INVALID_STATE;
    if (at_enabled && !esp_at_is_initialized()) return ESP_ERR_INVALID_STATE;

    s_runtime.log_enabled = log_enabled;
    s_runtime.at_enabled = at_enabled;
    s_runtime.initialized = true;

    if (s_runtime.at_enabled) {
        esp_err_t err = register_at_commands();
        if (err != ESP_OK) {
            s_runtime = (esp_runtime_state_t){0};
            return err;
        }
    }

    RT_LOGI("initialized (AT=%s)", s_runtime.at_enabled ? "on" : "off");
    return ESP_OK;
}

esp_err_t esp_runtime_deinit(void)
{
    if (!s_runtime.initialized) return ESP_ERR_INVALID_STATE;

    esp_err_t first_err = ESP_OK;
    for (int i = (int)ESP_RUNTIME_MODULE_COUNT - 1; i >= 0; i--) {
        if (s_modules[i].is_initialized_fn != NULL && s_modules[i].is_initialized_fn()) {
            esp_err_t err = esp_runtime_disable((esp_runtime_module_t)i);
            if (first_err == ESP_OK && err != ESP_OK) {
                first_err = err;
            }
        }
    }

    unregister_at_commands();
    s_runtime = (esp_runtime_state_t){0};
    return first_err;
}

bool esp_runtime_is_initialized(void)
{
    return s_runtime.initialized;
}

esp_err_t esp_runtime_enable(esp_runtime_module_t module, bool log_enabled, bool at_enabled)
{
    if (!s_runtime.initialized) return ESP_ERR_INVALID_STATE;
    if (module < 0 || module >= ESP_RUNTIME_MODULE_COUNT) return ESP_ERR_INVALID_ARG;

    const esp_runtime_module_desc_t *desc = &s_modules[module];
    if (desc->is_initialized_fn()) return ESP_ERR_INVALID_STATE;
    if (at_enabled && !desc->supports_at) return ESP_ERR_NOT_SUPPORTED;
    if (at_enabled && !esp_at_is_initialized()) return ESP_ERR_INVALID_STATE;

    esp_err_t err = desc->init_fn(log_enabled, at_enabled);
    if (err != ESP_OK) return err;

    s_runtime.module_log_enabled[module] = log_enabled;
    s_runtime.module_at_enabled[module] = at_enabled;
    RT_LOGI("%s enabled (log=%s at=%s)", desc->name, log_enabled ? "on" : "off", at_enabled ? "on" : "off");
    return ESP_OK;
}

esp_err_t esp_runtime_disable(esp_runtime_module_t module)
{
    if (!s_runtime.initialized) return ESP_ERR_INVALID_STATE;
    if (module < 0 || module >= ESP_RUNTIME_MODULE_COUNT) return ESP_ERR_INVALID_ARG;

    const esp_runtime_module_desc_t *desc = &s_modules[module];
    if (!desc->is_initialized_fn()) {
        s_runtime.module_log_enabled[module] = false;
        s_runtime.module_at_enabled[module] = false;
        return ESP_OK;
    }

    esp_err_t err = desc->deinit_fn();
    if (err != ESP_OK) return err;

    s_runtime.module_log_enabled[module] = false;
    s_runtime.module_at_enabled[module] = false;
    RT_LOGI("%s disabled", desc->name);
    return ESP_OK;
}

bool esp_runtime_module_is_enabled(esp_runtime_module_t module)
{
    if (module < 0 || module >= ESP_RUNTIME_MODULE_COUNT) return false;
    return s_modules[module].is_initialized_fn();
}

esp_err_t esp_runtime_get_status(esp_runtime_module_t module, esp_runtime_status_t *out_status)
{
    if (module < 0 || module >= ESP_RUNTIME_MODULE_COUNT || out_status == NULL) return ESP_ERR_INVALID_ARG;

    const esp_runtime_module_desc_t *desc = &s_modules[module];
    bool enabled = desc->is_initialized_fn();
    out_status->initialized = s_runtime.initialized;
    out_status->enabled = enabled;
    out_status->supports_at = desc->supports_at;
    out_status->log_enabled = enabled ? s_runtime.module_log_enabled[module] : false;
    out_status->at_enabled = enabled ? s_runtime.module_at_enabled[module] : false;
    out_status->name = desc->name;
    return ESP_OK;
}

esp_err_t esp_runtime_find_module(const char *name, esp_runtime_module_t *out_module)
{
    if (name == NULL || out_module == NULL) return ESP_ERR_INVALID_ARG;

    for (int i = 0; i < ESP_RUNTIME_MODULE_COUNT; i++) {
        if (ci_equals(name, s_modules[i].name)) {
            *out_module = (esp_runtime_module_t)i;
            return ESP_OK;
        }
    }
    return ESP_ERR_NOT_FOUND;
}

const char *esp_runtime_module_name(esp_runtime_module_t module)
{
    if (module < 0 || module >= ESP_RUNTIME_MODULE_COUNT) return NULL;
    return s_modules[module].name;
}

static void print_status_one(esp_runtime_module_t module)
{
    esp_runtime_status_t status = {0};
    if (esp_runtime_get_status(module, &status) != ESP_OK) return;

    if (status.supports_at) {
        AT(C "  %-10s" W " : %s log=%s at=%s", status.name,
           status.enabled ? G "ENABLED" W : R "DISABLED" W,
           status.log_enabled ? "TRUE" : "FALSE",
           status.at_enabled ? "TRUE" : "FALSE");
    } else {
        AT(C "  %-10s" W " : %s log=%s at=N/A", status.name,
           status.enabled ? G "ENABLED" W : R "DISABLED" W,
           status.log_enabled ? "TRUE" : "FALSE");
    }
}

static void handle_at_runtime_query(const char *param)
{
    (void)param;
    AT(C "ESP Runtime:");
    for (int i = 0; i < ESP_RUNTIME_MODULE_COUNT; i++) {
        print_status_one((esp_runtime_module_t)i);
    }
}

static void handle_at_runtime(const char *param)
{
    if (param == NULL || param[0] == '\0') {
        AT(R "ERROR: use AT+ESP=<MODULE>,STATUS|ENABLE[,LOG,AT]|DISABLE");
        return;
    }

    char work[128];
    strncpy(work, param, sizeof(work) - 1U);
    work[sizeof(work) - 1U] = '\0';

    char *tokens[4] = {0};
    int token_count = 0;
    char *ctx = NULL;
    char *tok = strtok_r(work, ",", &ctx);
    while (tok != NULL && token_count < 4) {
        tokens[token_count++] = trim_ws(tok);
        tok = strtok_r(NULL, ",", &ctx);
    }

    if (token_count < 2) {
        AT(R "ERROR: use AT+ESP=<MODULE>,STATUS|ENABLE[,LOG,AT]|DISABLE");
        return;
    }

    esp_runtime_module_t module = ESP_RUNTIME_MODULE_BUTTON;
    if (esp_runtime_find_module(tokens[0], &module) != ESP_OK) {
        AT(R "ERROR: modulo invalido");
        return;
    }

    if (ci_equals(tokens[1], "STATUS")) {
        print_status_one(module);
        return;
    }

    if (ci_equals(tokens[1], "DISABLE")) {
        esp_err_t err = esp_runtime_disable(module);
        if (err != ESP_OK) {
            AT(R "ERROR: disable failed (%s)", esp_err_to_name(err));
            return;
        }
        AT(G "OK");
        return;
    }

    if (ci_equals(tokens[1], "ENABLE")) {
        bool log_enabled = false;
        bool at_enabled = false;
        if (token_count >= 3 && parse_bool_token(tokens[2], &log_enabled) != ESP_OK) {
            AT(R "ERROR: log invalido");
            return;
        }
        if (token_count >= 4 && parse_bool_token(tokens[3], &at_enabled) != ESP_OK) {
            AT(R "ERROR: at invalido");
            return;
        }

        esp_err_t err = esp_runtime_enable(module, log_enabled, at_enabled);
        if (err != ESP_OK) {
            AT(R "ERROR: enable failed (%s)", esp_err_to_name(err));
            return;
        }
        AT(G "OK");
        return;
    }

    AT(R "ERROR: acao invalida");
}
