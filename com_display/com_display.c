#include "com_display.h"
#ifdef CONFIG_DISPLAY_TYPE_ST7789
#include "ST7789/st7789.h"
#endif
#ifdef CONFIG_DISPLAY_TYPE_SSD1306
#include "SSD1306/ssd1306.h"
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
#elif defined(CONFIG_DISPLAY_TYPE_SSD1306)
    esp_err_t ret = ssd1306_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "SSD1306 init failed: %s", esp_err_to_name(ret));
        return ret;
    }
    ESP_LOGI(TAG, "SSD1306 initialized successfully");
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
#elif defined(CONFIG_DISPLAY_TYPE_SSD1306)
    // For SSD1306, color 0 is black, non-zero is white
    return ssd1306_fill_screen(color != 0);
#else
    ESP_LOGW(TAG, "Display fill not supported for current type");
    return ESP_ERR_NOT_SUPPORTED;
#endif
}

esp_err_t com_display_draw_pixel(uint16_t x, uint16_t y, uint16_t color)
{
#ifdef CONFIG_DISPLAY_TYPE_ST7789
    return st7789_draw_pixel(x, y, color);
#elif defined(CONFIG_DISPLAY_TYPE_SSD1306)
    // For SSD1306, color 0 is black, non-zero is white
    return ssd1306_draw_pixel(x, y, color != 0);
#else
    ESP_LOGW(TAG, "Display draw pixel not supported for current type");
    return ESP_ERR_NOT_SUPPORTED;
#endif
}

esp_err_t com_display_draw_char(uint16_t x, uint16_t y, char c, uint16_t color)
{
#ifdef CONFIG_DISPLAY_TYPE_ST7789
    // ST7789 draw char not implemented yet
    ESP_LOGW(TAG, "ST7789 draw char not supported");
    return ESP_ERR_NOT_SUPPORTED;
#elif defined(CONFIG_DISPLAY_TYPE_SSD1306)
    // For SSD1306, color 0 is black, non-zero is white
    return ssd1306_draw_char(x, y, c, color != 0);
#else
    ESP_LOGW(TAG, "Display draw char not supported for current type");
    return ESP_ERR_NOT_SUPPORTED;
#endif
}

esp_err_t com_display_draw_string(uint16_t x, uint16_t y, const char *str, uint16_t color)
{
    if (str == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    uint16_t current_x = x;
    for (size_t i = 0; str[i] != '\0'; i++) {
        esp_err_t ret = com_display_draw_char(current_x, y, str[i], color);
        if (ret != ESP_OK) {
            return ret;
        }
        // Assuming character width is 6 pixels
        current_x += 6;
    }
    return ESP_OK;
}

esp_err_t com_display_set_orientation(bool normal)
{
#ifdef CONFIG_DISPLAY_TYPE_ST7789
    // ST7789 orientation not implemented yet
    ESP_LOGW(TAG, "ST7789 orientation not supported");
    return ESP_ERR_NOT_SUPPORTED;
#elif defined(CONFIG_DISPLAY_TYPE_SSD1306)
    return ssd1306_set_orientation(normal);
#else
    ESP_LOGW(TAG, "Display orientation not supported for current type");
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
#elif defined(CONFIG_DISPLAY_TYPE_SSD1306)
    // For SSD1306, map rotation to orientation
    bool normal;
    switch (rotation) {
        case 0: normal = true; break;  // Normal orientation
        case 180: normal = false; break;  // Inverted orientation
        default: return ESP_ERR_INVALID_ARG;  // SSD1306 does not support 90/270 degree rotation easily
    }
    return ssd1306_set_orientation(normal);
#else
    ESP_LOGW(TAG, "Display rotation not supported for current type");
    return ESP_ERR_NOT_SUPPORTED;
#endif
}


