# Contributing to NerdMiner_Craft

Thanks for your interest! This is a personal fork of [BitMaker-hub/NerdMiner_v2](https://github.com/BitMaker-hub/NerdMiner_v2) focused on:

1. Running on the **Freenove FNK0103 ESP32 Display** family.
2. A **Minecraft-style** UI with mining animations.

If your contribution is unrelated to those two goals, it's likely a better fit upstream.

## Ground rules

- Be kind. See [`CODE_OF_CONDUCT.md`](CODE_OF_CONDUCT.md).
- One change per PR. Mixing a refactor with a feature makes review painful.
- Mining and stratum code touches every board — keep changes there minimal and well justified.

## Setting up

You need [PlatformIO Core](https://docs.platformio.org/en/latest/core/installation/index.html). The project builds many environments; you only need to build the one for your hardware:

```bash
# List all environments declared in platformio.ini
pio project config

# Build a single environment
pio run -e <environment-name>

# Flash to the connected board
pio run -e <environment-name> -t upload

# Tail the serial monitor
pio device monitor -b 115200
```

There is no test suite for the firmware itself — verification is on real hardware.

## Workflow

1. **Open an issue first** for non-trivial changes. For sprite or theme work, use the [Theme proposal template](.github/ISSUE_TEMPLATE/theme_proposal.yml).
2. **Branch** from `main`:
   - `feat/<short-name>` — new feature
   - `fix/<short-name>` — bug fix
   - `theme/<short-name>` — sprite / animation / screen art
   - `board/<variant>` — board or display variant work
   - `docs/<short-name>` — documentation only
3. **Commit** in [Conventional Commits](https://www.conventionalcommits.org/) style:
   - `feat(screen): add diamond-pickaxe swing animation`
   - `fix(t-display): correct rotation on cold boot`
   - `docs(architecture): document image header generation`
4. **Open a PR** using the [pull request template](.github/PULL_REQUEST_TEMPLATE.md). Tick every board variant you actually booted on — the CI build can't tell you the colors are right.
5. **Update [`CHANGELOG.md`](CHANGELOG.md)** under `[Unreleased]`.

## Adding a screen or animation

The full architecture lives in [`docs/screens.md`](docs/screens.md). Short version:

- Pixel art goes in `media-src/` (PNG, transparent where appropriate, exact target resolution).
- Convert PNG → RGB565 C arrays with the [`tools/png2rgb565.py`](tools/png2rgb565.py) helper.
- Append the array to the right header in `src/media/images_<W>_<H>.h`.
- Wire it into the relevant display driver in `src/drivers/displays/`.
- Keep animation frame counts small — every frame eats flash, and the FNK0103 partition has limits.

## Adding a board variant

The full architecture lives in [`docs/boards.md`](docs/boards.md). Short version:

- New entry in `boards/` (JSON) if PlatformIO doesn't ship one.
- New env block in `platformio.ini` referencing it.
- New device header in `src/drivers/devices/`.
- New display driver in `src/drivers/displays/` (or extend an existing one if the IC is already supported).
- Update `default_envs` in `platformio.ini` and the matrix in `.github/workflows/build.yml`.

## Style

- C / C++: follow whatever style is already in the file you're editing. No automatic formatter is enforced today.
- Comments: explain **why**, not what. Prefer Doxygen-style `///` or `/** */` blocks at the top of public functions.
- Markdown: lints with `markdownlint-cli2` per [`.markdownlint.jsonc`](.markdownlint.jsonc). Most rules are off; long lines are fine.

## Reporting security issues

Don't open a public issue. See [`SECURITY.md`](SECURITY.md).
