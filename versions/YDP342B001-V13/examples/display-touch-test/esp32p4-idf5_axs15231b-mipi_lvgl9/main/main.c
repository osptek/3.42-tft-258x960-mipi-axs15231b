/*
 * SPDX-FileCopyrightText: Copyright 2026 OSPTEK
 * SPDX-License-Identifier: CC-BY-4.0
 *
 * https://github.com/osptek
 */

#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_timer.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_mipi_dsi.h"
#include "esp_lcd_axs15231b.h"
#include "esp_ldo_regulator.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_log.h"
#include "lvgl.h"

static const char *TAG = "example";

#define EXAMPLE_MIPI_DSI_LCD_H_RES 258 // 宽
#define EXAMPLE_MIPI_DSI_LCD_V_RES 960  // 高

#if LV_COLOR_DEPTH == 16
#define MIPI_DPI_PX_FORMAT (LCD_COLOR_PIXEL_FORMAT_RGB565)
#define BSP_LCD_COLOR_DEPTH (16)
#define LV_COLOR_FORMAT (LV_COLOR_FORMAT_RGB565)
#elif LV_COLOR_DEPTH >= 24
#define MIPI_DPI_PX_FORMAT (LCD_COLOR_PIXEL_FORMAT_RGB888)
#define BSP_LCD_COLOR_DEPTH (24)
#define LV_COLOR_FORMAT (LV_COLOR_FORMAT_RGB888)
#endif

// “VDD_MIPI_DPHY”应供电 2.5V，可从内部 LDO 稳压器或外部 LDO 芯片获取电源
#define EXAMPLE_MIPI_DSI_PHY_PWR_LDO_CHAN 3 // LDO_VO3 连接至 VDD_MIPI_DPHY
#define EXAMPLE_MIPI_DSI_PHY_PWR_LDO_VOLTAGE_MV 2500
#define EXAMPLE_LCD_BK_LIGHT_ON_LEVEL 1
#define EXAMPLE_LCD_BK_LIGHT_OFF_LEVEL !EXAMPLE_LCD_BK_LIGHT_ON_LEVEL
#define EXAMPLE_PIN_NUM_BK_LIGHT -1 //背光
#define EXAMPLE_PIN_NUM_LCD_RST  -1

#define EXAMPLE_LVGL_DRAW_BUF_LINES 200 // 每个绘制缓冲区中的显示线数
#define EXAMPLE_LVGL_TICK_PERIOD_MS 2
#define EXAMPLE_LVGL_TASK_MAX_DELAY_MS 500
#define EXAMPLE_LVGL_TASK_MIN_DELAY_MS 1
#define EXAMPLE_LVGL_TASK_STACK_SIZE (4 * 1024)
#define EXAMPLE_LVGL_TASK_PRIORITY 2

static SemaphoreHandle_t lvgl_api_mux = NULL;

extern void example_lvgl_demo_ui(lv_display_t *disp);

static void example_lvgl_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    esp_lcd_panel_handle_t panel_handle = lv_display_get_user_data(disp);
    int offsetx1 = area->x1;
    int offsetx2 = area->x2;
    int offsety1 = area->y1;
    int offsety2 = area->y2;
    // 将绘制缓冲区传递给驱动程序
    esp_lcd_panel_draw_bitmap(panel_handle, offsetx1, offsety1, offsetx2 + 1, offsety2 + 1, px_map);
}

static void example_increase_lvgl_tick(void *arg)
{
    /* 告诉LVGL已经过去了多少毫秒 */
    lv_tick_inc(EXAMPLE_LVGL_TICK_PERIOD_MS);
}

static bool example_lvgl_lock(int timeout_ms)
{
    // 将超时时间（以毫秒为单位）转换为 FreeRTOS时钟周期
    // 如果将“timeout_ms”设置为 -1，程序将阻塞，直到满足条件
    const TickType_t timeout_ticks = (timeout_ms == -1) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);
    return xSemaphoreTakeRecursive(lvgl_api_mux, timeout_ticks) == pdTRUE;
}

