# Architecture

This document is a map of the firmware. It is the entry point for anyone touching the code; deeper documents (linked at the end) cover specific subsystems.

## Big picture

```text
                                +---------------------+
                Wi-Fi  <------> |   wManager.cpp      |  captive portal, NVS settings
                                +---------------------+
                                          |
                                          v
       Stratum (TCP)  <-->  +------------------------+         +-----------------+
                            |   stratum.cpp/.h       | <-----> |  mining.cpp/.h  |  hash loop, two cores
                            +------------------------+         +-----------------+
                                          |                              |
                                          v                              v
                                +------------------------+    +------------------------------+
                                |   monitor.cpp/.h       |<-> |  src/drivers/displays/*      |  per-board screen renderers
                                +------------------------+    +------------------------------+
                                                                          |
                                                                          v
                                                            +------------------------------+
                                                            |  src/media/images_<WxH>.h    |  RGB565 PROGMEM bitmaps
                                                            +------------------------------+
```

The split is:

- **Network / config** — `wManager.*`, captive portal, NVS-stored settings (Wi-Fi, BTC address, pool).
- **Mining loop** — `mining.*` runs the SHA‑256 inner loop, pinned to a CPU core. `stratum.*` talks to the pool.
- **Telemetry** — `monitor.*` aggregates the data the screens want to display (hashrate, valid shares, uptime, etc.) and exposes it via `getMiningData()`.
- **Display abstraction** — every board provides a `DisplayDriver` (function-pointer struct) and a set of "cyclic screens" that take turns on the panel. The mining loop has no idea which screen is rendering; the driver pulls fresh data each frame.
- **Art assets** — pixel art lives in `src/media/images_<W>_<H>.h` as `PROGMEM` `uint16_t` arrays in RGB565, generated from PNGs with [`tools/png2rgb565.py`](../tools/png2rgb565.py).

## Threads / cores

Both ESP32 cores are used. The mining workers are pinned (`xTaskCreatePinnedToCore`) so the screen redraw cannot starve hashing. Every time a Stratum `mining.notify` lands, in-flight work is invalidated to avoid stale shares.

## How a board "exists" in the code

Adding a board means wiring up four things:

1. A **board JSON** in `boards/` (sometimes — PlatformIO ships some out of the box).
2. A **PlatformIO env** in [`platformio.ini`](../platformio.ini) that picks the board, library deps, and `-D` defines.
3. A **device header** in `src/drivers/devices/<name>.h` with pin assignments (`#define TFT_BL`, `TFT_DC`, etc.) and the build-defined identifier (e.g. `NERDMINER_FNK0103_ST7789_28`).
4. A **display driver** in `src/drivers/displays/<name>DisplayDriver.cpp` guarded by `#ifdef <NAME>` that fills out a `DisplayDriver` struct and is `extern`ed in [`displayDriver.h`](../src/drivers/displays/displayDriver.h).

Step-by-step in [`docs/boards.md`](boards.md).

## How a screen "exists" in the code

Each driver exposes:

- One or more **cyclic screens** (e.g. miner stats, clock, global stats). They're cycled by a timer in `monitor.cpp`.
- A **loading screen** (boot splash) and a **setup screen** (Wi-Fi captive-portal hint).
- An optional **animation function** that the main loop calls every frame to update sprites without re-rendering the static background.

Step-by-step in [`docs/screens.md`](screens.md).

## Build / flash flow

PlatformIO builds one env at a time:

```bash
pio run -e <env>           # compile
pio run -e <env> -t upload # flash
pio device monitor -b 115200
```

CI replicates this in [`.github/workflows/build.yml`](../.github/workflows/build.yml). Tagged releases bundle the artifacts via [`.github/workflows/release.yml`](../.github/workflows/release.yml).

## Where to look next

| If you want to... | Read |
|-------------------|------|
| Add a Minecraft sprite or animate a screen | [`docs/screens.md`](screens.md) |
| Port to a new FNK0103 variant or unrelated board | [`docs/boards.md`](boards.md) |
| Build, flash, and recover a brick | [`docs/build-and-flash.md`](build-and-flash.md) |
| Understand the mining inner loop | Read `src/mining.cpp` from `runMiner()` down |
| Understand the Stratum dialect | Read `src/stratum.cpp`; pool default is `public-pool.io:21496` |
