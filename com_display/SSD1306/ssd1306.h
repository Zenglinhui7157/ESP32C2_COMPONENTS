#ifndef __SSD1306_H__
#define __SSD1306_H__

#include "esp_log.h"
#include <stdint.h>
#include "driver/i2c.h"

#define SSD1306_I2C_PORT       CONFIG_SSD1306_I2C_PORT
#define SSD1306_PIN_SDA        CONFIG_SSD1306_PIN_SDA
#define SSD1306_PIN_SCL        CONFIG_SSD1306_PIN_SCL
#define SSD1306_I2C_ADDR       CONFIG_SSD1306_I2C_ADDR
#define SSD1306_WIDTH          CONFIG_SSD1306_WIDTH
#define SSD1306_HEIGHT         CONFIG_SSD1306_HEIGHT

#define SSD1306_I2C_FREQ_HZ    400000

// SSD1306 commands
#define SSD1306_CMD_SET_CONTRAST        0x81
#define SSD1306_CMD_DISPLAY_ALL_ON_RESUME 0xA4
#define SSD1306_CMD_DISPLAY_ALL_ON       0xA5
#define SSD1306_CMD_NORMAL_DISPLAY       0xA6
#define SSD1306_CMD_INVERT_DISPLAY       0xA7
#define SSD1306_CMD_DISPLAY_OFF          0xAE
#define SSD1306_CMD_DISPLAY_ON           0xAF
#define SSD1306_CMD_SET_DISPLAY_OFFSET   0xD3
#define SSD1306_CMD_SET_COM_PINS         0xDA
#define SSD1306_CMD_SET_VCOM_DETECT      0xDB
#define SSD1306_CMD_SET_DISPLAY_CLOCK_DIV 0xD5
#define SSD1306_CMD_SET_PRECHARGE        0xD9
#define SSD1306_CMD_SET_MULTIPLEX        0xA8
#define SSD1306_CMD_SET_LOW_COLUMN       0x00
#define SSD1306_CMD_SET_HIGH_COLUMN      0x10
#define SSD1306_CMD_SET_START_LINE       0x40
#define SSD1306_CMD_MEMORY_MODE          0x20
#define SSD1306_CMD_COLUMN_ADDR          0x21
#define SSD1306_CMD_PAGE_ADDR            0x22
#define SSD1306_CMD_COM_SCAN_INC         0xC0
#define SSD1306_CMD_COM_SCAN_DEC         0xC8
#define SSD1306_CMD_SEG_REMAP            0xA0
#define SSD1306_CMD_CHARGE_PUMP          0x8D

/**
 * @brief Initialize the SSD1306 display
 *
 * @return esp_err_t ESP_OK on success
 */
esp_err_t ssd1306_init(void);

/**
 * @brief Fill the screen with a color (true for white, false for black)
 *
 * @param color true for white, false for black
 * @return esp_err_t ESP_OK on success
 */
esp_err_t ssd1306_fill_screen(bool color);

/**
 * @brief Draw a pixel at specified coordinates
 *
 * @param x X coordinate
 * @param y Y coordinate
 * @param color true for white, false for black
 * @return esp_err_t ESP_OK on success
 */
esp_err_t ssd1306_draw_pixel(uint16_t x, uint16_t y, bool color);

/**
 * @brief Draw a character at specified coordinates
 *
 * @param x X coordinate
 * @param y Y coordinate
 * @param c Character to draw
 * @param color true for white, false for black
 * @return esp_err_t ESP_OK on success
 */
esp_err_t ssd1306_draw_char(uint16_t x, uint16_t y, char c, bool color);

/**
 * @brief Set the display orientation
 *
 * @param normal true for normal orientation (left to right, top to bottom), false for inverted
 * @return esp_err_t ESP_OK on success
 */
esp_err_t ssd1306_set_orientation(bool normal);

#endif // __SSD1306_H__