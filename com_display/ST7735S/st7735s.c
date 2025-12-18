#include "st7735s.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "../FONTS/font.h"

#define TAG "ST7735S"

static spi_device_handle_t st7735s_spi;

static esp_err_t st7735s_gpio_init(void)
{
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL << ST7735S_PIN_NUM_DC) | (1ULL << ST7735S_PIN_NUM_RST) | (1ULL << ST7735S_PIN_NUM_BLK),
        .pull_down_en = 0,
        .pull_up_en = 0
    };
    esp_err_t ret = gpio_config(&io_conf);
    if (ret != ESP_OK) return ret;
    ret = gpio_set_level(ST7735S_PIN_NUM_BLK, 1);
    return ret;
}

esp_err_t st7735s_init(void)
{
    ESP_LOGI(TAG, "st7735s_init called");
    esp_err_t ret = st7735s_gpio_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "GPIO init failed: %d", ret);
        return ret;
    }

    spi_host_device_t spi_host;
    switch(CONFIG_ST7735S_SPI_HOST) {
        case 0: spi_host = SPI1_HOST; break;
        case 1: spi_host = SPI2_HOST; break;
        default: spi_host = SPI2_HOST; break;
    }

    spi_bus_config_t buscfg = {
        .mosi_io_num = ST7735S_PIN_NUM_MOSI,
        .miso_io_num = -1,
        .sclk_io_num = ST7735S_PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = TFT_WIDTH * TFT_HEIGHT * 2 + 8
    };
    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = ST7735S_SPI_CLOCK_HZ,
        .mode = ST7735S_SPI_MODE,
        .spics_io_num = ST7735S_PIN_NUM_CS,
        .queue_size = 7,
        .pre_cb = NULL
    };
    ret = spi_bus_initialize(spi_host, &buscfg, SPI_DMA_CH_AUTO);
    if(ret != ESP_OK)
    {
        ESP_LOGE(TAG, "spi_bus_initialize failed: %d", ret);
        return ret;
    }
    ret = spi_bus_add_device(spi_host, &devcfg, &st7735s_spi);
    if(ret != ESP_OK)
    {
        ESP_LOGE(TAG, "spi_bus_add_device failed: %d", ret);
        return ret;
    }
    ret = st7735s_reset();

    // ST7735S初始化命令序列
    ret = st7735s_write_cmd(0x01); // SWRESET
    vTaskDelay(pdMS_TO_TICKS(150));

    ret = st7735s_write_cmd(0x11); // SLPOUT
    vTaskDelay(pdMS_TO_TICKS(255));

    ret = st7735s_write_cmd(0xB1); // FRMCTR1
    uint8_t frmctr1[] = {0x01, 0x2C, 0x2D};
    ret = st7735s_write_data(frmctr1, sizeof(frmctr1));

    ret = st7735s_write_cmd(0xB2); // FRMCTR2
    uint8_t frmctr2[] = {0x01, 0x2C, 0x2D};
    ret = st7735s_write_data(frmctr2, sizeof(frmctr2));

    ret = st7735s_write_cmd(0xB3); // FRMCTR3
    uint8_t frmctr3[] = {0x01, 0x2C, 0x2D, 0x01, 0x2C, 0x2D};
    ret = st7735s_write_data(frmctr3, sizeof(frmctr3));

    ret = st7735s_write_cmd(0xC0); // PWCTR1
    uint8_t pwctr1[] = {0xA2, 0x02, 0x84};
    ret = st7735s_write_data(pwctr1, sizeof(pwctr1));

    ret = st7735s_write_cmd(0xC1); // PWCTR2
    uint8_t pwctr2 = 0xC5;
    ret = st7735s_write_data(&pwctr2, 1);

    ret = st7735s_write_cmd(0xC2); // PWCTR3
    uint8_t pwctr3[] = {0x0A, 0x00};
    ret = st7735s_write_data(pwctr3, sizeof(pwctr3));

    ret = st7735s_write_cmd(0xC3); // PWCTR4
    uint8_t pwctr4[] = {0x8A, 0x2A};
    ret = st7735s_write_data(pwctr4, sizeof(pwctr4));

    ret = st7735s_write_cmd(0xC4); // PWCTR5
    uint8_t pwctr5[] = {0x8A, 0xEE};
    ret = st7735s_write_data(pwctr5, sizeof(pwctr5));

    ret = st7735s_write_cmd(0xC5); // VMCTR1
    uint8_t vmctr1 = 0x0E;
    ret = st7735s_write_data(&vmctr1, 1);

    ret = st7735s_write_cmd(0x20); // INVOFF

    // 移除初始化时的MADCTL设置，让set_rotation处理
    // ret = st7735s_write_cmd(0x36); // MADCTL
    // uint8_t madctl = 0xC8; // 默认旋转 - RGB, Row/Column Exchange
    // ret = st7735s_write_data(&madctl, 1);

    ret = st7735s_write_cmd(0x3A); // COLMOD
    uint8_t colmod = 0x05; // 16-bit RGB565
    ret = st7735s_write_data(&colmod, 1);

    ret = st7735s_write_cmd(0xE0); // GMCTRP1
    uint8_t gmctrp1[] = {0x0F, 0x1A, 0x0F, 0x18, 0x2F, 0x28, 0x20, 0x22, 0x1F, 0x1B, 0x23, 0x37, 0x00, 0x07, 0x02, 0x10};
    ret = st7735s_write_data(gmctrp1, sizeof(gmctrp1));

    ret = st7735s_write_cmd(0xE1); // GMCTRN1
    uint8_t gmctrn1[] = {0x0F, 0x1B, 0x0F, 0x17, 0x33, 0x2C, 0x29, 0x2E, 0x30, 0x30, 0x39, 0x3F, 0x00, 0x07, 0x03, 0x10};
    ret = st7735s_write_data(gmctrn1, sizeof(gmctrn1));

    ret = st7735s_write_cmd(0x13); // NORON
    vTaskDelay(pdMS_TO_TICKS(10));

    ret = st7735s_write_cmd(0x29); // DISPON
    vTaskDelay(pdMS_TO_TICKS(100));

    ret = st7735s_set_rotation(DISPLAY_ROTATION_0); // 默认旋转

    ESP_LOGI(TAG, "st7735s init finished");
    return ESP_OK;
}

