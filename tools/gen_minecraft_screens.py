#!/usr/bin/env python3
"""Generate Minecraft-themed PNG screens for the FNK0103B panel.

Programmatic alternative to hand-painting in Pixelorama. Uses the same
32-color palette as media-src/minecraft-palette.gpl so screens stay
visually coherent with anything hand-painted later.

Each screen is intentionally simple: tiled block textures + a framed
content area. The firmware overlays text (hashrate, time, version) on
top, so the PNGs are the *background plate* only, not finished mockups.

Run from repo root:
    python tools/gen_minecraft_screens.py

Outputs to media-src/fnk0103b-320x240/. Idempotent — re-running
overwrites the PNGs with the latest generator output.
"""
from __future__ import annotations

import random
from pathlib import Path

from PIL import Image, ImageDraw

# 32-color Minecraft palette (mirrors media-src/minecraft-palette.gpl).
P = {
    "black": (0, 0, 0),
    "white": (255, 255, 255),
    "gui_text_shadow": (31, 31, 31),
    "gui_slot_shadow": (55, 55, 55),
    "gui_inv_mid": (139, 139, 139),
    "gui_inv_light": (198, 198, 198),
    "gui_border": (85, 85, 85),
    "gui_yellow": (255, 255, 170),
    "dirt": (134, 96, 67),
    "dirt_shadow": (90, 63, 44),
    "grass": (94, 159, 61),
    "grass_shadow": (63, 111, 35),
    "sand": (219, 198, 129),
    "stone": (125, 125, 125),
    "stone_shadow": (92, 92, 92),
    "cobble": (140, 140, 140),
    "cobble_shadow": (94, 94, 94),
    "oak_plank": (156, 127, 78),
    "oak_shadow": (111, 88, 50),
    "oak_grain": (80, 61, 34),
    "iron": (216, 216, 216),
    "iron_shadow": (160, 160, 160),
    "gold": (255, 217, 59),
    "gold_shadow": (184, 145, 36),
    "diamond": (93, 236, 245),
    "diamond_shadow": (63, 161, 168),
    "redstone": (226, 43, 23),
    "lapis": (52, 94, 200),
    "emerald": (23, 221, 98),
    "lava": (255, 106, 0),
    "lava_shadow": (163, 61, 0),
    "water": (63, 118, 228),
}


def speckle_tile(size: int, base: tuple[int, int, int],
                 dark: tuple[int, int, int],
                 darker: tuple[int, int, int] | None = None,
                 density: float = 0.30,
                 seed: int = 42) -> Image.Image:
    """Generate a Minecraft-style speckled block tile."""
    rng = random.Random(seed)
    img = Image.new("RGB", (size, size), base)
    for y in range(size):
        for x in range(size):
            r = rng.random()
            if r < density:
                img.putpixel((x, y), dark)
            elif darker is not None and r < density + 0.08:
                img.putpixel((x, y), darker)
    return img


def tile_fill(width: int, height: int, tile: Image.Image,
              y_offset: int = 0) -> Image.Image:
    """Tile a sample image across a region starting at y_offset."""
    bg = Image.new("RGB", (width, height), P["black"])
    for y in range(y_offset, height, tile.height):
        for x in range(0, width, tile.width):
            bg.paste(tile, (x, y))
    return bg


def draw_minecraft_panel(img: Image.Image, x: int, y: int, w: int, h: int,
                         fill_color: tuple[int, int, int] = None) -> None:
    """Draw a Minecraft-inventory-style 3-tone bevel panel.

    Outer dark border, lighter inner edge, mid-tone fill. Mimics the look
    of Minecraft's GUI item slots and inventory backgrounds.
    """
    if fill_color is None:
        fill_color = P["gui_inv_mid"]
    d = ImageDraw.Draw(img)
    # Outer dark border
    d.rectangle([x, y, x + w - 1, y + h - 1], outline=P["black"], width=1)
    # Light highlight (top + left, 1px inside)
    d.rectangle([x + 1, y + 1, x + w - 2, y + h - 2],
                outline=P["gui_inv_light"], width=1)
    # Dark shadow (bottom + right, 1px inside)
    d.line([(x + 1, y + h - 2), (x + w - 2, y + h - 2)], fill=P["gui_slot_shadow"])
    d.line([(x + w - 2, y + 1), (x + w - 2, y + h - 2)], fill=P["gui_slot_shadow"])
    # Inner fill
    d.rectangle([x + 2, y + 2, x + w - 3, y + h - 3], fill=fill_color)


