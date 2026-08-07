<p align="left"><img alt="OSPTEK" src="./images/logo.png" width="200" /></p>

<h1 align="center">OSPTEK 3.42″ TFT 258×960（AXS15231B · MIPI）</h1>

<p align="center"><b>条状 TFT 模组 · MIPI · AXS15231B · 电容触摸</b></p>

<p align="center"><a href="./README_EN.md">English</a> | 简体中文</p>

<p align="center">
  <img alt="Size: 3.42 inch" src="https://img.shields.io/badge/Size-3.42%22-3498DB?style=flat-square" />
  <img alt="Resolution: 258x960" src="https://img.shields.io/badge/Resolution-258%C3%97960-8E44AD?style=flat-square" />
  <img alt="Interface: MIPI" src="https://img.shields.io/badge/Interface-MIPI-27AE60?style=flat-square" />
  <img alt="Driver: AXS15231B" src="https://img.shields.io/badge/Driver-AXS15231B-E7352C?style=flat-square" />
</p>

## 目录

- [产品简介](#产品简介)
- [规格参数](#规格参数)
- [示例工程](#示例工程)
- [仓库结构](#仓库结构)
- [相关资料](#相关资料)
- [购买链接](#购买链接)
- [技术支持](#技术支持)

---

## 产品简介

OSPTEK **3.42 寸 258×960 TFT** 是一款 **MIPI** 接口彩色显示模组，显示驱动与触摸均为 **AXS15231B**（电容触摸经 I2C）。适合条状 HMI、侧边信息条与窄条交互面板等场景。

规格标识（仓库名）：`3.42-tft-258x960-mipi-axs15231b`

当前模组版本：**YDP342B001-V13**。电气与外形细节以 [`docs/YDP_342_B001_V13_363bf4a9b8.pdf`](./docs/YDP_342_B001_V13_363bf4a9b8.pdf) 为准。

## 规格参数

| 项目 | 规格 |
| ---- | ---- |
| 尺寸 | 3.42 英寸 |
| 类型 | TFT（彩色） |
| 分辨率 | 258×960 |
| 接口 | MIPI |
| 驱动 IC | AXS15231B |
| 触摸驱动 | AXS15231B |

> 完整外形尺寸、FPC 定义、供电与时序以产品规格书 / 驱动手册为准。

## 示例工程

| 说明 | 路径 |
| ---- | ---- |
| ESP32-P4 · AXS15231B MIPI + esp-lvgl-port / LVGL9 | [`examples/P4-IDF_AXS15231B-MIPI_ESP-LVGL-PORT_V9/`](./examples/P4-IDF_AXS15231B-MIPI_ESP-LVGL-PORT_V9/) |
| ESP32-P4 · LVGL + TE 防撕裂 | [`examples/with-te/p4-idf_axs15231b-mipi_lvgl_common_demo/`](./examples/with-te/p4-idf_axs15231b-mipi_lvgl_common_demo/) |
| ESP32-P4 · JPEG 解码演示 | [`examples/jpg-decoder/p4-idf_axs15231b-mipi_jpeg-decode/`](./examples/jpg-decoder/p4-idf_axs15231b-mipi_jpeg-decode/) |
| ESP32-P4 · MIPI 显示测试（LVGL9） | [`examples/display-touch-test/esp32p4-idf5_axs15231b-mipi_lvgl9/`](./examples/display-touch-test/esp32p4-idf5_axs15231b-mipi_lvgl9/) |
| ESP32-P4 · 触摸 I2C 测试 | [`examples/display-touch-test/esp32p4-idf5_axs15231b-touch-i2c/`](./examples/display-touch-test/esp32p4-idf5_axs15231b-touch-i2c/) |

## 仓库结构

```text
3.42-tft-258x960-mipi-axs15231b/
├── README.md
├── README_EN.md
├── MODULE_VERSION.md
├── LICENSE
├── images/          # README 用图
├── docs/            # 规格书、驱动手册、初始化等
└── examples/        # 示例工程
```

## 相关资料

### 本产品资料

| 资料 | 链接 |
| ---- | ---- |
| 产品规格书（YDP342B001-V13） | [`docs/YDP_342_B001_V13_363bf4a9b8.pdf`](./docs/YDP_342_B001_V13_363bf4a9b8.pdf) |
| 驱动 IC 数据手册（AXS15231B） | [`docs/AXS_15231_B_Datasheet_V0_9_20240221_5a76ce6ce2.pdf`](./docs/AXS_15231_B_Datasheet_V0_9_20240221_5a76ce6ce2.pdf) |
| 初始化序列（文本） | [`docs/371+231_241217(显示底部线条、残影、喇叭黑屏优化、TE修改）.txt`](./docs/371%2B231_241217%28%E6%98%BE%E7%A4%BA%E5%BA%95%E9%83%A8%E7%BA%BF%E6%9D%A1%E3%80%81%E6%AE%8B%E5%BD%B1%E3%80%81%E5%96%87%E5%8F%AD%E9%BB%91%E5%B1%8F%E4%BC%98%E5%8C%96%E3%80%81TE%E4%BF%AE%E6%94%B9%EF%BC%89.txt) |

### 示例工程

- [ESP32-P4 AXS15231B MIPI + LVGL9](./examples/P4-IDF_AXS15231B-MIPI_ESP-LVGL-PORT_V9/)
- [ESP32-P4 LVGL + TE](./examples/with-te/p4-idf_axs15231b-mipi_lvgl_common_demo/)
- [ESP32-P4 JPEG 解码](./examples/jpg-decoder/p4-idf_axs15231b-mipi_jpeg-decode/)
- [ESP32-P4 MIPI 显示测试](./examples/display-touch-test/esp32p4-idf5_axs15231b-mipi_lvgl9/)
- [ESP32-P4 触摸 I2C 测试](./examples/display-touch-test/esp32p4-idf5_axs15231b-touch-i2c/)

## 购买链接

<p align="center">
  <a href="https://shop110742373.taobao.com/"><img alt="淘宝官方店铺" src="https://img.shields.io/badge/淘宝-官方店铺-FF6A00?style=for-the-badge" /></a>
  &nbsp;&nbsp;
  <a href="https://www.aliexpress.com/store/1105701619"><img alt="速卖通官方店铺" src="https://img.shields.io/badge/速卖通-官方店铺-FF6A00?style=for-the-badge" /></a>
</p>

**国内（淘宝）**

- 店铺：[鱼鹰光电工厂店](https://shop110742373.taobao.com/)

**海外（AliExpress）**

- 店铺：[OSPTEK Official Store](https://www.aliexpress.com/store/1105701619)

## 技术支持

- 技术支持 / 产品咨询：<luyu@osptek.com>
- QQ 技术交流群：**985881096**
- 公司官网：<https://osptek.com/>
- 有任何问题，都可以在本仓库 Issues 中提问

---

<p align="center"><sub>© 2026 OSPTEK 鱼鹰光电 · 本仓库资料采用 CC BY 4.0 许可</sub></p>
