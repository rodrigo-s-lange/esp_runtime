# Examples

## Minimal main

```c
#include "esp_runtime.h"
#include "esp_log.h"

static const char *TAG = "main";

void app_main(void)
{
    ESP_LOGI(TAG, "init at");
    ESP_ERROR_CHECK(esp_at_init(false));

    ESP_LOGI(TAG, "init runtime");
    ESP_ERROR_CHECK(esp_runtime_init(false, true));

    ESP_ERROR_CHECK(esp_runtime_enable(ESP_RUNTIME_MODULE_ST7789V2, false, true));
    ESP_ERROR_CHECK(esp_st7789v2_set_rotation(ESP_ST7789V2_ROTATION_0));
    ESP_ERROR_CHECK(esp_st7789v2_fill_screen(BLACK));
}
```

## Enable a module from firmware

```c
ESP_ERROR_CHECK(esp_runtime_enable(ESP_RUNTIME_MODULE_BUTTON, true, true));
```

## Disable a module from firmware

```c
ESP_ERROR_CHECK(esp_runtime_disable(ESP_RUNTIME_MODULE_BUTTON));
```
