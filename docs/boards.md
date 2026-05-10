# Adding a board

This walks through porting NerdMiner_Craft to a new board (whether that's another FNK0103 variant or something unrelated). Read [`architecture.md`](architecture.md) first.

## Decision tree

```text
Is the LCD controller IC already supported by an existing driver?
├── Yes  →  Reuse the driver, only add a device header + env.
└── No   →  Add a new display driver too.

Is the board JSON already shipped by PlatformIO?
├── Yes  →  `board = <name>` in platformio.ini, no JSON needed.
└── No   →  Drop a JSON in boards/.
```

## Files you will touch

| File | What goes there |
|------|-----------------|
| `boards/<name>.json` | (sometimes) PlatformIO board definition: MCU, flash, partition, upload protocol. |
| `platformio.ini` | New `[env:<name>]` block with build flags, lib_deps, partition. |
| `src/drivers/devices/<name>.h` | Pin assignments and the `#define <NAME>` identifier. |
| `src/drivers/displays/<name>DisplayDriver.cpp` | Or extend an existing driver behind `#ifdef`. |
| `src/drivers/displays/displayDriver.h` | `extern DisplayDriver <name>DisplayDriver;` |
| `src/drivers/displays/display.cpp` | Add the `#ifdef` arm that picks `currentDisplayDriver`. |
| `src/media/images_<W>_<H>.h` | New header if the resolution is new. |
| `.github/workflows/build.yml` | Append the new env to the matrix. |
| `.github/workflows/release.yml` | Same matrix. |
| `README.md` / `docs/build-and-flash.md` | Note the variant, link to art assets. |

## Worked example: Freenove FNK0103, 2.8" ST7789 (320×240)

The 2.8" ST7789 variant is the closest existing analogue to the LilyGo T-Display ecosystem — same controller, different resolution. Here's the order of operations:

### 1. PlatformIO env

The Freenove board uses an ESP32-S3-WROOM-1 (N16R8). PlatformIO ships `esp32-s3-devkitc-1` which is electrically compatible. In `platformio.ini`:

```ini
[env:NerdMiner_Craft-FNK0103-ST7789-28]
extends = env:NerdminerV2
board = esp32-s3-devkitc-1
board_build.partitions = huge_app.csv
build_flags =
    -D NERDMINER_FNK0103_ST7789_28=1
    -D USER_SETUP_LOADED=1
    -D ST7789_DRIVER=1
    -D TFT_WIDTH=240
    -D TFT_HEIGHT=320
    -D TFT_MISO=-1
    -D TFT_MOSI=11           ; verify against your board's silk screen
    -D TFT_SCLK=12
    -D TFT_CS=10
    -D TFT_DC=9
    -D TFT_RST=14
    -D TFT_BL=2
    -D LOAD_GLCD=1
    -D LOAD_FONT2=1
    -D LOAD_FONT4=1
    -D LOAD_GFXFF=1
    -D SMOOTH_FONT=1
    -D SPI_FREQUENCY=40000000
```

> **Pin numbers above are placeholders.** Cross-check against Freenove's hardware tutorial PDF for FNK0103 and the actual silk screen on your board before flashing.

### 2. Device header

`src/drivers/devices/fnk0103.h`:

```cpp
#pragma once

#ifdef NERDMINER_FNK0103_ST7789_28
  #define BOARD_NAME "Freenove FNK0103 — 2.8\" ST7789 TN"
  #define HAS_LCD     1
  #define LCD_WIDTH   320
  #define LCD_HEIGHT  240
  // Touch/buttons left undefined — board has no native buttons; we may add a
  // BOOT button shortcut later.
#endif
```

### 3. Display driver

If the existing `tDisplayDriver.cpp` is too tied to 320×170, copy it to a new `fnk0103DisplayDriver.cpp`, gate every function with `#ifdef NERDMINER_FNK0103_ST7789_28`, and adjust the WIDTH/HEIGHT constants and the image headers it references. Then in `displayDriver.h`:

```cpp
extern DisplayDriver fnk0103DisplayDriver;
```

And in `display.cpp` (or wherever `currentDisplayDriver` is selected):

```cpp
#elif defined(NERDMINER_FNK0103_ST7789_28)
  currentDisplayDriver = &fnk0103DisplayDriver;
```

### 4. Art

`src/media/images_320_240.h` — new file. Generate each array with [`tools/png2rgb565.py`](../tools/png2rgb565.py); see [`docs/screens.md`](screens.md).

### 5. CI

Append `"NerdMiner_Craft-FNK0103-ST7789-28"` to the matrix in [`.github/workflows/build.yml`](../.github/workflows/build.yml) and [`.github/workflows/release.yml`](../.github/workflows/release.yml).

## Verifying

```bash
pio run -e NerdMiner_Craft-FNK0103-ST7789-28
pio run -e NerdMiner_Craft-FNK0103-ST7789-28 -t upload
pio device monitor -b 115200
```

Expected serial output on a healthy boot includes the `BOARD_NAME` string and a Wi-Fi captive portal banner if not yet configured. The display should show the loading screen within ~2 seconds.
