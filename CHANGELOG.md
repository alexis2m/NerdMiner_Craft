# Changelog

All notable changes to NerdMiner_Craft are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/), and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html) once a `1.0.0` is tagged.

## [Unreleased]

### Added

- Forked from [BitMaker-hub/NerdMiner_v2](https://github.com/BitMaker-hub/NerdMiner_v2).
- `.github/` scaffolding: issue templates (bug, feature, theme proposal), PR template, CODEOWNERS, Dependabot for Actions, build workflow with cached PlatformIO, lint workflow (markdown + YAML), label catalog.
- Repo docs: `CONTRIBUTING.md`, `CODE_OF_CONDUCT.md` (Contributor Covenant 2.1), `SECURITY.md`, `SUPPORT.md`, this changelog.
- Doxygen / `docs/` placeholders for upcoming architecture documentation.

### Changed

- Release workflow approver switched from `BitMaker-hub` to `alexis2m` so tags can actually publish on this fork.
- Release naming convention updated to `nerdminer-craft-release-*` and `nerdminer-craft-prerelease-*`.

### Planned (not yet shipped)

- `boards/freenove-fnk0103-st7789-2.8.json` and matching display driver in `src/drivers/displays/`.
- `src/media/images_320_240.h` containing Minecraft-style screen art.
- Mining-loop-driven pickaxe swing animation.

[Unreleased]: https://github.com/alexis2m/NerdMiner_Craft/compare/main...HEAD
