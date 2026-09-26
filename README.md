# lvgl-pfd

A Primary Flight Display (PFD) simulator written in C using [LVGL](https://lvgl.io/) and rendered through an SDL2 window. It mimics the core instruments found on a modern glass-cockpit PFD — attitude indicator, speed/altitude tapes, heading tape, roll indicator, and FMA annunciations — with real-time interactive control.

---

## Features

- **Artificial Horizon** — Sky/ground split with smooth roll and pitch rendering using triangle primitives
- **Pitch Ladder** — Graduated pitch lines (±30°) with degree labels, clipped to the ADI region
- **Roll Indicator** — Arc-based bank angle indicator (±60°) with a movable pointer triangle
- **Speed Tape** (left) — Vertical scrolling airspeed tape with current-value readout
- **Altitude Tape** (right) — Vertical scrolling altitude tape with current-value readout
- **Heading Tape** (bottom) — Horizontal compass tape with cardinal labels (N/E/S/W) and exact heading readout
- **FMA Bar** — Three-box Flight Mode Annunciator (e.g. `FMC SPD`, `LNAV`, flight phase)
- **FLT DIR Annunciator** — Toggleable "FLT DIR" label above the roll arc (enabled on screens >= 200 px tall)
- **Aircraft Chevron** — Fixed center-screen aircraft symbol
- **Night / Day Mode** — Configurable color palette (day: bright blue sky / green ground; night: dark navy / deep green)
- **Responsive scaling** — Font sizes and UI geometry adapt automatically to the configured screen resolution
- **STM32 H745I support** — Specifically designed for the screen of the STM32 H745I Discovery board (480x272).

---

## Dependencies

| Dependency | Purpose |
|---|---|
| [LVGL](https://github.com/lvgl/lvgl) | Graphics library (drawing primitives, event system) |
| [SDL2](https://www.libsdl.org/) | Window creation, keyboard and mouse input |
| [FreeType2](https://freetype.org/) | Runtime TrueType font rendering via `lv_freetype` |
| [B612 / B612 Mono fonts](https://b612-font.com/) | Aerospace-grade monospaced typeface used for all labels |

> LVGL is expected to be built as part of the `lv_port_linux` project. The default `LV_DIR` in the Makefile points to `~/Downloads/lv_port_linux/lvgl/src` — adjust this to match your local setup. If `lv_port_linux` is not installed, run `git submodule update --init` to clone it.

---

## Font Setup

The project uses the **B612** and **B612 Mono** font families. Font paths are hard-coded in [`include/pfd.h`](include/pfd.h):

```c
#define PATH_REGULAR      "~/Documents/B612-Regular.ttf"
#define PATH_MONO_REGULAR "~/Documents/B612Mono-Regular.ttf"
#define PATH_MONO_BOLD    "~/Documents/B612Mono-Bold.ttf"
```

Download the fonts from the [B612 website](https://b612-font.com/) or install them via your package manager, then place them at the paths above (or update the `#define`s accordingly).

---

## Configuration

Edit [`config.h`](config.h) before building:

```c
#define PFD_BG_NIGHT_MODE 0        // 0 = day mode, 1 = night mode

#define SCALE 3                    // Resolution multiplier (for larger displays, default: 1)

#define SCR_WIDTH  (480 * SCALE)   // Effective width  (1440 px for scale = 3)
#define SCR_HEIGHT (272 * SCALE)   // Effective height (816 px for scale = 3)
```

> **Note:** Editing [`include/pfd.h`](include/pfd.h) is not recommended. All user-facing configuration lives in `config.h`. If `pfd.h` is modified, and resulted in a corruption, restore it with:
> ```bash
> git restore include/pfd.h
> ```

---

## Building

```bash
# Build the simulator
make

# Build and run immediately
make run

# Remove build artifacts
make clean
```

The compiled binary is placed at `bin/pfd_sim`.

### Adjusting the LVGL path

If your `lv_port_linux` checkout lives somewhere other than the default, override `LV_DIR` on the command line:

```bash
make LV_DIR=/path/to/lv_port_linux/lvgl/src
```

---

## Controls

### Keyboard

| Key | Action |
|---|---|
| `W` / `S` | Pitch nose-up / nose-down (0.1 per press) |
| `A` / `D` | Roll left / right (0.5 per press; only when airspeed > 0) |
| Arrow Up / Down | Increase / decrease airspeed (2.5 kt per press) |
| Arrow Left / Right | Change heading (2 deg; only in **TAXI** mode) |
| `F` | Toggle **FLT DIR** annunciator |
| `0` | Set flight phase to **PARK** |
| `1` | Set flight phase to **TAXI** |
| `2` | Set flight phase to **TKOFF** |
| `3` | Set flight phase to **CRUISE** |
| `4` | Set flight phase to **LND** |

### Mouse

| Action | Effect |
|---|---|
| Left-click and drag | Set roll (±45°) and pitch (±30°) proportionally to cursor position relative to screen center |
| Scroll wheel | Increase / decrease airspeed (2.5 kt per tick) |

---

## Simulation Behaviour

The main loop runs at approximately 50 Hz (20 ms sleep per tick) and continuously updates:

- **Heading** — Drifts proportionally to the current roll angle (`roll * 0.05` deg per tick)
- **Altitude** — Climbs/descends based on pitch and airspeed (`pitch * 0.1 * speed / 200` ft per tick)

---

## Project Structure

```
lvgl-pfd/
├── config.h          # User-facing configuration (resolution, night mode)
├── pfd.c             # Main source: drawing routines, input handlers, simulation loop
├── include/
│   └── pfd.h         # Internal macros, constants, font handles, function declarations
├── Makefile          # Build system
└── bin/
    └── pfd_sim       # Compiled binary (generated by make)
```

---

## License

This project is licensed under the **MIT License** — see the [LICENSE](LICENSE) file for details.

Copyright (c) 2026 Arda Türkmenoğlu
