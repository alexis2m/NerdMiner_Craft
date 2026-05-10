# Media sources

Source artwork lives here. Generated `uint16_t` arrays under [`src/media/`](../src/media/) are derived from these files and should never be edited by hand.

## Layout

```
media-src/
├── README.md                         this file
├── minecraft-palette.gpl             palette shared across every sprite
└── fnk0103b-320x240/                 per-board sprite source
    ├── loading.png
    ├── setup.png
    ├── miner.png
    ├── clock.png
    ├── global.png
    ├── btc-price.png
    └── animations/
        ├── pickaxe-frame-0.png
        ├── pickaxe-frame-1.png
        └── …
```

## Workflow

1. **Open the palette in your pixel-art tool first.** Pixelorama: `File → Import → Import as Palette`. Aseprite: `Palette panel → folder icon → Load palette`. Both load `minecraft-palette.gpl` natively.
2. **Design at the native panel resolution** in landscape — `320 × 240` for the FNK0103B. Don't upscale; we want pixel-perfect art, not anti-aliased renderings.
3. **Export as PNG** with the palette embedded if your tool offers that (Pixelorama: `Export → Spritesheet/Image → keep palette`). RGBA is fine; alpha is collapsed to the background color at conversion time.
4. **Convert to RGB565** with the helper:

   ```bash
   python tools/png2rgb565.py \
     media-src/fnk0103b-320x240/miner.png \
     src/media/images_320_240.h
   ```

5. **Commit both** the source PNG and the regenerated header in the same commit. The PNG is the source of truth; the header is the build artifact, kept in-tree because PlatformIO builds don't run Python.

## Palette rationale

The palette in `minecraft-palette.gpl` is a 32-color subset chosen for the NerdMiner UI specifically. It covers:

- the canonical Minecraft GUI grays (slot borders, inventory background)
- a small set of iconic block colors (dirt, grass, stone, cobblestone, oak)
- the ore/treasure family (iron, gold, diamond, redstone, lapis, emerald)
- the elemental colors (lava, water)
- pure black/white for outlines and text

Stay inside the palette. RGB565 only encodes 65,536 distinct colors, so dithering across many similar shades wastes flash and never looks crisp on the panel anyway. A tight palette also keeps the screens visually coherent — every sprite "feels Minecraft" because it shares the same color vocabulary.

## Adding a new resolution

When porting to a different FNK0103 variant (e.g. 480×320 for the 3.5" boards), create a sibling folder `media-src/fnk0103n-480x320/` and regenerate to `src/media/images_480_320.h`. The palette is shared.
