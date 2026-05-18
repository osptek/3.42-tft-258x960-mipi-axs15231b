# 3.42 寸 258×960 TFT MIPI 模组（AXS15231B）资料与示例

**English：** [`README_EN.md`](README_EN.md)

---

> 本仓库提供该模组的 **示例工程**，以及数据手册、规格与接口说明等资料，便于选型参考与集成开发。

## 产品概要

| 项目 | 说明 |
|:--|:--|
| 模组规格 | 3.42 英寸 **TFT**，分辨率 **258×960** |
| 接口 | **MIPI** |
| 驱动芯片 | **AXS15231B** |
| 规格标识 | 产品资料中常用 **`3.42-tft-258x960-mipi-axs15231b`** 表示本规格 |

---

## 仓库结构

### 顶层目录

| 路径 | 说明 |
|:--|:--|
| `docs/` | 数据手册、规格说明、初始化相关文档 |
| `examples/` | 按功能分类的 **示例工程** |

### `examples/` 分类

| 分类 | 说明（对应内部资料目录） |
|:--|:--|
| `examples/` 根目录 | **ESP-IDF代码**（esp-lvgl-port + LVGL9） |
| `with-te/` | **屏幕防撕裂代码** |
| `jpg-decoder/` | **jpeg解码** |
| `display-touch-test/` | **单独测试显示**、**单独测试触摸** |

### 示例工程路径

#### 基础（`examples/` 根目录）

| 说明 | 路径 |
|:--|:--|
| esp-lvgl-port + LVGL9 | `examples/P4-IDF_AXS15231B-MIPI_ESP-LVGL-PORT_V9/` |

#### 屏幕防撕裂代码（`with-te/`）

| 说明 | 路径 |
|:--|:--|
| LVGL 通用演示 | `examples/with-te/p4-idf_axs15231b-mipi_lvgl_common_demo/` |

#### jpeg解码（`jpg-decoder/`）

| 说明 | 路径 |
|:--|:--|
| JPEG 解码 | `examples/jpg-decoder/p4-idf_axs15231b-mipi_jpeg-decode/` |

#### 单独测试（`display-touch-test/`）

| 说明 | 路径 |
|:--|:--|
| MIPI 显示测试 | `examples/display-touch-test/esp32p4-idf5_axs15231b-mipi_lvgl9/` |
| 触摸 I2C 测试 | `examples/display-touch-test/esp32p4-idf5_axs15231b-touch-i2c/` |
