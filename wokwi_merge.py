"""
PlatformIO post-build hook: produce a merged-image firmware_merged.bin
that Wokwi can load directly from offset 0x0.

PIO's per-target firmware.bin is just the application (would normally
be flashed to 0x10000). Wokwi loads a single .bin from offset 0x0
expecting the bootloader at the start, so handing it firmware.bin alone
makes the simulator interpret app bytes as bootloader and crash at the
entry point. firmware_merged.bin avoids that by stitching:

    bootloader.bin  -> 0x1000
    partitions.bin  -> 0x8000
    firmware.bin    -> 0x10000

into one image starting at 0x0.

Wired up via extra_scripts in the WOKWI env(s). No-op for envs that
do not include this script.
"""
import os
import subprocess
from pathlib import Path

Import("env")  # noqa: F821 — provided by SCons


def merge_for_wokwi(source, target, env):
    build_dir = Path(env.subst("$BUILD_DIR"))
    bootloader = build_dir / "bootloader.bin"
    partitions = build_dir / "partitions.bin"
    firmware = build_dir / "firmware.bin"
    merged = build_dir / "firmware_merged.bin"

    missing = [p for p in (bootloader, partitions, firmware) if not p.exists()]
    if missing:
        print(f"[wokwi_merge] skipping — missing inputs: {missing}")
        return

    esptool_dir = Path(env.subst("$PROJECT_PACKAGES_DIR")) / "tool-esptoolpy"
    esptool_py = esptool_dir / "esptool.py"
    python = env.subst("$PYTHONEXE")

    cmd = [
        python, str(esptool_py),
        "--chip", "esp32",
        "merge_bin",
        "-o", str(merged),
        "--flash_mode", "dio",
        "--flash_size", "4MB",
        "0x1000", str(bootloader),
        "0x8000", str(partitions),
        "0x10000", str(firmware),
    ]
    print(f"[wokwi_merge] {' '.join(cmd)}")
    subprocess.check_call(cmd)
    print(f"[wokwi_merge] -> {merged} ({merged.stat().st_size} bytes)")


# Run after the application binary is built.
env.AddPostAction("$BUILD_DIR/${PROGNAME}.bin", merge_for_wokwi)  # noqa: F821
