# 3.42" 258×960 TFT MIPI module (AXS15231B) — documentation & samples

**简体中文：** [`README.md`](README.md)

---

> This repository provides **sample projects** for this module, together with datasheets, specifications, and interface / bring-up documentation for selection reference and integration.

## Product overview

| Item | Description |
|:--|:--|
| Module | 3.42-inch **TFT** panel, **258×960** resolution |
| Interface | **MIPI** |
| Driver IC | **AXS15231B** |
| Spec ID | **`3.42-tft-258x960-mipi-axs15231b`** is the common product designation in documentation |

---

## Repository layout

### Top-level

| Path | Contents |
|:--|:--|
| `docs/` | Datasheets, specifications, initialization documentation |
| `examples/` | **Sample projects** by category |

### `examples/` layout

| Location | Description (internal package folder) |
|:--|:--|
| `examples/` root | **ESP-IDF代码** (esp-lvgl-port + LVGL9) |
| `with-te/` | **屏幕防撕裂代码** |
| `jpg-decoder/` | **jpeg解码** |
| `display-touch-test/` | **单独测试显示**、**单独测试触摸** |

### Sample project paths

#### Baseline (`examples/` root)

| Description | Path |
|:--|:--|
| esp-lvgl-port + LVGL9 | `examples/P4-IDF_AXS15231B-MIPI_ESP-LVGL-PORT_V9/` |

#### Tear avoidance (`with-te/`)

| Description | Path |
|:--|:--|
| LVGL common demo | `examples/with-te/p4-idf_axs15231b-mipi_lvgl_common_demo/` |

#### JPEG decode (`jpg-decoder/`)

| Description | Path |
|:--|:--|
| JPEG decode | `examples/jpg-decoder/p4-idf_axs15231b-mipi_jpeg-decode/` |

#### Display / touch tests (`display-touch-test/`)

| Description | Path |
|:--|:--|
| MIPI display test | `examples/display-touch-test/esp32p4-idf5_axs15231b-mipi_lvgl9/` |
| Touch I2C test | `examples/display-touch-test/esp32p4-idf5_axs15231b-touch-i2c/` |
