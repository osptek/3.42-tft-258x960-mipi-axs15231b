/*
 * SPDX-FileCopyrightText: Copyright 2026 OSPTEK
 * SPDX-License-Identifier: CC-BY-4.0
 *
 * https://github.com/osptek
 */

#include "esp_err.h"
#include "esp_log.h"
#include "esp_check.h"
#include "driver/i2c_master.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_mipi_dsi.h"
#include "driver/gpio.h"
#include "esp_ldo_regulator.h"
#include "esp_lvgl_port.h"
#include "lv_demos.h"

#include "esp_lcd_axs15231b.h"

/* LCD size */
#define EXAMPLE_LCD_H_RES   (258)
#define EXAMPLE_LCD_V_RES   (960)

#if LV_COLOR_DEPTH == 16
#define MIPI_DPI_PX_FORMAT (LCD_COLOR_PIXEL_FORMAT_RGB565)
#define BSP_LCD_COLOR_DEPTH (16)
#define LV_COLOR_FORMAT LV_COLOR_FORMAT_RGB565
#elif LV_COLOR_DEPTH == 24
#define MIPI_DPI_PX_FORMAT (LCD_COLOR_PIXEL_FORMAT_RGB888)
#define BSP_LCD_COLOR_DEPTH (24)
#define LV_COLOR_FORMAT LV_COLOR_FORMAT_RGB888
#endif

// “VDD_MIPI_DPHY”应供电 2.5V，可从内部 LDO 稳压器或外部 LDO 芯片获取电源
#define EXAMPLE_MIPI_DSI_PHY_PWR_LDO_CHAN 3 // LDO_VO3 连接至 VDD_MIPI_DPHY
#define EXAMPLE_MIPI_DSI_PHY_PWR_LDO_VOLTAGE_MV 2500
#define EXAMPLE_LCD_BK_LIGHT_ON_LEVEL 1
#define EXAMPLE_LCD_BK_LIGHT_OFF_LEVEL !EXAMPLE_LCD_BK_LIGHT_ON_LEVEL
#define EXAMPLE_PIN_NUM_BK_LIGHT -1
#define EXAMPLE_PIN_NUM_LCD_RST  -1

/* Touch settings */
#define EXAMPLE_TOUCH_I2C_NUM       (0)
#define EXAMPLE_TOUCH_I2C_CLK_HZ    (400000)

/* LCD touch pins */
#define EXAMPLE_TOUCH_I2C_SCL       (GPIO_NUM_8)
#define EXAMPLE_TOUCH_I2C_SDA       (GPIO_NUM_7)

static const char *TAG = "EXAMPLE";

/* LCD IO and panel */
static esp_lcd_panel_handle_t lcd_panel = NULL;
static esp_lcd_panel_io_handle_t io_handle = NULL;
static esp_lcd_touch_handle_t touch_handle = NULL;

/* LVGL display and touch */
static lv_display_t *lvgl_disp = NULL;
static lv_indev_t *lvgl_touch_indev = NULL;

static void example_bsp_enable_dsi_phy_power(void)
{
    // 打开 MIPI DSI PHY 的电源，使其从“无电”状态进入“开机”状态
    esp_ldo_channel_handle_t ldo_mipi_phy = NULL;
#ifdef EXAMPLE_MIPI_DSI_PHY_PWR_LDO_CHAN
    esp_ldo_channel_config_t ldo_mipi_phy_config = {
        .chan_id = EXAMPLE_MIPI_DSI_PHY_PWR_LDO_CHAN,
        .voltage_mv = EXAMPLE_MIPI_DSI_PHY_PWR_LDO_VOLTAGE_MV,
    };
    ESP_ERROR_CHECK(esp_ldo_acquire_channel(&ldo_mipi_phy_config, &ldo_mipi_phy));
    ESP_LOGI(TAG, "MIPI DSI PHY Powered on");
#endif
}

static void example_bsp_init_lcd_backlight(void)
{
#if EXAMPLE_PIN_NUM_BK_LIGHT >= 0
    gpio_config_t bk_gpio_config = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = 1ULL << EXAMPLE_PIN_NUM_BK_LIGHT};
    ESP_ERROR_CHECK(gpio_config(&bk_gpio_config));
