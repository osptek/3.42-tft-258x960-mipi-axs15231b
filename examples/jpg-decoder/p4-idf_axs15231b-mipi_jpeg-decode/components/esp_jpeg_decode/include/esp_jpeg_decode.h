#pragma once

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "esp_err.h"
#include "driver/jpeg_decode.h"

typedef struct {
    FILE *input;                // 输入文件句柄
    uint8_t *jpeg_buf;          // JPEG 输入缓冲区
    uint8_t *output_buf;       // RGB565 输出缓冲区
    jpeg_decoder_handle_t decoder_engine;  // JPEG 解码器句柄
    int16_t w, h;               // 图像宽高
    size_t jpeg_buffer_size;    // JPEG 输入缓冲区大小
    size_t output_buffer_size;  // RGB565 输出缓冲区大小
    jpeg_decode_cfg_t decode_cfg; // JPEG 解码配置
} esp_jpeg_decode_t;

// 初始化 JPEG 解码器，指定缓冲区大小
esp_err_t esp_jpeg_decode_setup(esp_jpeg_decode_t *jpg, const char *path);

// 解码 JPEG 图像
esp_err_t esp_jpeg_decode(esp_jpeg_decode_t *jpg);

// 获取图像宽度
int16_t esp_jpeg_decode_get_width(esp_jpeg_decode_t *jpg);

// 获取图像高度
int16_t esp_jpeg_decode_get_height(esp_jpeg_decode_t *jpg);

// 获取输出缓冲区
uint8_t *esp_jpeg_decode_get_out_buf(esp_jpeg_decode_t *jpg);

// 关闭 JPEG 解码器
void esp_jpeg_decode_close(esp_jpeg_decode_t *jpg);
