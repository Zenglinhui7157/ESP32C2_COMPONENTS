#include "st7789.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define TAG "ST7789"

static spi_device_handle_t st7789_spi;

static esp_err_t st7789_gpio_init(void)
{
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL << ST7789_PIN_NUM_DC) | (1ULL << ST7789_PIN_NUM_RST) | (1ULL << ST7789_PIN_NUM_BLK),
        .pull_down_en = 0,
        .pull_up_en = 0
    };
    esp_err_t ret = gpio_config(&io_conf);
    if (ret != ESP_OK) return ret;
    ret = gpio_set_level(ST7789_PIN_NUM_BLK, 1);
    return ret;
}

esp_err_t st7789_init(void)
{
    ESP_LOGI(TAG, "st7789_init called");
    esp_err_t ret = st7789_gpio_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "GPIO init failed: %d", ret);
        return ret;
    }

    spi_host_device_t spi_host;
    switch(CONFIG_ST7789_SPI_HOST) {
        case 0: spi_host = SPI1_HOST; break;
        case 1: spi_host = SPI2_HOST; break;
        default: spi_host = SPI2_HOST; break;
    }

    spi_bus_config_t buscfg = {
        .mosi_io_num = ST7789_PIN_NUM_MOSI,
        .miso_io_num = -1,
        .sclk_io_num = ST7789_PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = TFT_WIDTH * TFT_HEIGHT * 2 + 8
    };
    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = ST7789_SPI_CLOCK_HZ,
        .mode = ST7789_SPI_MODE,
        .spics_io_num = ST7789_PIN_NUM_CS,
        .queue_size = 7,
        .pre_cb = NULL
    };
    ret = spi_bus_initialize(spi_host, &buscfg, SPI_DMA_CH_AUTO);
    if(ret != ESP_OK)
    {
        ESP_LOGE(TAG, "spi_bus_initialize failed: %d", ret);
        return ret;
    }
    ret = spi_bus_add_device(spi_host, &devcfg, &st7789_spi);
    if(ret != ESP_OK)
    {
        ESP_LOGE(TAG, "spi_bus_add_device failed: %d", ret);
        return ret;
    }
    ret = st7789_reset();

    // ST7789初始化命令序列（参考Hardware/st7789.c）
    ret = st7789_write_cmd(0x11); // Sleep Out
    vTaskDelay(pdMS_TO_TICKS(120));

    ret = st7789_write_cmd(0xB2); // Frame rate control
    uint8_t porch_data[] = {0x0C, 0x0C, 0x00, 0x33, 0x33};
    ret = st7789_write_data(porch_data, sizeof(porch_data));

    ret = st7789_write_cmd(0x20); // Display inversion off

    ret = st7789_write_cmd(0xB7); // Gate control
    uint8_t gate = 0x56;
    ret = st7789_write_data(&gate, 1);

    ret = st7789_write_cmd(0xBB); // VCOM
    uint8_t vcom = 0x18;
    ret = st7789_write_data(&vcom, 1);

    ret = st7789_write_cmd(0xC0); // LCM control
    uint8_t lcm = 0x2C;
    ret = st7789_write_data(&lcm, 1);

    ret = st7789_write_cmd(0xC2); // VDV & VRH enable
    uint8_t vdv_vrh = 0x01;
    ret = st7789_write_data(&vdv_vrh, 1);

    ret = st7789_write_cmd(0xC3); // VRH set
    uint8_t vrh = 0x1F;
    ret = st7789_write_data(&vrh, 1);

    ret = st7789_write_cmd(0xC4); // VDV set
    uint8_t vdv = 0x20;
    ret = st7789_write_data(&vdv, 1);

    ret = st7789_write_cmd(0xC6); // FR Control 2
    uint8_t fr = 0x0F;
    ret = st7789_write_data(&fr, 1);

    ret = st7789_write_cmd(0xD0); // Power Control 1
    uint8_t power_data[] = {0xA6, 0xA1};
    ret = st7789_write_data(power_data, 2);

    // Gamma正
    uint8_t gamma_positive[] = {0xD0,0x0D,0x14,0x0B,0x0B,0x07,0x3A,0x44,0x50,0x08,0x13,0x13,0x2D,0x32};
    ret = st7789_write_cmd(0xE0);
    ret = st7789_write_data(gamma_positive, sizeof(gamma_positive));

    // Gamma负
    // uint8_t gamma_negative[] = {0xD0,0x04,0x0C,0x11,0x13,0x2C,0x3F,0x44,0x51,0x2F,0x1F,0x1F,0x20,0x23};
    uint8_t gamma_negative[] = {0xD0,0x0D,0x14,0x0B,0x0B,0x07,0x3A,0x44,0x50,0x08,0x13,0x13,0x2D,0x32};
    ret = st7789_write_cmd(0xE1);
    ret = st7789_write_data(gamma_negative, sizeof(gamma_negative));

    ret = st7789_write_cmd(0x36); // MADCTL
    uint8_t madctl = 0x60; // 默认旋转
    ret = st7789_write_data(&madctl, 1);

    ret = st7789_write_cmd(0x3A); // COLMOD
    uint8_t colmod = 0x55; // 16-bit RGB565
    ret = st7789_write_data(&colmod, 1);

    ret = st7789_write_cmd(0xE7); // SPI双向模式
    uint8_t spi_mode = 0x00;
    ret = st7789_write_data(&spi_mode, 1);

    ret = st7789_write_cmd(0x21); // Display Inversion On
    ret = st7789_write_cmd(0x29); // Display On

    ret = st7789_set_rotation(DISPLAY_ROTATION_0); // 默认旋转
    vTaskDelay(pdMS_TO_TICKS(100));

    ESP_LOGI(TAG, "st7789 init finished");
    return ESP_OK;
}