#endif
}

static void example_bsp_set_lcd_backlight(uint32_t level)
{
#if EXAMPLE_PIN_NUM_BK_LIGHT >= 0
    gpio_set_level(EXAMPLE_PIN_NUM_BK_LIGHT, level);
#endif
}

static const axs15231b_lcd_init_cmd_t lcd_init_cmds[] = {
// {cmd, { data }, data_size, delay_ms}
    {0xBB, (uint8_t []){0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 8, 0},
    {0xBB, (uint8_t []){0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x5A, 0xA5}, 8, 0},
    {0xA0, (uint8_t []){0x00, 0x30, 0x00, 0x02, 0x00, 0x00, 0x04, 0x3F, 0x20, 0x04,
                        0x3F, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x00}, 17, 0},
    {0xA2, (uint8_t []){0x20, 0x19, 0x0A, 0x64, 0x3C, 0x60, 0x32, 0xC0, 0x02, 0x39, 
                        0x7F, 0x7F, 0x7F, 0x20, 0xF8, 0x10, 0x02, 0xFF, 0xFF, 0xF0,
                        0x90, 0x01, 0x32, 0xA0, 0x91, 0xC0, 0x20, 0x7F, 0xFF, 0x00,
                        0x04}, 31, 0},
    {0xD0, (uint8_t []){0xC0, 0x02, 0x72, 0x24, 0x08, 0x05, 0x10, 0x10, 0x70, 0x11, 
                        0xC2, 0x40, 0x20, 0x02, 0xAA, 0x03, 0x10, 0x12, 0x60, 0x14,
                        0x1E, 0x51, 0x15, 0x00, 0xB4, 0x00, 0x00, 0x03, 0x0d, 0x12}, 30, 0},
    {0xA3, (uint8_t []){0xA0, 0x06, 0xAA, 0x28, 0x08, 0x02, 0x0A, 0x04, 0x04, 0x04,
                        0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x00,
                        0x55, 0x55}, 22, 0},
    {0xC1, (uint8_t []){0x31, 0x04, 0x02, 0x02, 0x71, 0x05, 0x24, 0x55, 0x02, 0x00, 
                        0x41, 0x01, 0x53, 0xFF, 0xFF, 0xFF, 0x4F, 0x52, 0x00, 0x4F,
                        0x52, 0x00, 0x45, 0x3B, 0x0B, 0x02, 0x0D, 0x00, 0xFF, 0x40}, 30, 0},
    {0xC3, (uint8_t []){0x00, 0x00, 0x00, 0x50, 0x03, 0x00, 0x00, 0x00, 0x01, 0x80,
                        0x01}, 11, 0},
    {0xC4, (uint8_t []){0x00, 0x24, 0x33, 0x90, 0x7a, 0xea, 0x64, 0x32, 0xC8, 0x32, 
                        0x32, 0x32, 0x90, 0x32, 0x10, 0x06, 0xDC, 0xFA, 0x14, 0x03,
                        0x80, 0xFE, 0x10, 0x40, 0x00, 0x0A, 0x02, 0x44, 0x50}, 29, 0},
    {0xC5, (uint8_t []){0x18, 0x00, 0x00, 0x03, 0xFE, 0x60, 0x65, 0x40, 0x10, 0x10,
                        0x88, 0xDE, 0x0D, 0x08, 0x0F, 0x0F, 0x01, 0x18, 0x38, 0x40,
                        0x10, 0x10, 0x00}, 23, 0},
    {0xC6, (uint8_t []){0x05, 0x0A, 0x05, 0x0A, 0x00, 0xE0, 0x2E, 0x0B, 0x12, 0x22,
                        0x12, 0x22, 0x01, 0x00, 0x00, 0x3F, 0x6A, 0x18, 0xC8, 0x22}, 20, 0},
    {0xC7, (uint8_t []){0x50, 0x30, 0x28, 0x00, 0xa2, 0x80, 0x8f, 0x00, 0x80, 0x00,
                        0x00, 0x11, 0x9F, 0x6f, 0xff, 0x22, 0x01, 0x01, 0xB9, 0x0f}, 20, 0},
    {0xC9, (uint8_t []){0x33, 0x44, 0x44, 0x01}, 4, 0},
    {0xCF, (uint8_t []){0x34, 0x1E, 0x88, 0x58, 0x13, 0x18, 0x56, 0x18, 0x1E, 0x68,
                        0xf4, 0x00, 0x68, 0x0f, 0x66, 0xe6, 0x0C, 0x77, 0x22, 0x44,
                        0xAA, 0x55, 0x04, 0x04, 0x16, 0xA0, 0x08}, 27, 0},
    {0xD5, (uint8_t []){0x19, 0x20, 0x84, 0x80, 0x35, 0x03, 0xC2, 0xB5, 0x08, 0xC4,
                        0xC4, 0x88, 0x04, 0x28, 0x53, 0x42, 0x26, 0x19, 0x03, 0x03,
                        0x86, 0x00, 0x00, 0x00, 0xC0, 0x53, 0xFB, 0x19, 0x29, 0x00}, 30, 0},
    {0xD6, (uint8_t []){0x10, 0x32, 0x54, 0x76, 0x98, 0xBA, 0xDC, 0xFE, 0x95, 0x00,
                        0x01, 0x01, 0xC5, 0xC5, 0x88, 0x75, 0x36, 0x20, 0x03, 0x03,
                        0x03, 0x03, 0x10, 0x10, 0x00, 0x83, 0x51, 0x22, 0x01, 0x00}, 30, 0},
    {0xD7, (uint8_t []){0x00, 0x18, 0x1F, 0x08, 0x0A, 0x0C, 0x0E, 0x06, 0x1F, 0x1A,
                        0x19, 0x1F, 0x15, 0x20, 0x04, 0x00, 0x15, 0x2D, 0x1F}, 19, 0},
    {0xD8, (uint8_t []){0x01, 0x18, 0x1F, 0x09, 0x0B, 0x0D, 0x0F, 0x07, 0x1F, 0x1A,
                        0x19,0x1F}, 12, 0},
    {0xD9, (uint8_t []){0x05, 0x1F, 0x18, 0x08, 0x0A, 0x0C, 0x0E, 0x01, 0x1F, 0x1A,
                        0x19,0x1F}, 12, 0},
    {0xDD, (uint8_t []){0x04, 0x1F, 0x18, 0x09, 0x0B, 0x0D, 0x0F, 0x00, 0x1F, 0x1A,
                        0x19,0x1F}, 12, 0},
    {0xDF, (uint8_t []){0x44, 0x73, 0x4B, 0x69, 0x00, 0x0A, 0x02, 0x90}, 8, 0},
    {0xE0, (uint8_t []){0x24, 0x0c, 0x0d, 0x12, 0x0a, 0x08, 0x11, 0x23, 0x44, 0x1e, 
                        0x0f, 0x1a, 0x18, 0x29, 0x2c, 0x0D, 0x09}, 17, 0},
    {0xE1, (uint8_t []){0x24, 0x0c, 0x0d, 0x12, 0x0a, 0x08, 0x11, 0x23, 0x44, 0x1E,
                        0x0f, 0x1a, 0x18, 0x29, 0x2c, 0x0D, 0x0F}, 17, 0},
    {0xE2, (uint8_t []){0x19, 0x20, 0x0A, 0x13, 0x13, 0x06, 0x11, 0x25, 0xD4, 0x22,
                        0x0B, 0x13, 0x12, 0x2D, 0x32, 0x2f, 0x03}, 17, 0},
    {0xE3, (uint8_t []){0x38, 0x20, 0x0A, 0x13, 0x13, 0x06, 0x11, 0x25, 0xC4, 0x21,
                        0x0A, 0x12, 0x11, 0x2C, 0x32, 0x2f, 0x27}, 17, 0},
    {0xE4, (uint8_t []){0x19, 0x20, 0x0D, 0x14, 0x0D, 0x08, 0x12, 0x2A, 0xD4, 0x26,
                        0x0E, 0x15, 0x13, 0x34, 0x39, 0x2f, 0x03}, 17, 0},
    {0xE5, (uint8_t []){0x38, 0x20, 0x0D, 0x13, 0x0D, 0x07, 0x12, 0x29, 0xC4, 0x25,
                        0x0D, 0x15, 0x12, 0x33, 0x39, 0x2f, 0x27}, 17, 0},
    {0xA4, (uint8_t []){0x85, 0x85, 0x95, 0x82, 0xAF, 0xAA, 0xAD, 0x80, 0x10, 0x30,
                        0x40, 0x40, 0x20, 0xFF, 0x60, 0x30}, 16, 0},
    {0xA4, (uint8_t []){0x85, 0x85, 0x95, 0x85}, 4, 0},
    {0xBB, (uint8_t []){0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 8, 0},
    {0x11, (uint8_t []){0x00}, 0, 100}, // Sleep Out
    {0x29, (uint8_t []){0x00}, 0, 100}, // Display On
};

static esp_err_t app_lcd_init(void)
{
    esp_err_t ret = ESP_OK;

    example_bsp_enable_dsi_phy_power();
    example_bsp_init_lcd_backlight();
    example_bsp_set_lcd_backlight(EXAMPLE_LCD_BK_LIGHT_OFF_LEVEL);

    // 首先创建 MIPI DSI 总线，它还将初始化 DSI PHY
    esp_lcd_dsi_bus_handle_t mipi_dsi_bus;
    esp_lcd_dsi_bus_config_t bus_config = {                    \
        .bus_id = 0,                                           \
        .num_data_lanes = 1,                                   \
        .lane_bit_rate_mbps = 650,                             \
    };
    ESP_GOTO_ON_ERROR(esp_lcd_new_dsi_bus(&bus_config, &mipi_dsi_bus), err, TAG, "LCD init failed");

    ESP_LOGI(TAG, "Install MIPI DSI LCD control panel");
    // 我们使用DBI接口发送LCD命令和参数
    esp_lcd_dbi_io_config_t dbi_config = {   \
        .virtual_channel = 0,                \
        .lcd_cmd_bits = 8,                   \
        .lcd_param_bits = 8,                 \
    };

    ESP_GOTO_ON_ERROR(esp_lcd_new_panel_io_dbi(mipi_dsi_bus, &dbi_config, &io_handle), err, TAG, "LCD init failed");

    // 创建AXS15231B控制面板
    esp_lcd_dpi_panel_config_t dpi_config = {                 \
        .dpi_clk_src = MIPI_DSI_DPI_CLK_SRC_DEFAULT,          \
        .dpi_clock_freq_mhz = 27,                             \
        .virtual_channel = 0,                                 \
        .pixel_format = MIPI_DPI_PX_FORMAT,                   \
        .num_fbs = 1,                                         \
        .video_timing = {                                     \
            .h_size = EXAMPLE_LCD_H_RES,                      \
            .v_size = EXAMPLE_LCD_V_RES,                      \
            .hsync_back_porch = 60,                           \
            .hsync_pulse_width = 10,                          \
            .hsync_front_porch = 50,                          \
            .vsync_back_porch = 100,                          \
            .vsync_pulse_width = 25,                          \
            .vsync_front_porch = 120,                         \
        },                                                    \
        .flags.use_dma2d = true,                              \
    };

    axs15231b_vendor_config_t vendor_config = {
        .init_cmds = lcd_init_cmds,      // Uncomment these line if use custom initialization commands
        .init_cmds_size = sizeof(lcd_init_cmds) / sizeof(axs15231b_lcd_init_cmd_t),
        .flags.use_mipi_interface = 1,
        .mipi_config = {
            .dsi_bus = mipi_dsi_bus,
            .dpi_config = &dpi_config,
        },
    };
    const esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = EXAMPLE_PIN_NUM_LCD_RST,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
        .bits_per_pixel = BSP_LCD_COLOR_DEPTH,
        .vendor_config = &vendor_config,
    };
    ESP_GOTO_ON_ERROR(esp_lcd_new_panel_axs15231b(io_handle, &panel_config, &lcd_panel), err, TAG, "LCD init failed");
    ESP_GOTO_ON_ERROR(esp_lcd_panel_reset(lcd_panel), err, TAG, "LCD init failed");
    ESP_GOTO_ON_ERROR(esp_lcd_panel_init(lcd_panel), err, TAG, "LCD init failed");
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(lcd_panel, true));

    // 打开背光
    example_bsp_set_lcd_backlight(EXAMPLE_LCD_BK_LIGHT_ON_LEVEL);

    return ret;

err:
    if (lcd_panel) {
        esp_lcd_panel_del(lcd_panel);
    }
    return ret;
}

