#ifndef _COM_DISPLAY_H_
#define _COM_DISPLAY_H_

#include "esp_log.h"
#include "esp_err.h"
#include <stdint.h>

#if defined(CONFIG_DISPLAY_TYPE_ST7789)
#include "ST7789/st7789.h"
#elif defined(CONFIG_DISPLAY_TYPE_ST7735S)
#include "ST7735S/st7735s.h"
#elif defined(CONFIG_DISPLAY_TYPE_SSD1306)
#include "SSD1306/ssd1306.h"
#endif

#define COM_DISPLAY_INIT_TAG "com_display_init"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the display component
 *
 * @return esp_err_t ESP_OK on success
 */
esp_err_t com_display_init(void);

/**
 * @brief Fill the screen with a color
 *
 * @param color The color to fill (RGB565)
 * @return esp_err_t ESP_OK on success
 */
esp_err_t com_display_fill_screen(uint16_t color);

/**
 * @brief Draw a pixel at specified coordinates
 *
 * @param x X coordinate
 * @param y Y coordinate
 * @param color Color of the pixel (RGB565)
 * @return esp_err_t ESP_OK on success
 */
esp_err_t com_display_draw_pixel(uint16_t x, uint16_t y, uint16_t color);

/**
 * @brief Draw a character at specified coordinates
 *
 * @param x X coordinate
 * @param y Y coordinate
 * @param c Character to draw
 * @param color Color of the character (RGB565)
 * @return esp_err_t ESP_OK on success
 */
esp_err_t com_display_draw_char(uint16_t x, uint16_t y, char c, uint16_t color);

/**
 * @brief Draw a string at specified coordinates
 *
 * @param x X coordinate
 * @param y Y coordinate
 * @param str String to draw
 * @param color Color of the string (RGB565)
 * @return esp_err_t ESP_OK on success
 */
esp_err_t com_display_draw_string(uint16_t x, uint16_t y, const char *str, uint16_t color);

/**
 * @brief Set display orientation
 *
 * @param normal true for normal orientation (left to right, top to bottom), false for inverted
 * @return esp_err_t ESP_OK on success
 */
esp_err_t com_display_set_orientation(bool normal);

/**
 * @brief Set display rotation
 *
 * @param rotation Rotation angle
 * @return esp_err_t ESP_OK on success
 */
esp_err_t com_display_set_rotation(uint8_t rotation);

#ifdef __cplusplus
}
#endif

#endif