static void example_lvgl_unlock(void)
{
    xSemaphoreGiveRecursive(lvgl_api_mux);
}

static void example_lvgl_port_task(void *arg)
{
    ESP_LOGI(TAG, "Starting LVGL task");
    uint32_t task_delay_ms = EXAMPLE_LVGL_TASK_MAX_DELAY_MS;
    while (1)
    {
        // 由于 LVGL API 不是线程安全的，因此锁定互斥锁
        if (example_lvgl_lock(-1))
        {
            task_delay_ms = lv_timer_handler();
            // 释放互斥锁
            example_lvgl_unlock();
        }
        if (task_delay_ms > EXAMPLE_LVGL_TASK_MAX_DELAY_MS)
        {
            task_delay_ms = EXAMPLE_LVGL_TASK_MAX_DELAY_MS;
        }
        else if (task_delay_ms < EXAMPLE_LVGL_TASK_MIN_DELAY_MS)
        {
            task_delay_ms = EXAMPLE_LVGL_TASK_MIN_DELAY_MS;
        }
        vTaskDelay(pdMS_TO_TICKS(task_delay_ms));
    }
}

static bool example_notify_lvgl_flush_ready(esp_lcd_panel_handle_t panel, esp_lcd_dpi_panel_event_data_t *edata, void *user_ctx)
{
    lv_display_t *disp = (lv_display_t *)user_ctx;
    lv_display_flush_ready(disp);
    return false;
}

