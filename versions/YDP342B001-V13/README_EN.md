<p align="left"><img alt="OSPTEK" src="./images/logo.png" width="200" /></p>

<h1 align="center">OSPTEK 3.42″ TFT 258×960 (AXS15231B · MIPI)</h1>

<p align="center"><b>Bar TFT module · MIPI · AXS15231B · capacitive touch</b></p>

<p align="center"><a href="./README.md">简体中文</a> | English · <a href="../../README_EN.md">Family index</a></p>

<p align="center">
  <img alt="Size: 3.42 inch" src="https://img.shields.io/badge/Size-3.42%22-3498DB?style=flat-square" />
  <img alt="Resolution: 258x960" src="https://img.shields.io/badge/Resolution-258%C3%97960-8E44AD?style=flat-square" />
  <img alt="Interface: MIPI" src="https://img.shields.io/badge/Interface-MIPI-27AE60?style=flat-square" />
  <img alt="Driver: AXS15231B" src="https://img.shields.io/badge/Driver-AXS15231B-E7352C?style=flat-square" />
</p>

<p align="center"><img alt="OSPTEK 3.42&quot; 258×960 TFT MIPI module (AXS15231B) product image" src="./images/product.png" width="640" /></p>

## Contents

- [Overview](#overview)
- [Specifications](#specifications)
- [Sample projects](#sample-projects)
- [Repository layout](#repository-layout)
- [Resources](#resources)
- [Buy](#buy)
- [Support](#support)

---

## Overview

OSPTEK **3.42″ 258×960 TFT** is a **MIPI** color display module. Display and capacitive touch are both driven by **AXS15231B** (touch over I2C). Suited to bar-style HMI and narrow side panels.

Spec ID (repository name): `3.42-tft-258x960-mipi-axs15231b`

Current module version: **YDP342B001-V13**. Electrical and mechanical details follow [`docs/YDP_342_B001_V13_363bf4a9b8.pdf`](./docs/YDP_342_B001_V13_363bf4a9b8.pdf).

## Specifications

| Item | Spec |
| ---- | ---- |
| Size | 3.42 inch |
| Type | TFT (color) |
| Resolution | 258×960 |
| Interface | MIPI |
| Driver IC | AXS15231B |
| Touch driver | AXS15231B |

> Full outline, FPC definition, power, and timing follow the product datasheet / driver IC datasheet.

## Sample projects

| Description | Path |
| ---- | ---- |
| ESP32-P4 · AXS15231B MIPI + esp-lvgl-port / LVGL9 | [`examples/P4-IDF_AXS15231B-MIPI_ESP-LVGL-PORT_V9/`](./examples/P4-IDF_AXS15231B-MIPI_ESP-LVGL-PORT_V9/) |
| ESP32-P4 · LVGL + TE | [`examples/with-te/p4-idf_axs15231b-mipi_lvgl_common_demo/`](./examples/with-te/p4-idf_axs15231b-mipi_lvgl_common_demo/) |
| ESP32-P4 · JPEG decode demo | [`examples/jpg-decoder/p4-idf_axs15231b-mipi_jpeg-decode/`](./examples/jpg-decoder/p4-idf_axs15231b-mipi_jpeg-decode/) |
| ESP32-P4 · MIPI display test (LVGL9) | [`examples/display-touch-test/esp32p4-idf5_axs15231b-mipi_lvgl9/`](./examples/display-touch-test/esp32p4-idf5_axs15231b-mipi_lvgl9/) |
| ESP32-P4 · touch I2C test | [`examples/display-touch-test/esp32p4-idf5_axs15231b-touch-i2c/`](./examples/display-touch-test/esp32p4-idf5_axs15231b-touch-i2c/) |

## Repository layout

```text
3.42-tft-258x960-mipi-axs15231b/                                # repo root (nav: ../../README_EN.md)
└── versions/
    └── YDP342B001-V13/                                # full materials for this part number
        ├── README.md
        ├── README_EN.md
        ├── images/
        ├── docs/
        └── examples/
```

## Resources

### Product files

| Resource | Link |
| ---- | ---- |
| Product datasheet (YDP342B001-V13) | [`docs/YDP_342_B001_V13_363bf4a9b8.pdf`](./docs/YDP_342_B001_V13_363bf4a9b8.pdf) |
| Driver IC datasheet (AXS15231B) | [`docs/AXS_15231_B_Datasheet_V0_9_20240221_5a76ce6ce2.pdf`](./docs/AXS_15231_B_Datasheet_V0_9_20240221_5a76ce6ce2.pdf) |
| Init sequence (text) | [`docs/371+231_241217(显示底部线条、残影、喇叭黑屏优化、TE修改）.txt`](./docs/371%2B231_241217%28%E6%98%BE%E7%A4%BA%E5%BA%95%E9%83%A8%E7%BA%BF%E6%9D%A1%E3%80%81%E6%AE%8B%E5%BD%B1%E3%80%81%E5%96%87%E5%8F%AD%E9%BB%91%E5%B1%8F%E4%BC%98%E5%8C%96%E3%80%81TE%E4%BF%AE%E6%94%B9%EF%BC%89.txt) |

### Samples

- [ESP32-P4 AXS15231B MIPI + LVGL9](./examples/P4-IDF_AXS15231B-MIPI_ESP-LVGL-PORT_V9/)
- [ESP32-P4 LVGL + TE](./examples/with-te/p4-idf_axs15231b-mipi_lvgl_common_demo/)
- [ESP32-P4 JPEG decode](./examples/jpg-decoder/p4-idf_axs15231b-mipi_jpeg-decode/)
- [ESP32-P4 MIPI display test](./examples/display-touch-test/esp32p4-idf5_axs15231b-mipi_lvgl9/)
- [ESP32-P4 touch I2C test](./examples/display-touch-test/esp32p4-idf5_axs15231b-touch-i2c/)

## Buy

<p align="center">
  <a href="https://www.aliexpress.com/store/1105701619"><img alt="AliExpress store" src="https://img.shields.io/badge/AliExpress-Official_Store-FF6A00?style=for-the-badge" /></a>
  &nbsp;&nbsp;
  <a href="https://shop110742373.taobao.com/"><img alt="Taobao store" src="https://img.shields.io/badge/Taobao-Official_Store-FF6A00?style=for-the-badge" /></a>
</p>

**Overseas (AliExpress)**

- Store: [OSPTEK Official Store](https://www.aliexpress.com/store/1105701619)

**China (Taobao)**

- Store: [鱼鹰光电工厂店](https://shop110742373.taobao.com/)

## Support

- Technical support / product inquiry: <luyu@osptek.com>
- QQ group: **985881096**
- Website: <https://osptek.com/>
- Feel free to open an Issue in this repository with any questions

---

<p align="center"><sub>© 2026 OSPTEK · Materials in this repository are licensed under CC BY 4.0</sub></p>