esp_err_t st7735s_reset(void)
{
    ESP_LOGI(TAG, "st7735s_reset called");
    esp_err_t ret = gpio_set_level(ST7735S_PIN_NUM_RST, 0);
    if (ret != ESP_OK) return ret;
    vTaskDelay(pdMS_TO_TICKS(150));
    ret = gpio_set_level(ST7735S_PIN_NUM_RST, 1);
    if (ret != ESP_OK) return ret;
    vTaskDelay(pdMS_TO_TICKS(150));
    return ESP_OK;
}

esp_err_t st7735s_write_cmd(uint8_t cmd)
{
    gpio_set_level(ST7735S_PIN_NUM_DC, 0);
    spi_transaction_t t = {
        .length = 8,
        .tx_buffer = &cmd
    };
    return spi_device_transmit(st7735s_spi, &t);
}

esp_err_t st7735s_write_data(const uint8_t *data, int len)
{
    gpio_set_level(ST7735S_PIN_NUM_DC, 1);
    spi_transaction_t t = {
        .length = len * 8,
        .tx_buffer = data
    };
    return spi_device_transmit(st7735s_spi, &t);
}

esp_err_t st7735s_set_rotation(DisplayRotation rotation)
{
    ESP_LOGI(TAG, "set rotation: %d", rotation);
    esp_err_t ret = st7735s_write_cmd(0x36);
    if (ret != ESP_OK) return ret;

    // ST7735S MADCTL设置 - 根据数据手册
    // bit7=MY, bit6=MX, bit5=MV, bit4=ML, bit3=RGB, bit2=MH
    uint8_t madctl = 0x08; // RGB=1 (bit3), 其他位为0

    switch(rotation) {
        case DISPLAY_ROTATION_0:
            // 0度：MY=0, MX=0, MV=0 (正常方向)
            madctl |= 0x00;
            break;
        case DISPLAY_ROTATION_90:
            // 90度：MY=0, MX=1, MV=1 (顺时针90度)
            madctl |= 0x60; // 01100000b
            break;
        case DISPLAY_ROTATION_180:
            // 180度：MY=1, MX=1, MV=0
            madctl |= 0xC0; // 11000000b
            break;
        case DISPLAY_ROTATION_270:
            // 270度：MY=1, MX=0, MV=1
            madctl |= 0xA0; // 10100000b
            break;
    }
    return st7735s_write_data(&madctl, 1);
}

esp_err_t st7735s_set_window(uint16_t xStart, uint16_t xEnd, uint16_t yStart, uint16_t yEnd)
{
    esp_err_t ret = st7735s_write_cmd(0x2A); // Column Address Set
    if (ret != ESP_OK) return ret;
    uint8_t data_col[4] = {xStart >> 8, xStart & 0xFF, xEnd >> 8, xEnd & 0xFF};
    ret = st7735s_write_data(data_col, 4);
    if (ret != ESP_OK) return ret;

    ret = st7735s_write_cmd(0x2B); // Row Address Set
    if (ret != ESP_OK) return ret;
    uint8_t data_row[4] = {yStart >> 8, yStart & 0xFF, yEnd >> 8, yEnd & 0xFF};
    ret = st7735s_write_data(data_row, 4);
    if (ret != ESP_OK) return ret;

    ret = st7735s_write_cmd(0x2C); // Memory Write
    return ret;
}

esp_err_t st7735s_draw_pixel(uint16_t x, uint16_t y, uint16_t color)
{
    esp_err_t ret = st7735s_set_window(x, x, y, y);
    if (ret != ESP_OK) return ret;
    return st7735s_write_data((uint8_t*)&color, 2);
}

