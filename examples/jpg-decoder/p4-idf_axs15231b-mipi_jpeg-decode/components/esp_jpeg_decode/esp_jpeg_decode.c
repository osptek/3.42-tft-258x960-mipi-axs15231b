#include "esp_jpeg_decode.h"
#include "esp_log.h"

static const char *TAG = "ESP_JPEG_DECODE";

esp_err_t esp_jpeg_decode_setup(esp_jpeg_decode_t *jpg, const char *path) {
    // 打开 JPEG 文件
    jpg->input = fopen(path, "r");
    if (!jpg->input) {
        ESP_LOGE(TAG, "无法打开文件: %s", path);
        return ESP_FAIL;
    }

    // 配置 JPEG 解码引擎
    jpeg_decode_engine_cfg_t decode_eng_cfg = {
        .intr_priority = 0,
        .timeout_ms = 40,
    };
    ESP_ERROR_CHECK(jpeg_new_decoder_engine(&decode_eng_cfg, &jpg->decoder_engine));

    // 分配缓冲区
    size_t tx_buffer_size, rx_buffer_size;
    jpeg_decode_memory_alloc_cfg_t rx_mem_cfg = {
        .buffer_direction = JPEG_DEC_ALLOC_OUTPUT_BUFFER,
    };
    jpeg_decode_memory_alloc_cfg_t tx_mem_cfg = {
        .buffer_direction = JPEG_DEC_ALLOC_INPUT_BUFFER,
    };
    jpg->jpeg_buf = (uint8_t *)jpeg_alloc_decoder_mem(jpg->jpeg_buffer_size, &tx_mem_cfg, &tx_buffer_size);
    jpg->output_buf = (uint8_t *)jpeg_alloc_decoder_mem(jpg->output_buffer_size, &rx_mem_cfg, &rx_buffer_size);
    if (!jpg->jpeg_buf || !jpg->output_buf) {
        ESP_LOGE(TAG, "缓冲区分配失败");
        fclose(jpg->input);
        jpg->input = NULL;
        return ESP_FAIL;
    }

    // 将整个 JPEG 文件读取到输入缓冲区
    size_t bytes_read = fread(jpg->jpeg_buf, 1, jpg->jpeg_buffer_size, jpg->input);
    if (bytes_read == 0) {
        ESP_LOGE(TAG, "读取 JPEG 文件失败");
        fclose(jpg->input);
        jpg->input = NULL;
        free(jpg->jpeg_buf);
        jpg->jpeg_buf = NULL;
        free(jpg->output_buf);
        jpg->output_buf = NULL;
        return ESP_FAIL;
    }
    jpg->jpeg_buffer_size = bytes_read;

    return ESP_OK;
}

esp_err_t esp_jpeg_decode(esp_jpeg_decode_t *jpg) {
    // 获取 JPEG 头部信息
    jpeg_decode_picture_info_t header_info;
    esp_err_t ret = jpeg_decoder_get_info(jpg->jpeg_buf, jpg->jpeg_buffer_size, &header_info);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "获取 JPEG 信息失败: %s", esp_err_to_name(ret));
        return ret;
    }
    jpg->w = header_info.width;
    jpg->h = header_info.height;

    uint32_t out_size;
    ret = jpeg_decoder_process(jpg->decoder_engine, &jpg->decode_cfg, (const uint8_t *)jpg->jpeg_buf,
                               jpg->jpeg_buffer_size, (uint8_t *)jpg->output_buf, jpg->output_buffer_size, &out_size);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "JPEG 解码失败: %s", esp_err_to_name(ret));
        return ret;
    }

    return ESP_OK;
}

int16_t esp_jpeg_decode_get_width(esp_jpeg_decode_t *jpg) {
    return jpg->w;
}

int16_t esp_jpeg_decode_get_height(esp_jpeg_decode_t *jpg) {
    return jpg->h;
}

uint8_t *esp_jpeg_decode_get_out_buf(esp_jpeg_decode_t *jpg) {
    return jpg->output_buf;
}

void esp_jpeg_decode_close(esp_jpeg_decode_t *jpg) {
    if (jpg->input) {
        fclose(jpg->input);
        jpg->input = NULL;
    }
    if (jpg->jpeg_buf) {
        free(jpg->jpeg_buf);
        jpg->jpeg_buf = NULL;
    }
    if (jpg->output_buf) {
        free(jpg->output_buf);
        jpg->output_buf = NULL;
    }
    if (jpg->decoder_engine) {
        jpeg_del_decoder_engine(jpg->decoder_engine);
        jpg->decoder_engine = NULL;
    }
}