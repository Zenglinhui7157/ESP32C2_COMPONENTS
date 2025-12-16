#ifndef _COM_DISPLAY_H_
#define _COM_DISPLAY_H_

#include "esp_log.h"
#include "esp_err.h"
#include <stdint.h>

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
