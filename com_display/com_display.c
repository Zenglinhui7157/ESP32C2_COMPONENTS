#include "com_display.h"
#ifdef CONFIG_DISPLAY_TYPE_ST7789
#include "ST7789/st7789.h"
#endif

static const char *TAG = "com_display";

esp_err_t com_display_init(void)
{
    ESP_LOGI(TAG, "com_display_init called");

#ifdef CONFIG_DISPLAY_TYPE_ST7789
    esp_err_t ret = st7789_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "ST7789 init failed: %s", esp_err_to_name(ret));
        return ret;
    }
    ESP_LOGI(TAG, "ST7789 initialized successfully");
    return ESP_OK;
#else
    ESP_LOGW(TAG, "No display type selected in Kconfig");
    return ESP_ERR_NOT_SUPPORTED;
#endif
}

esp_err_t com_display_fill_screen(uint16_t color)
{
#ifdef CONFIG_DISPLAY_TYPE_ST7789
    return st7789_fill_screen(color);
#else
    ESP_LOGW(TAG, "Display fill not supported for current type");
    return ESP_ERR_NOT_SUPPORTED;
#endif
}

esp_err_t com_display_draw_pixel(uint16_t x, uint16_t y, uint16_t color)
{
#ifdef CONFIG_DISPLAY_TYPE_ST7789
    return st7789_draw_pixel(x, y, color);
#else
    ESP_LOGW(TAG, "Display draw pixel not supported for current type");
    return ESP_ERR_NOT_SUPPORTED;
#endif
}

esp_err_t com_display_set_rotation(uint8_t rotation)
{
#ifdef CONFIG_DISPLAY_TYPE_ST7789
    DisplayRotation rot;
    switch (rotation) {
        case 0: rot = DISPLAY_ROTATION_0; break;
        case 90: rot = DISPLAY_ROTATION_90; break;
        case 180: rot = DISPLAY_ROTATION_180; break;
        case 270: rot = DISPLAY_ROTATION_270; break;
        default: return ESP_ERR_INVALID_ARG;
    }
    return st7789_set_rotation(rot);
#else
    ESP_LOGW(TAG, "Display rotation not supported for current type");
    return ESP_ERR_NOT_SUPPORTED;
#endif
}