static esp_err_t app_touch_init(void)
{
    /* Initilize I2C */
    const i2c_master_bus_config_t i2c_bus_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = EXAMPLE_TOUCH_I2C_NUM,
        .sda_io_num = EXAMPLE_TOUCH_I2C_SDA,
        .scl_io_num = EXAMPLE_TOUCH_I2C_SCL,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    i2c_master_bus_handle_t i2c_bus;
    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_bus_config, &i2c_bus));
    
    esp_lcd_panel_io_handle_t tp_io_handle = NULL;
    esp_lcd_panel_io_i2c_config_t tp_io_config = ESP_LCD_TOUCH_IO_I2C_AXS15231B_CONFIG();
    tp_io_config.scl_speed_hz = EXAMPLE_TOUCH_I2C_CLK_HZ;

    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c(i2c_bus, &tp_io_config, &tp_io_handle));

    /* Initialize touch HW */
    const esp_lcd_touch_config_t tp_cfg = {
        .x_max = EXAMPLE_LCD_H_RES,
        .y_max = EXAMPLE_LCD_V_RES,
        .rst_gpio_num = GPIO_NUM_NC,
        .int_gpio_num = GPIO_NUM_NC,
        .levels = {
            .reset = 0,
            .interrupt = 0,
        },
        .flags = {
            .swap_xy = 0,
            .mirror_x = 0,
            .mirror_y = 0,
        },
    };
    
    return esp_lcd_touch_new_i2c_axs15231b(tp_io_handle, &tp_cfg, &touch_handle);
}