esp_err_t st7789_reset(void)
{
    ESP_LOGI(TAG, "st7789_reset called");
    esp_err_t ret = gpio_set_level(ST7789_PIN_NUM_RST, 0);
    if (ret != ESP_OK) return ret;
    vTaskDelay(pdMS_TO_TICKS(150));
    ret = gpio_set_level(ST7789_PIN_NUM_RST, 1);
    if (ret != ESP_OK) return ret;
    vTaskDelay(pdMS_TO_TICKS(150));
    return ESP_OK;
}

esp_err_t st7789_write_cmd(uint8_t cmd)
{
    gpio_set_level(ST7789_PIN_NUM_DC, 0);
    spi_transaction_t t = {
        .length = 8,
        .tx_buffer = &cmd
    };
    return spi_device_transmit(st7789_spi, &t);
}

esp_err_t st7789_write_data(const uint8_t *data, int len)
{
    gpio_set_level(ST7789_PIN_NUM_DC, 1);
    spi_transaction_t t = {
        .length = len * 8,
        .tx_buffer = data
    };
    return spi_device_transmit(st7789_spi, &t);
}

esp_err_t st7789_set_rotation(DisplayRotation rotation)
{
    ESP_LOGI(TAG, "set rotation: %d", rotation);
    // TODO: 发送 MADCTL 命令
    esp_err_t ret = st7789_write_cmd(0x36);
    if (ret != ESP_OK) return ret;
    uint8_t madctl = 0x00; // RGB color order, matching STM32
    switch(rotation) {
        case DISPLAY_ROTATION_0: madctl |= 0x00; break;
        case DISPLAY_ROTATION_90: madctl |= 0x20; break; // Match STM32
        case DISPLAY_ROTATION_180: madctl |= 0xC0; break;
        case DISPLAY_ROTATION_270: madctl |= 0x60; break; // Match STM32
    }
    return st7789_write_data(&madctl, 1);
}

esp_err_t st7789_set_window(uint16_t xStart, uint16_t xEnd, uint16_t yStart, uint16_t yEnd)
{
    // ESP_LOGI(TAG, "set window: x[%d-%d] y[%d-%d]", xStart, xEnd, yStart, yEnd);
    // TODO: 发送窗口设置命令
    esp_err_t ret = st7789_write_cmd(0x2A); // Column Address Set
    if (ret != ESP_OK) return ret;
    uint8_t data_col[4] = {xStart >> 8, xStart & 0xFF, xEnd >> 8, xEnd & 0xFF};
    ret = st7789_write_data(data_col, 4);
    if (ret != ESP_OK) return ret;
    ret = st7789_write_cmd(0x2B); // Row Address Set
    if (ret != ESP_OK) return ret;
    uint8_t data_row[4] = {yStart >> 8, yStart & 0xFF, yEnd >> 8, yEnd & 0xFF};
    ret = st7789_write_data(data_row, 4);
    if (ret != ESP_OK) return ret;
    // Memory Write 命令由 LVGL 刷新回调发送
    return ESP_OK;
}

esp_err_t st7789_draw_pixel(uint16_t x, uint16_t y, uint16_t color)
{
    // ESP_LOGI(TAG, "Draw pixel at (%d, %d) color: 0x%04X", x, y, color);
    // TODO: 设置窗口并写入像素数据
    esp_err_t ret = st7789_set_window(x, x, y, y);
    if (ret != ESP_OK) return ret;
    ret = st7789_write_cmd(0x2C); // Memory Write
    if (ret != ESP_OK) return ret;
    uint8_t data[2] = {color >> 8, color & 0xFF};
    return st7789_write_data(data, 2);
}

esp_err_t st7789_fill_screen(uint16_t color)
{
    ESP_LOGI(TAG, "Fill screen with color: 0x%04X", color);
    // TODO: 填充整个屏幕
    esp_err_t ret = st7789_set_window(0, TFT_WIDTH-1, 0, TFT_HEIGHT-1);
    if (ret != ESP_OK) return ret;
    ret = st7789_write_cmd(0x2C); // Memory Write
    if (ret != ESP_OK) return ret;
    // 批量填充像素数据
    size_t size = TFT_WIDTH * TFT_HEIGHT;
    // 减少批量大小以避免堆栈溢出
    size_t max_pixels_per_batch = 512;
    uint8_t pixel_data[max_pixels_per_batch * 2];
    for (size_t i = 0; i < max_pixels_per_batch; ++i) {
        pixel_data[i * 2] = color >> 8;
        pixel_data[i * 2 + 1] = color & 0xFF;
    }
    size_t pixels_sent = 0;
    while (pixels_sent < size) {
        size_t batch = (size - pixels_sent) > max_pixels_per_batch ? max_pixels_per_batch : (size - pixels_sent);
        ret = st7789_write_data(pixel_data, batch * 2);
        if (ret != ESP_OK) return ret;
        pixels_sent += batch;
    }
    return ESP_OK;
}

esp_err_t st7789_write_pixels(const uint8_t *data, size_t len)
{
    // 使用DMA传输像素数据，假设窗口已设置且Memory Write命令已发送
    gpio_set_level(ST7789_PIN_NUM_DC, 1);
    spi_transaction_t t = {
        .length = len * 8,
        .tx_buffer = data
    };
    return spi_device_transmit(st7789_spi, &t);
}

esp_err_t st7789_flush_buffer(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint8_t *data)
{
    // 设置窗口并使用DMA刷新缓冲区
    esp_err_t ret = st7789_set_window(x, x + w - 1, y, y + h - 1);
    if (ret != ESP_OK) return ret;
    ret = st7789_write_cmd(0x2C); // Memory Write
    if (ret != ESP_OK) return ret;
    return st7789_write_pixels(data, w * h * 2);
}
