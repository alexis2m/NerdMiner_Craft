# Build and flash

This is the procedure for building NerdMiner_Craft from source and flashing it to your Freenove FNK0103. If you only want to flash a pre-built binary, grab the `.bin` files from [Releases](https://github.com/alexis2m/NerdMiner_Craft/releases) and skip to [Flashing](#flashing).

## Prerequisites

- Python 3.9+ on `PATH`
- [PlatformIO Core](https://docs.platformio.org/en/latest/core/installation/index.html) — install with `pip install platformio` or via the VS Code extension.
- A USB-C cable known to carry data (many cheap cables are charge-only).
- macOS / Linux: no driver. Windows: install Silicon Labs CP210x or CH340 driver depending on the USB-UART chip on your specific board (Freenove documents which one in their hardware PDF).

## Picking the right env

Each FNK0103 variant has its own PlatformIO env. List them:

```bash
pio project config | grep "^\[env:NerdMiner_Craft"
```

Pick the env matching your hardware:

| Variant | Env name |
|---------|----------|
| 2.8" ST7789 TN (320×240) | `NerdMiner_Craft-FNK0103-ST7789-28` |
| 2.8" ILI9341 TN (320×240) | `NerdMiner_Craft-FNK0103-ILI9341-28` |
| 3.2" ST7789 IPS (320×240) | `NerdMiner_Craft-FNK0103-ST7789-32-IPS` |
| 3.5" ST7796 TN (480×320) | `NerdMiner_Craft-FNK0103-ST7796-35` |
| 4.0" ST7796 TN (480×320) | `NerdMiner_Craft-FNK0103-ST7796-40` |

> Some entries are TBD until the corresponding driver is merged. Check `platformio.ini` for the current set.

## Building

```bash
pio run -e <env>
```

Artifacts land under `.pio/build/<env>/`:

- `firmware.bin`     — the application
- `bootloader.bin`   — second-stage bootloader
- `partitions.bin`   — partition table

## Flashing

### Easy path: PlatformIO

```bash
pio run -e <env> -t upload
```

It will pick the serial port automatically; if more than one is present, set `upload_port = /dev/cu.usbmodemXXXX` (or `COMx` on Windows) under your env block.

### Manual path: esptool

If `pio` can't see the port, drop to esptool:

```bash
pip install esptool
esptool.py --chip esp32s3 --port /dev/cu.usbmodemXXXX --baud 921600 \
    write_flash \
    0x0000  .pio/build/<env>/bootloader.bin \
    0x8000  .pio/build/<env>/partitions.bin \
    0x10000 .pio/build/<env>/firmware.bin
```

### Browser path: ESP Web Tools

[https://espressif.github.io/esptool-js/](https://espressif.github.io/esptool-js/) works from Chrome / Edge / Brave. Connect at 115200 baud, pick the three `.bin` files at the offsets above. Useful when you don't have PlatformIO installed.

## First boot

1. Power the board. The LCD shows the loading splash within ~2 seconds.
2. The board comes up as a Wi-Fi access point named **`MinerAP`** (password `MineYourCoins`). Connect a phone or laptop.
3. Open `http://192.168.4.1` if the captive portal doesn't auto-open.
4. Pick your home Wi-Fi (2.4 GHz only — ESP32 doesn't do 5 GHz), enter your **public** BTC receiving address, and optionally pick a different pool. Save.
5. The device reboots and starts mining. The mining stats screen scrolls in.

## Recovering a bricked board

A "brick" usually means a bad partition table or a flash that didn't finish. Steps:

1. Hold **BOOT** while plugging the USB cable to force download mode.
2. Erase flash:

   ```bash
   esptool.py --chip esp32s3 --port <port> erase_flash
   ```

3. Reflash from scratch with the manual path above.

## Serial logs

Always run with the serial monitor open while you work:

```bash
pio device monitor -b 115200
```

Logs include hashrate, accepted/rejected shares, captive-portal events, and any fault codes. Attach the relevant snippet when filing a bug.