static void example_bsp_enable_dsi_phy_power(void)
{
    // 打开 MIPI DSI PHY 的电源，使其从“无电”状态进入“关机”状态
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
    {0xBB, (uint8_t []){0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x5A, 0xA5}, 8, 0},
    {0xA0, (uint8_t []){0x00, 0x30, 0x00, 0x02, 0x00, 0x00, 0x04, 0x3F, 0x20, 0x04, 0x3F, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x00}, 17, 0},
    {0xA2, (uint8_t []){0x20, 0x19, 0x0A, 0x64, 0x3C, 0x60, 0x32, 0xC0, 0x02, 0x39, 0x7F, 0x7F, 0x7F, 0x20, 0xF8, 0x10, 0x02, 0xFF, 0xFF, 0xF0, 0x90, 0x01, 0x32, 0xA0, 0x91, 0xC0, 0x20, 0x7F, 0xFF, 0x00, 0x04}, 31, 0},
    {0xD0, (uint8_t []){0xC0, 0x02, 0x72, 0x24, 0x08, 0x05, 0x10, 0x10, 0x70, 0x11, 0xC2, 0x40, 0x20, 0x02, 0xAA, 0x03, 0x10, 0x12, 0x60, 0x14, 0x1E, 0x51, 0x15, 0x00, 0xB4, 0x00, 0x00, 0x03, 0x0D, 0x12}, 30, 0},
    {0xA3, (uint8_t []){0xA0, 0x06, 0xAA, 0x28, 0x08, 0x02, 0x0A, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x00, 0x55, 0x55}, 22, 0},
    {0xC1, (uint8_t []){0x31, 0x04, 0x02, 0x02, 0x71, 0x05, 0x24, 0x55, 0x02, 0x00, 0x41, 0x01, 0x53, 0xFF, 0xFF, 0xFF, 0x4F, 0x52, 0x00, 0x4F, 0x52, 0x00, 0x45, 0x3B, 0x0B, 0x02, 0x0D, 0x00, 0xFF, 0x40}, 30, 0},
    {0xC3, (uint8_t []){0x00, 0x00, 0x00, 0x50, 0x03, 0x00, 0x00, 0x00, 0x01, 0x80, 0x01}, 11, 0},
    {0xC4, (uint8_t []){0x00, 0x24, 0x33, 0x90, 0x7A, 0xEA, 0x64, 0x32, 0xC8, 0x32, 0x32, 0x32, 0x90, 0x32, 0x10, 0x06, 0xDC, 0xFA, 0x14, 0x03, 0x80, 0xFE, 0x10, 0x40, 0x00, 0x0A, 0x02, 0x44, 0x50}, 29, 0},
    {0xC5, (uint8_t []){0x18, 0x00, 0x00, 0x03, 0xFE, 0x60, 0x65, 0x40, 0x10, 0x10, 0x88, 0xDE, 0x0D, 0x08, 0x0F, 0x0F, 0x01, 0x18, 0x38, 0x40, 0x10, 0x10, 0x00}, 23, 0},
    {0xC6, (uint8_t []){0x05, 0x0A, 0x05, 0x0A, 0x00, 0xE0, 0x2E, 0x0B, 0x12, 0x22, 0x12, 0x22, 0x01, 0x00, 0x00, 0x3F, 0x6A, 0x18, 0xC8, 0x22}, 20, 0},
    {0xC7, (uint8_t []){0x50, 0x30, 0x28, 0x00, 0xA2, 0x80, 0x8F, 0x00, 0x80, 0x00, 0x00, 0x11, 0x9F, 0x6F, 0xFF, 0x22, 0x01, 0x01, 0xB9, 0x0F}, 20, 0},
    {0xC9, (uint8_t []){0x33, 0x44, 0x44, 0x01}, 4, 0},
    {0xCF, (uint8_t []){0x34, 0x1E, 0x88, 0x58, 0x13, 0x18, 0x56, 0x18, 0x1E, 0x68, 0xF4, 0x00, 0x68, 0x0F, 0x66, 0xE6, 0x0C, 0x77, 0x22, 0x44, 0xAA, 0x55, 0x04, 0x04, 0x16, 0xA0, 0x08}, 27, 0},
    {0xD5, (uint8_t []){0x19, 0x20, 0x84, 0x80, 0x35, 0x03, 0xC2, 0xB5, 0x08, 0xC4, 0xC4, 0x88, 0x04, 0x28, 0x53, 0x42, 0x26, 0x19, 0x03, 0x03, 0x86, 0x00, 0x00, 0x00, 0xC0, 0x53, 0xFB, 0x19, 0x29, 0x00}, 30, 0},
    {0xD6, (uint8_t []){0x10, 0x32, 0x54, 0x76, 0x98, 0xBA, 0xDC, 0xFE, 0x95, 0x00, 0x01, 0x01, 0xC5, 0xC5, 0x88, 0x75, 0x36, 0x20, 0x03, 0x03, 0x03, 0x03, 0x10, 0x10, 0x00, 0x83, 0x51, 0x22, 0x01, 0x00}, 30, 0},
    {0xD7, (uint8_t []){0x00, 0x18, 0x1F, 0x08, 0x0A, 0x0C, 0x0E, 0x06, 0x1F, 0x1A, 0x19, 0x1F, 0x15, 0x20, 0x04, 0x00, 0x15, 0x2D, 0x1F}, 19, 0},
    {0xD8, (uint8_t []){0x01, 0x18, 0x1F, 0x09, 0x0B, 0x0D, 0x0F, 0x07, 0x1F, 0x1A, 0x19, 0x1F}, 12, 0},
    {0xD9, (uint8_t []){0x05, 0x1F, 0x18, 0x08, 0x0A, 0x0C, 0x0E, 0x01, 0x1F, 0x1A, 0x19, 0x1F}, 12, 0},
    {0xDD, (uint8_t []){0x04, 0x1F, 0x18, 0x09, 0x0B, 0x0D, 0x0F, 0x00, 0x1F, 0x1A, 0x19, 0x1F}, 12, 0},
    {0xDF, (uint8_t []){0x44, 0x73, 0x4B, 0x69, 0x00, 0x0A, 0x02, 0x90}, 8, 0},
    {0xE0, (uint8_t []){0x24, 0x0C, 0x0D, 0x12, 0x0A, 0x08, 0x11, 0x23, 0x44, 0x1E, 0x0F, 0x1A, 0x18, 0x29, 0x2C, 0x0D, 0x09}, 17, 0},
    {0xE1, (uint8_t []){0x24, 0x0C, 0x0D, 0x12, 0x0A, 0x08, 0x11, 0x23, 0x44, 0x1E, 0x0F, 0x1A, 0x18, 0x29, 0x2C, 0x0D, 0x0F}, 17, 0},
    {0xE2, (uint8_t []){0x19, 0x20, 0x0A, 0x13, 0x13, 0x06, 0x11, 0x25, 0xD4, 0x22, 0x0B, 0x13, 0x12, 0x2D, 0x32, 0x2F, 0x03}, 17, 0},
    {0xE3, (uint8_t []){0x38, 0x20, 0x0A, 0x13, 0x13, 0x06, 0x11, 0x25, 0xC4, 0x21, 0x0A, 0x12, 0x11, 0x2C, 0x32, 0x2F, 0x27}, 17, 0},
    {0xE4, (uint8_t []){0x19, 0x20, 0x0D, 0x14, 0x0D, 0x08, 0x12, 0x2A, 0xD4, 0x26, 0x0E, 0x15, 0x13, 0x34, 0x39, 0x2F, 0x03}, 17, 0},
    {0xE5, (uint8_t []){0x38, 0x20, 0x0D, 0x13, 0x0D, 0x07, 0x12, 0x29, 0xC4, 0x25, 0x0D, 0x15, 0x12, 0x33, 0x39, 0x2F, 0x27}, 17, 0},
    {0xA4, (uint8_t []){0x85, 0x85, 0x95, 0x82, 0xAF, 0xAA, 0xAD, 0x80, 0x10, 0x30, 0x40, 0x40, 0x20, 0xFF, 0x60, 0x30}, 16, 0},
    {0xA4, (uint8_t []){0x85, 0x85, 0x95, 0x85}, 4, 0},
    {0xBB, (uint8_t []){0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 8, 0},
    {0x11, (uint8_t []){0x00}, 0, 200}, // Sleep Out
    {0x29, (uint8_t []){0x00}, 0, 100}, // Display On
};

void app_main(void)
{
    example_bsp_enable_dsi_phy_power();
    example_bsp_init_lcd_backlight();
    example_bsp_set_lcd_backlight(EXAMPLE_LCD_BK_LIGHT_OFF_LEVEL);

    // 首先创建 MIPI DSI 总线，它还将初始化 DSI PHY
    esp_lcd_dsi_bus_handle_t mipi_dsi_bus;
    esp_lcd_dsi_bus_config_t bus_config = {                    \
        .bus_id = 0,                                           \
        .num_data_lanes = 1,                                   \
        .phy_clk_src = MIPI_DSI_PHY_CLK_SRC_DEFAULT,           \
        .lane_bit_rate_mbps = 650,                             \
    };
    ESP_ERROR_CHECK(esp_lcd_new_dsi_bus(&bus_config, &mipi_dsi_bus));

    ESP_LOGI(TAG, "Install MIPI DSI LCD control panel");
    esp_lcd_panel_io_handle_t mipi_dbi_io;
    // 我们使用DBI接口发送LCD命令和参数
    esp_lcd_dbi_io_config_t dbi_config = {   \
        .virtual_channel = 0,                \
        .lcd_cmd_bits = 8,                   \
        .lcd_param_bits = 8,                 \
    };

    ESP_ERROR_CHECK(esp_lcd_new_panel_io_dbi(mipi_dsi_bus, &dbi_config, &mipi_dbi_io));

    // 创建AXS15231B控制面板
    esp_lcd_panel_handle_t panel_handle;
    esp_lcd_dpi_panel_config_t dpi_config = {                 \
        .dpi_clk_src = MIPI_DSI_DPI_CLK_SRC_DEFAULT,          \
        .dpi_clock_freq_mhz = 27,                             \
        .virtual_channel = 0,                                 \
        .pixel_format = MIPI_DPI_PX_FORMAT,                   \
        .num_fbs = 1,                                         \
        .video_timing = {                                     \
            .h_size = 258,                                    \
            .v_size = 960,                                    \
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
    ESP_ERROR_CHECK(esp_lcd_new_panel_axs15231b(mipi_dbi_io, &panel_config, &panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));

    // 打开背光
    example_bsp_set_lcd_backlight(EXAMPLE_LCD_BK_LIGHT_ON_LEVEL);

    ESP_LOGI(TAG, "Initialize LVGL library");
    lv_init();
    // 创建 lvgl 显示
    lv_display_t *display = lv_display_create(EXAMPLE_MIPI_DSI_LCD_H_RES, EXAMPLE_MIPI_DSI_LCD_V_RES);
    // 将 mipi 面板句柄关联到显示器
    lv_display_set_user_data(display, panel_handle);
    // 创建绘制缓冲区
    void *buf1 = NULL;
    void *buf2 = NULL;
    ESP_LOGI(TAG, "Allocate separate LVGL draw buffers");
    // 笔记:
    // 将显示缓冲区保存在**内部** RAM 中可以加快 UI 速度，因为 LVGL 大量使用它，并且它应该具有快速的访问时间
    // 此示例从 PSRAM 分配缓冲区主要是因为我们想节省内部 RAM
    size_t draw_buffer_sz = EXAMPLE_MIPI_DSI_LCD_H_RES * EXAMPLE_LVGL_DRAW_BUF_LINES * sizeof(lv_color_t);
    buf1 = heap_caps_malloc(draw_buffer_sz, MALLOC_CAP_SPIRAM);
    assert(buf1);
    buf2 = heap_caps_malloc(draw_buffer_sz, MALLOC_CAP_SPIRAM);
    assert(buf2);
    // 初始化 LVGL 绘制缓冲区
    lv_display_set_buffers(display, buf1, buf2, draw_buffer_sz, LV_DISPLAY_RENDER_MODE_PARTIAL);
    // 设置颜色深度
    lv_display_set_color_format(display, LV_COLOR_FORMAT);
    // 设置可以将渲染的图像复制到显示屏某个区域的回调
    lv_display_set_flush_cb(display, example_lvgl_flush_cb);

    ESP_LOGI(TAG, "Register DPI panel event callback for LVGL flush ready notification");
    esp_lcd_dpi_panel_event_callbacks_t cbs = {
        .on_color_trans_done = example_notify_lvgl_flush_ready,
    };
    ESP_ERROR_CHECK(esp_lcd_dpi_panel_register_event_callbacks(panel_handle, &cbs, display));

    ESP_LOGI(TAG, "Use esp_timer as LVGL tick timer");
    const esp_timer_create_args_t lvgl_tick_timer_args = {
        .callback = &example_increase_lvgl_tick,
        .name = "lvgl_tick"};
    esp_timer_handle_t lvgl_tick_timer = NULL;
    ESP_ERROR_CHECK(esp_timer_create(&lvgl_tick_timer_args, &lvgl_tick_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(lvgl_tick_timer, EXAMPLE_LVGL_TICK_PERIOD_MS * 1000));

    // LVGL API 旨在跨线程调用，不受保护，因此我们在这里使用互斥锁
    lvgl_api_mux = xSemaphoreCreateRecursiveMutex();
    assert(lvgl_api_mux);

    ESP_LOGI(TAG, "Create LVGL task");
    xTaskCreate(example_lvgl_port_task, "LVGL", EXAMPLE_LVGL_TASK_STACK_SIZE, NULL, EXAMPLE_LVGL_TASK_PRIORITY, NULL);

    ESP_LOGI(TAG, "Display LVGL Meter Widget");
    // 由于 LVGL API 不是线程安全的，因此锁定互斥锁
    if (example_lvgl_lock(-1))
    {
         example_lvgl_demo_ui(display);
        // 释放互斥锁
        example_lvgl_unlock();
    }
}
