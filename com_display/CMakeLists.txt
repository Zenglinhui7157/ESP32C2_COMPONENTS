idf_component_register(
        SRCS
            com_display.c
        INCLUDE_DIRS "." "FONTS" "LVGL"
            REQUIRES lvgl driver
)

if(CONFIG_DISPLAY_TYPE_ST7789)
    target_sources(${COMPONENT_LIB} PRIVATE ST7789/st7789.c)
    target_include_directories(${COMPONENT_LIB} PRIVATE ST7789)
endif()

if(CONFIG_DISPLAY_TYPE_SSD1306)
    target_sources(${COMPONENT_LIB} PRIVATE SSD1306/ssd1306.c)
    target_include_directories(${COMPONENT_LIB} PRIVATE SSD1306)
endif()
