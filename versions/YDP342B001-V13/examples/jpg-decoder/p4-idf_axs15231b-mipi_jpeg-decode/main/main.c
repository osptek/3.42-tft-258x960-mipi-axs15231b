/*
 * SPDX-FileCopyrightText: Copyright 2026 OSPTEK
 * SPDX-License-Identifier: CC-BY-4.0
 *
 * https://github.com/osptek
 */

#include "lcd_init.h"
#include "esp_sdmmc_card.h"
#include "esp_jpeg_decode.h"

#define ROOT "/sdcard"
#define JPEG_FILENAME ROOT "/output_256_960.jpg"

#define FRAME_WIDTH 256 // 帧宽
#define FRAME_HEIGHT 960 // 帧高

#if BSP_LCD_COLOR_DEPTH ==16
#define OUTPUT_COLOR_BYTE 2 //输出颜色字节
#define JPEG_DECODE_OUT_FORMAT JPEG_DECODE_OUT_FORMAT_RGB565
#elif BSP_LCD_COLOR_DEPTH ==24
#define OUTPUT_COLOR_BYTE 3 //输出颜色字节
#define JPEG_DECODE_OUT_FORMAT JPEG_DECODE_OUT_FORMAT_RGB888
#endif

// 变量
static int16_t x = 0, y = 0, w = -1, h = -1;

void app_main(void) {
    
    // 初始化 LCD
    ESP_ERROR_CHECK(app_lcd_init());

    // 初始化 SDMMC
    esp_sdmmc_pin_config_t pin_config = {
        .clk = SDMMC_CLK,    // 时钟引脚
        .cmd = SDMMC_CMD,    // 命令引脚
        .d0 = SDMMC_D0,      // 数据0引脚
        .d1 = SDMMC_D1,      // 数据1引脚
        .d2 = SDMMC_D2,      // 数据2引脚
        .d3 = SDMMC_D3,      // 数据3引脚
        .width = SDMMC_WIDTH, // 使用宏定义的总线宽度
        .slot = SDMMC_SLOT   // 使用宏定义的卡槽编号
    };
    esp_sdmmc_card_init(&pin_config);

    // 初始化 JPEG 解码器
    esp_jpeg_decode_t jpg = {
        .jpeg_buffer_size = FRAME_WIDTH * FRAME_HEIGHT, // 输入缓冲区大小
        .output_buffer_size = FRAME_WIDTH * FRAME_HEIGHT * OUTPUT_COLOR_BYTE, // 输出缓冲区大小
        .decode_cfg = {
            .output_format = JPEG_DECODE_OUT_FORMAT, // 输出格式
            .rgb_order = JPEG_DEC_RGB_ELEMENT_ORDER_BGR, // RGB 顺序
        },
    };
   esp_jpeg_decode_setup(&jpg, JPEG_FILENAME);

   // 解码 JPEG
   esp_jpeg_decode(&jpg);

   // 设置显示坐标
   w = esp_jpeg_decode_get_width(&jpg);
   h = esp_jpeg_decode_get_height(&jpg);

   // 显示图像
   uint8_t *out_buf = esp_jpeg_decode_get_out_buf(&jpg);
   drawRGBBitmap(x, y, w, h, out_buf);
   ESP_LOGI(TAG, "图像显示完成");

   // 清理
   esp_jpeg_decode_close(&jpg);
   esp_sdmmc_card_deinit();
   ESP_LOGI(TAG, "JPEG结束");
}