def draw_pickaxe(img: Image.Image, cx: int, cy: int, scale: int = 2,
                 head_color=None, handle_color=None,
                 handle_shadow=None) -> None:
    """Draw a tiny iron pickaxe centered at (cx, cy)."""
    if head_color is None:
        head_color = P["iron"]
    if handle_color is None:
        handle_color = P["oak_plank"]
    if handle_shadow is None:
        handle_shadow = P["oak_grain"]

    # Pickaxe sprite, 13x13 pixels at scale=1 (scale up for larger)
    # Head is a horizontal bar with pointy ends, handle goes diagonal.
    # 'H' = head, 'h' = handle, 's' = handle shadow, '.' = transparent
    sprite = [
        ".HHHHHHHH....",
        "HHHHHHHHHH...",
        "HHHHHHHHHHH..",
        ".HHHHHHHH....",
        "....HHsh.....",
        "...HHsh......",
        "...Hsh.......",
        "..hsh........",
        "..hh.........",
        ".hh..........",
        ".h...........",
        "h............",
        ".............",
    ]
    color_map = {
        "H": head_color,
        "h": handle_color,
        "s": handle_shadow,
    }
    sw, sh = len(sprite[0]), len(sprite)
    x0, y0 = cx - (sw * scale) // 2, cy - (sh * scale) // 2
    for sy, row in enumerate(sprite):
        for sx, ch in enumerate(row):
            if ch in color_map:
                color = color_map[ch]
                for dy in range(scale):
                    for dx in range(scale):
                        px = x0 + sx * scale + dx
                        py = y0 + sy * scale + dy
                        if 0 <= px < img.width and 0 <= py < img.height:
                            img.putpixel((px, py), color)


def gen_loading_screen() -> Image.Image:
    """320x170 loading splash. Dirt+grass terrain backdrop, framed title plate.

    Title text (NerdMiner Craft) is intentionally NOT rendered into the
    PNG — the firmware overlays it via OpenFontRender so font choice is
    a separate iteration.
    """
    W, H = 320, 170

    # Tiled dirt across full canvas
    dirt = speckle_tile(16, P["dirt"], P["dirt_shadow"], seed=11)
    img = tile_fill(W, H, dirt)

    # Grass strip across the top (8px tall)
    grass_top = speckle_tile(16, P["grass"], P["grass_shadow"], seed=22)
    grass_strip = grass_top.crop((0, 0, 16, 8))
    for x in range(0, W, 16):
        img.paste(grass_strip, (x, 0))

    # Centered title panel — leaves room for firmware text overlay
    panel_w, panel_h = 200, 64
    panel_x = (W - panel_w) // 2
    panel_y = (H - panel_h) // 2 - 8
    draw_minecraft_panel(img, panel_x, panel_y, panel_w, panel_h,
                         fill_color=P["oak_plank"])

    # Pickaxe icon left of where the title text will go
    draw_pickaxe(img, panel_x + 28, panel_y + panel_h // 2, scale=2,
                 head_color=P["iron"], handle_color=P["oak_grain"],
                 handle_shadow=P["black"])

    return img


def main() -> None:
    out_dir = Path("media-src/fnk0103b-320x240")
    out_dir.mkdir(parents=True, exist_ok=True)

    loading = gen_loading_screen()
    out_path = out_dir / "loading.png"
    loading.save(out_path)
    print(f"Generated: {out_path} ({loading.width}x{loading.height})")


if __name__ == "__main__":
    main()