static esp_err_t app_lvgl_init(void)
{
    /* Initialize LVGL */
    const lvgl_port_cfg_t lvgl_cfg = ESP_LVGL_PORT_INIT_CONFIG();
    ESP_RETURN_ON_ERROR(lvgl_port_init(&lvgl_cfg), TAG, "LVGL port initialization failed");

    /* Add LCD screen */
    ESP_LOGD(TAG, "Add LCD screen");
    const lvgl_port_display_cfg_t disp_cfg = {
        .io_handle = io_handle,
        .panel_handle = lcd_panel,
        .buffer_size = EXAMPLE_LCD_H_RES * EXAMPLE_LCD_V_RES,
        .double_buffer = true,
        .hres = EXAMPLE_LCD_H_RES,
        .vres = EXAMPLE_LCD_V_RES,
        .monochrome = false,
        .color_format = LV_COLOR_FORMAT,
        .rotation = {
            .swap_xy = false,
            .mirror_x = false,
            .mirror_y = false,
        },
        .flags = {
            .buff_dma = false,
            .buff_spiram = true,
            .sw_rotate = true,
            .swap_bytes = false,
            .full_refresh = false,
            .direct_mode = false,
        }
    };

    const lvgl_port_display_dsi_cfg_t dpi_cfg = {
        .flags = {
#if CONFIG_BSP_DISPLAY_LVGL_AVOID_TEAR
            .avoid_tearing = true,
#else
            .avoid_tearing = false,
#endif
        }
    };

    lvgl_disp = lvgl_port_add_disp_dsi(&disp_cfg, &dpi_cfg);

    /* Add touch input (for selected screen) */
    const lvgl_port_touch_cfg_t touch_cfg = {
        .disp = lvgl_disp,
        .handle = touch_handle,
    };
    lvgl_touch_indev = lvgl_port_add_touch(&touch_cfg);

    return ESP_OK;
}

void app_main(void)
{
    /* LCD HW initialization */
    ESP_ERROR_CHECK(app_lcd_init());

    /* Touch initialization */
    ESP_ERROR_CHECK(app_touch_init());

    /* LVGL initialization */
    ESP_ERROR_CHECK(app_lvgl_init());

    /* Show LVGL objects */
    lvgl_port_lock(0);

    // lv_demo_music();
    lv_demo_widgets();

    lvgl_port_unlock();
}
