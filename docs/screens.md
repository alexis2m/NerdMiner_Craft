# Screens, sprites, and animations

This document explains how the on-device UI is structured and how to add a new Minecraft-themed screen or animation. Read [`architecture.md`](architecture.md) first if `DisplayDriver` is unfamiliar.

## Mental model

A "screen" is a function that renders one frame to an off-screen sprite, then pushes it to the panel. There are three kinds:

| Kind | Signature | When called |
|------|-----------|-------------|
| **Loading** | `void()` | Once, at boot, before Wi-Fi is up. |
| **Setup** | `void()` | While the captive portal is active. |
| **Cyclic** | `void(unsigned long mElapsed)` | Repeatedly. The driver rotates through `cyclic_screens[]`. `mElapsed` is ms since the last call to *this* screen. |

There is also an optional `animateCurrentScreen(unsigned long frame)` that the main loop calls every frame so animations can update without re-rendering the whole background.

All three live behind the `DisplayDriver` struct ([`src/drivers/displays/displayDriver.h`](../src/drivers/displays/displayDriver.h)). The selected board provides exactly one driver; the mining/monitor code is display-agnostic.

## File layout

```text
src/drivers/displays/
  displayDriver.h          # struct + extern declarations for every driver
  display.h / display.cpp  # selects currentDisplayDriver based on build defines
  tDisplayDriver.cpp       # T-Display S3 (320x170)
  esp23_2432s028r.cpp      # CYD 2.8" (320x240)
  ...                      # one .cpp per board, guarded by #ifdef <BOARD>
src/media/
  images_320_170.h         # T-Display S3 art
  images_320_240.h         # FNK0103 320x240 art           (added by NerdMiner_Craft)
  images_480_320.h         # FNK0103 3.5"/4.0" art          (added by NerdMiner_Craft)
  myFonts.h                # bitmap fonts
media-src/                  # source PNGs (NerdMiner_Craft addition)
  fnk0103-320x240/
    miner-bg.png
    pickaxe-frame-0.png
    pickaxe-frame-1.png
    ...
```

## Adding a static screen

1. **Draw the art at the exact target resolution** — for the 2.8" FNK0103 this is 320×240. Save as PNG with a transparent background only if you intend to composite it; for full backgrounds, flatten to opaque pixels to save flash.
2. **Convert to RGB565** with [`tools/png2rgb565.py`](../tools/png2rgb565.py):

   ```bash
   python tools/png2rgb565.py \
       --input  media-src/fnk0103-320x240/miner-bg.png \
       --name   MinerScreen \
       --output - >> src/media/images_320_240.h
   ```

   The script emits a `static const uint16_t MinerScreen[] PROGMEM = { ... };` array plus `MinerScreenWidth` / `MinerScreenHeight` constants matching the existing convention.

3. **Reference the array from the driver** — push it as the background and overlay dynamic text on top:

   ```cpp
   void fnk0103_MinerScreen(unsigned long mElapsed) {
       mining_data data = getMiningData(mElapsed);
       background.pushImage(0, 0, MinerScreenWidth, MinerScreenHeight, MinerScreen);
       // Dynamic text drawn on top of the bitmap:
       render.rdrawString(data.currentHashRate.c_str(), 220, 110, TFT_WHITE);
       background.pushSprite(0, 0);
   }
   ```

4. **Register it** in the driver's `cyclic_screens[]` array and update `num_cyclic_screens`.

## Adding an animation

Animations live in `animateCurrentScreen(unsigned long frame)`. The contract:

- Called every main-loop iteration. `frame` is a monotonic counter — modulo it for cycle length.
- Does **not** re-push the background. It updates only the changing region with `pushImage()` followed by a partial `pushSprite(x, y, w, h)`.
- Must be cheap. The mining loop runs on the other core, but display SPI is shared — long pushes block other UI updates.

Pattern for a Minecraft pickaxe swing (4-frame cycle, ~125 ms per frame):

```cpp
static const uint16_t* PickaxeFrames[] = {
    PickaxeFrame0, PickaxeFrame1, PickaxeFrame2, PickaxeFrame3,
};

void fnk0103_AnimateMinerScreen(unsigned long frame) {
    if (frame % 8 != 0) return;          // 8 main-loop ticks per frame ≈ 125 ms
    const uint16_t idx = (frame / 8) % 4;
    background.pushImage(180, 60, PickaxeWidth, PickaxeHeight, PickaxeFrames[idx]);
    background.pushSprite(180, 60, PickaxeWidth, PickaxeHeight);
}
```

## Sprite-sheet vs per-frame arrays

For ≥8 frames, prefer a sprite sheet (single PNG, single array, offset addressing) — it cuts per-symbol bloat and makes flash-size accounting easier. For 2–4 frames, individual arrays read cleaner. The Minecraft pickaxe and TNT-detonate sprites in NerdMiner_Craft use individual arrays; the longer chest-opening sequence uses a sheet.

## Color palette

RGB565 packs 5/6/5 bits — be intentional with greens (the format favors green resolution). Minecraft's iconic palette translates well. A reference palette lives in [`media-src/palette.png`](../media-src/palette.png) (16 swatches) — match colors to it before exporting to keep screens visually consistent.

## Flash budget

Every PROGMEM bitmap costs `width × height × 2` bytes. At 320×240 that's **150 KB per full-screen image**. Check after each addition:

```bash
pio run -e <env> -t size
```

Total firmware (text + rodata) must stay under the partition size declared in the env (`huge_app.csv` gives ~3.6 MB; the default is much tighter). If you bust the budget: drop frames, share backgrounds across screens, or move to a sprite sheet.

## Testing

There's no unit test layer. To verify a screen:

1. Build and flash to the actual board (`pio run -e <env> -t upload`).
2. Watch the serial monitor at 115200 — every screen prints debug info on entry.
3. Photograph the panel; attach to the PR per the [PR template](../.github/PULL_REQUEST_TEMPLATE.md).