esp_err_t st7735s_fill_screen(uint16_t color)
{
    // 创建一个足够大的缓冲区来填充屏幕
    // 由于ESP32C2 SPI限制，我们需要分块发送
    const size_t max_pixels_per_chunk = 2048; // 4096字节 / 2字节每像素
    uint8_t color_buffer[4096]; // 2048像素 * 2字节每像素

    // 填充颜色缓冲区
    for (size_t i = 0; i < max_pixels_per_chunk; i++) {
        color_buffer[i * 2] = color >> 8;
        color_buffer[i * 2 + 1] = color & 0xFF;
    }

    size_t total_pixels = TFT_WIDTH * TFT_HEIGHT;
    size_t pixels_sent = 0;

    // 设置整个屏幕窗口
    esp_err_t ret = st7735s_set_window(0, TFT_WIDTH - 1, 0, TFT_HEIGHT - 1);
    if (ret != ESP_OK) return ret;

    ret = st7735s_write_cmd(0x2C); // Memory Write
    if (ret != ESP_OK) return ret;

    gpio_set_level(ST7735S_PIN_NUM_DC, 1);

    // 分块发送数据
    while (pixels_sent < total_pixels) {
        size_t chunk_pixels = (total_pixels - pixels_sent > max_pixels_per_chunk) ?
                             max_pixels_per_chunk : (total_pixels - pixels_sent);

        spi_transaction_t t = {
            .length = chunk_pixels * 2 * 8, // 2字节每像素
            .tx_buffer = color_buffer
        };
        ret = spi_device_transmit(st7735s_spi, &t);
        if (ret != ESP_OK) return ret;

        pixels_sent += chunk_pixels;
    }

    return ESP_OK;
}

esp_err_t st7735s_write_pixels(const uint8_t *data, size_t len)
{
    gpio_set_level(ST7735S_PIN_NUM_DC, 1);
    spi_transaction_t t = {
        .length = len * 8,
        .tx_buffer = data
    };
    return spi_device_transmit(st7735s_spi, &t);
}

esp_err_t st7735s_flush_buffer(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint8_t *data)
{
    esp_err_t ret = st7735s_set_window(x, x + w - 1, y, y + h - 1);
    if (ret != ESP_OK) return ret;

    // ESP32C2 SPI硬件最大传输长度限制，需要分块发送
    const size_t max_chunk_size = 4092; // ESP32 SPI最大传输长度
    size_t total_bytes = w * h * 2;
    size_t bytes_sent = 0;

    gpio_set_level(ST7735S_PIN_NUM_DC, 1);

    while (bytes_sent < total_bytes) {
        size_t chunk_size = (total_bytes - bytes_sent > max_chunk_size) ? max_chunk_size : (total_bytes - bytes_sent);

        spi_transaction_t t = {
            .length = chunk_size * 8,
            .tx_buffer = data + bytes_sent
        };
        ret = spi_device_transmit(st7735s_spi, &t);
        if (ret != ESP_OK) return ret;

        bytes_sent += chunk_size;
    }

    return ESP_OK;
}

esp_err_t st7735s_draw_char(uint16_t x, uint16_t y, char c, uint16_t color, uint16_t bgcolor)
{
    uint8_t char_index;

    // 获取字符索引 (ASCII码)
    if (c >= ' ' && c <= '~') {
        char_index = c - ' ';
    } else {
        // 不支持的字符用空格代替
        char_index = 0;
    }

    // 临时设置为正常模式进行字符绘制 (MADCTL=0x00)
    esp_err_t ret = st7735s_write_cmd(0x36); // MADCTL
    uint8_t temp_madctl = 0x00; // 正常模式：RGB, 行/列正常顺序
    ret = st7735s_write_data(&temp_madctl, 1);
    if (ret != ESP_OK) return ret;

    // 设置字符绘制窗口 (6x12像素) - 使用原始坐标
    ret = st7735s_set_window(x, x + 5, y, y + 11);
    if (ret != ESP_OK) return ret;

    ret = st7735s_write_cmd(0x2C); // Memory Write
    if (ret != ESP_OK) return ret;

    // 绘制字符位图 (12行 x 6列)
    // 阳码格式：1=前景色，0=背景色
    // 逐行正向：从上到下，从左到右
    for (uint8_t row = 0; row < 12; row++) {
        uint8_t bitmap_byte = char_12x6[char_index][row];

        // 从左到右扫描列 (col 0-5)
        for (uint8_t col = 0; col < 6; col++) {
            uint16_t pixel_color;
            if (bitmap_byte & (1 << col)) {  // 阳码：1=前景色
                pixel_color = color;
            } else {  // 0=背景色
                pixel_color = bgcolor;
            }

            // 发送像素数据 (RGB565格式)
            uint8_t pixel_data[2] = {pixel_color >> 8, pixel_color & 0xFF};
            ret = st7735s_write_data(pixel_data, 2);
            if (ret != ESP_OK) return ret;
        }
    }

    // 恢复MADCTL设置 (0xC8)
    ret = st7735s_write_cmd(0x36); // MADCTL
    uint8_t restore_madctl = 0xC8; // 恢复默认旋转
    ret = st7735s_write_data(&restore_madctl, 1);

    return ret;
}