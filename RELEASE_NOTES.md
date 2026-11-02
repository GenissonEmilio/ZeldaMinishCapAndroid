# Minish Cap Android v0.4.0 — European ROM support

The original European version of The Minish Cap is now supported alongside
USA in the same Android APK. The app verifies the ROM and automatically
selects its own recompiled code, symbols and executable RAM routines.

## Playing in Spanish

1. Install the ARM64 APK on Android 9 or newer. It uses the same application
   ID and signing certificate as v0.3.1, so install it as an update to keep
   your existing data.
2. Import your own unmodified Europe ROM (`.gba` or a ZIP containing one ROM).
3. At the game's file-selection screen, open **Language** and select
   **ESPAÑOL**. English, French, German and Italian are also available.

Supported EU dump: **BZMP**, revision 0, **16,777,216 bytes**.

- SHA-1: `cff199b36ff173fb6faf152653d1bccf87c26fb7`
- MD5: `2af78edbe244b5de44471368ae2b6f0b`
- CRC32: `E8637292`

## Changes

- Independent EU symbol/layout metadata from the pinned, byte-matching
  zeldaret/tmc build, and a separate generated EU corpus. This is more than
  adding another accepted checksum; the SHA-1 gate remains mandatory.
- Separate imported ROMs, cartridge saves, quick states, suspension files and
  code caches for USA and Europe. The last imported region becomes active;
  importing the other region preserves its existing progress.
- Save export uses the selected region, including when Android recreates the
  launcher while the destination picker is open.
- Source CI covers region dispatch, symbol and RAM-template isolation, plus
  transactional imports and save-library separation.

## Validation and current limits

The EU ZIP was imported through the Android picker on a POCO M8 Pro 5G.
Spanish selection, new-file/save creation, the introduction, the first scene
and EU suspend/resume were exercised. The measured 20-second samples ran at
about 59.3–59.4 guest FPS in strict mode, with no dispatch misses, interpreted
instructions or healed code. A USA boot check also passed, and its launch
left the EU cartridge save byte-identical. These checks do not cover the
entire campaign.

The in-inventory photo album/camera and Adaptive Widescreen remain
**USA-only**. The Android gallery remains available for both regions.

The public APK is non-debuggable and has no INTERNET permission. The open,
MIT-licensed Cult of GBA BIOS is included; no game ROM or signing key is
included. You must supply your own legally dumped game.

See [ANDROID.md](ANDROID.md) for build and usage details, and
[symbols/eu/provenance.json](symbols/eu/provenance.json) for EU metadata provenance.
