#include "ssd1306.h"
#include "esp_log.h"
#include <string.h>
#include "../FONTS/font.h"

static const char *TAG = "ssd1306";

static uint8_t display_buffer[SSD1306_WIDTH * SSD1306_HEIGHT / 8] __attribute__((used));

static esp_err_t i2c_master_init(void)
{
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = SSD1306_PIN_SDA,
        .scl_io_num = SSD1306_PIN_SCL,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = SSD1306_I2C_FREQ_HZ,
    };
    esp_err_t ret = i2c_param_config(SSD1306_I2C_PORT, &conf);
    if (ret != ESP_OK) {
        return ret;
    }
    ret = i2c_driver_install(SSD1306_I2C_PORT, conf.mode, 0, 0, 0);
    return ret;
}

static esp_err_t ssd1306_write_cmd(uint8_t cmd)
{
    uint8_t data[2] = {0x00, cmd}; // 0x00 for command
    i2c_cmd_handle_t cmd_handle = i2c_cmd_link_create();
    i2c_master_start(cmd_handle);
    i2c_master_write_byte(cmd_handle, (SSD1306_I2C_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write(cmd_handle, data, sizeof(data), true);
    i2c_master_stop(cmd_handle);
    esp_err_t ret = i2c_master_cmd_begin(SSD1306_I2C_PORT, cmd_handle, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd_handle);
    return ret;
}

static esp_err_t ssd1306_write_data(uint8_t *data, size_t len)
{
    i2c_cmd_handle_t cmd_handle = i2c_cmd_link_create();
    i2c_master_start(cmd_handle);
    i2c_master_write_byte(cmd_handle, (SSD1306_I2C_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd_handle, 0x40, true); // 0x40 for data
    i2c_master_write(cmd_handle, data, len, true);
    i2c_master_stop(cmd_handle);
    esp_err_t ret = i2c_master_cmd_begin(SSD1306_I2C_PORT, cmd_handle, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd_handle);
    return ret;
}

static esp_err_t ssd1306_update_display(void)
{
    for (uint8_t page = 0; page < SSD1306_HEIGHT / 8; page++) {
        ssd1306_write_cmd(SSD1306_CMD_COLUMN_ADDR);
        ssd1306_write_cmd(0); // Column start
        ssd1306_write_cmd(SSD1306_WIDTH - 1); // Column end
        ssd1306_write_cmd(SSD1306_CMD_PAGE_ADDR);
        ssd1306_write_cmd(page); // Page start
        ssd1306_write_cmd(page); // Page end
        ssd1306_write_data(&display_buffer[page * SSD1306_WIDTH], SSD1306_WIDTH);
    }
    return ESP_OK;
}

esp_err_t ssd1306_init(void)
{
    ESP_LOGI(TAG, "Initializing SSD1306");

    esp_err_t ret = i2c_master_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C init failed: %s", esp_err_to_name(ret));
        return ret;
    }

    // Initialize display buffer
    memset(display_buffer, 0, sizeof(display_buffer));

    // Initialization sequence
    ssd1306_write_cmd(SSD1306_CMD_DISPLAY_OFF);
    ssd1306_write_cmd(SSD1306_CMD_SET_DISPLAY_CLOCK_DIV);
    ssd1306_write_cmd(0x80);
    ssd1306_write_cmd(SSD1306_CMD_SET_MULTIPLEX);
    ssd1306_write_cmd(SSD1306_HEIGHT - 1);
    ssd1306_write_cmd(SSD1306_CMD_SET_DISPLAY_OFFSET);
    ssd1306_write_cmd(0x00);
    ssd1306_write_cmd(SSD1306_CMD_SET_START_LINE | 0x00);
    ssd1306_write_cmd(SSD1306_CMD_CHARGE_PUMP);
    ssd1306_write_cmd(0x14);
    ssd1306_write_cmd(SSD1306_CMD_MEMORY_MODE);
    ssd1306_write_cmd(0x00);
    ssd1306_write_cmd(SSD1306_CMD_SEG_REMAP | 0x00);
    ssd1306_write_cmd(SSD1306_CMD_COM_SCAN_INC);
    ssd1306_write_cmd(SSD1306_CMD_SET_COM_PINS);
    ssd1306_write_cmd(0x12);
    ssd1306_write_cmd(SSD1306_CMD_SET_CONTRAST);
    ssd1306_write_cmd(0xCF);
    ssd1306_write_cmd(SSD1306_CMD_SET_PRECHARGE);
    ssd1306_write_cmd(0xF1);
    ssd1306_write_cmd(SSD1306_CMD_SET_VCOM_DETECT);
    ssd1306_write_cmd(0x40);
    ssd1306_write_cmd(SSD1306_CMD_DISPLAY_ALL_ON_RESUME);
    ssd1306_write_cmd(SSD1306_CMD_NORMAL_DISPLAY);
    ssd1306_write_cmd(SSD1306_CMD_DISPLAY_ON);
    ssd1306_set_orientation(false); // Normal orientation
    ESP_LOGI(TAG, "SSD1306 initialized");
    return ESP_OK;
}

esp_err_t ssd1306_fill_screen(bool color)
{
    uint8_t fill_value = color ? 0xFF : 0x00;
    memset(display_buffer, fill_value, sizeof(display_buffer));
    return ssd1306_update_display();
}

esp_err_t ssd1306_draw_pixel(uint16_t x, uint16_t y, bool color)
{
    if (x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT) {
        return ESP_ERR_INVALID_ARG;
    }

    uint16_t index = x + (y / 8) * SSD1306_WIDTH;
    uint8_t bit = y % 8;
    (void)index; // Suppress unused variable warning

    if (color) {
        display_buffer[index] |= (1 << bit);
    } else {
        display_buffer[index] &= ~(1 << bit);
    }

    return ssd1306_update_display();
}

esp_err_t ssd1306_draw_char(uint16_t x, uint16_t y, char c, bool color)
{
    if (c < 32 || c > 126) {
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t char_index = c - 32; // Font starts from space (32)

    // 逐行绘制，阴码，逆向（行逆向，列正向）
    for (uint8_t row = 0; row < 12; row++) { // 12 rows
        uint8_t line = char_12x6[char_index][11 - row]; // 逆向行
        for (uint8_t col = 0; col < 6; col++) { // 6 columns per row
            if (line & (1 << col)) { // 阴码，正向列
                ssd1306_draw_pixel(x + col, y + (11 - row), color);
            }
        }
    }

    return ESP_OK;
}

esp_err_t ssd1306_set_orientation(bool normal)
{
    if (normal) {
        // Normal orientation: left to right, top to bottom
        ssd1306_write_cmd(SSD1306_CMD_SEG_REMAP | 0x00);
        ssd1306_write_cmd(SSD1306_CMD_COM_SCAN_INC);
    } else {
        // Inverted orientation: right to left, bottom to top
        ssd1306_write_cmd(SSD1306_CMD_SEG_REMAP | 0x01);
        ssd1306_write_cmd(SSD1306_CMD_COM_SCAN_DEC);
    }
    return ESP_OK;
}