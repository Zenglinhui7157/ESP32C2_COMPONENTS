#ifndef __ST7789_H__
#define __ST7789_H__

#include "esp_log.h"
#include <stdint.h>
#include "driver/spi_master.h"
#include "driver/gpio.h"

#define USE_KCONFIG_PIN_MAPPING 1

#define TFT_WIDTH  240
#define TFT_HEIGHT 240

#define ST7789_SPI_MODE 0 //模式0才能正常显示
#define ST7789_SPI_CLOCK_HZ 40000000 // 40 MHz

#define COLOR_RGB(r,g,b) (((r&0xF8)<<8)|((g&0xFC)<<3)|(b>>3))
#define COLOR_RED     0xF800
#define COLOR_GREEN   0x07E0
#define COLOR_BLUE    0x001F
#define COLOR_WHITE   0xFFFF
#define COLOR_BLACK   0x0000

#if USE_KCONFIG_PIN_MAPPING
// 使用Kconfig配置
#ifndef CONFIG_ST7789_WIDTH
#define CONFIG_ST7789_WIDTH 240
#endif
#ifndef CONFIG_ST7789_HEIGHT
#define CONFIG_ST7789_HEIGHT 240
#endif
#ifndef CONFIG_ST7789_PIN_MOSI
#define CONFIG_ST7789_PIN_MOSI 7
#endif
#ifndef CONFIG_ST7789_PIN_SCLK
#define CONFIG_ST7789_PIN_SCLK 6
#endif
#ifndef CONFIG_ST7789_PIN_CS
#define CONFIG_ST7789_PIN_CS 3
#endif
#ifndef CONFIG_ST7789_PIN_DC
#define CONFIG_ST7789_PIN_DC 10
#endif
#ifndef CONFIG_ST7789_PIN_RST
#define CONFIG_ST7789_PIN_RST 4
#endif
#ifndef CONFIG_ST7789_PIN_BL
#define CONFIG_ST7789_PIN_BL 18
#endif
#ifndef CONFIG_ST7789_SPI_HOST
#define CONFIG_ST7789_SPI_HOST 1
#endif

#define TFT_WIDTH  CONFIG_ST7789_WIDTH
#define TFT_HEIGHT CONFIG_ST7789_HEIGHT
#define ST7789_PIN_NUM_MOSI  CONFIG_ST7789_PIN_MOSI
#define ST7789_PIN_NUM_CLK   CONFIG_ST7789_PIN_SCLK
#define ST7789_PIN_NUM_CS    CONFIG_ST7789_PIN_CS
#define ST7789_PIN_NUM_DC    CONFIG_ST7789_PIN_DC
#define ST7789_PIN_NUM_RST   CONFIG_ST7789_PIN_RST
#define ST7789_PIN_NUM_BLK   CONFIG_ST7789_PIN_BL
#else
#define TFT_WIDTH  240
#define TFT_HEIGHT 240
// 手动定义引脚
#define ST7789_PIN_NUM_MOSI  7
#define ST7789_PIN_NUM_CLK   6
#define ST7789_PIN_NUM_CS    3
#define ST7789_PIN_NUM_DC    10
#define ST7789_PIN_NUM_RST   4
#define ST7789_PIN_NUM_BLK   18
#endif

typedef enum {
	DISPLAY_ROTATION_0  = 0,
	DISPLAY_ROTATION_90,
	DISPLAY_ROTATION_180,
	DISPLAY_ROTATION_270,
} DisplayRotation;

esp_err_t st7789_init(void);
esp_err_t st7789_fill_screen(uint16_t color);
esp_err_t st7789_write_cmd(uint8_t cmd);
esp_err_t st7789_write_data(const uint8_t *data, int len);
esp_err_t st7789_reset(void);
esp_err_t st7789_set_rotation(DisplayRotation rotation);
esp_err_t st7789_set_window(uint16_t xStart, uint16_t xEnd, uint16_t yStart, uint16_t yEnd);
esp_err_t st7789_draw_pixel(uint16_t x, uint16_t y, uint16_t color);
esp_err_t st7789_fill_screen(uint16_t color);
esp_err_t st7789_write_pixels(const uint8_t *data, size_t len);
esp_err_t st7789_flush_buffer(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint8_t *data);

#endif
