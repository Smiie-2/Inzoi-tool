# InZoi T.O.O.L. - Takes Objects Off Lot

Advanced object manipulation mod for **inZOI**, inspired by [TwistedMexi's T.O.O.L. mod](https://www.curseforge.com/sims4/mods/t-o-o-l) for The Sims 4. Freely move, rotate, scale, and elevate any in-game object with precision controls — including placement beyond lot boundaries.

**This is a UE4SS Lua mod** that runs via the [UE4SS scripting system](https://github.com/UE4SS-RE/RE-UE4SS), the only currently supported way to execute code mods in inZOI.

## Features

### Core Manipulation
- **Free Movement** — Move objects along any axis with arrow keys and numeric input
- **Free Rotation** — Rotate on all three axes (pitch, yaw, roll) with optional angle snapping
- **Free Scaling** — Scale objects uniformly with configurable min/max limits
- **Elevation Control** — Raise or lower objects with Page Up/Down
- **Off-Lot Placement** — Place objects anywhere, beyond normal lot boundaries

### Precision Controls
- **Coordinate Input** — Enter exact values via the numeric input panel (N key)
- **Grid Snapping** — Optional snap increments for movement, rotation, and scale
- **Axis Constraints** — Lock manipulation to a single axis (X/Y/Z) or plane (XY/XZ/YZ)

### User Interface
- **ImGui Overlay** — Full HUD showing mode, axis, object info, and transform values
- **Actor Browser** — Search and select any actor by class name
- **Settings Panel** — Configure all settings in-game with live sliders
- **Macro Panel** — Record, play, and manage manipulation macros

### Quality of Life
- **Full Undo/Redo** — Up to 100 levels of undo history (configurable)
- **Reset to Original** — Restore any object to its original transform with Delete key
- **Persistent Settings** — Preferences saved to JSON and restored on load

### Extensibility
- **Global TOOL API** — Full API available from the UE4SS console
- **Macro System** — Record, save, and replay sequences of operations
- **Presets** — Built-in presets (face north/south, nudge, reset)

## Requirements

- **inZOI** (Early Access or later)
- **[inZOI Mod Enabler](https://www.nexusmods.com/inzoi/mods/1)** — Required to load unofficial mods
- **[UE4SS for inZOI](https://www.nexusmods.com/inzoi/mods/243)** — The Lua scripting runtime

## Installation

### Step 1: Install the Mod Enabler
1. Download the [inZOI Mod Enabler](https://www.nexusmods.com/inzoi/mods/1) from Nexus Mods
2. Extract `dsound.dll` and the `bitfix` folder into:
   ```
   <inZOI>/BlueClient/Binaries/Win64/
   ```

### Step 2: Install UE4SS
1. Download [UE4SS packaged for inZOI](https://www.nexusmods.com/inzoi/mods/243) from Nexus Mods
2. Extract into `<inZOI>/BlueClient/Binaries/Win64/` (creates the `ue4ss` folder)

### Step 3: Install T.O.O.L.
1. Download or clone this repository
2. Copy the `UE4SS/Mods/InZoiTOOL` folder into:
   ```
   <inZOI>/BlueClient/Binaries/Win64/ue4ss/Mods/InZoiTOOL/
   ```
3. Verify the folder contains: `main.lua`, `enabled.txt`, `settings.lua`, `manipulator.lua`, `macros.lua`, `ui.lua`

### Step 4: Launch
1. Start inZOI — UE4SS loads automatically
2. Press **F2** in-game to toggle the T.O.O.L. overlay
3. Use the **Actor Browser** in the overlay to find and select objects

## Keybindings

| Key | Action |
|-----|--------|
| `F2` | Toggle TOOL on/off |
| `G` | Move mode |
| `R` | Rotate mode |
| `S` | Scale mode |
| `E` | Elevate mode |
| `X` / `Y` / `Z` | Constrain to axis |
| `Tab` | Cycle through axes |
| `Arrow Keys` | Nudge object (direction depends on mode) |
| `Page Up/Down` | Elevate up/down |
| `N` | Open numeric coordinate input |
| `Ctrl+Z` | Undo |
| `Ctrl+Y` | Redo |
| `Delete` | Reset object to original transform |
| `Escape` | Deselect |

## Console API

Open the UE4SS console (`~` key by default) to use the TOOL API directly:

```lua
-- Select an actor by class name
TOOL.selectByClass("BP_Furniture_C")

-- Move the selected object
TOOL.move(10, 0, 0)        -- Move +10 on X axis
TOOL.moveTo(100, 200, 50)  -- Move to absolute position

-- Rotate
TOOL.rotate(0, 45, 0)      -- Rotate 45 degrees yaw
TOOL.rotateTo(0, 0, 0)     -- Reset rotation

-- Scale
TOOL.scale(1.5)             -- Scale to 150%
TOOL.scaleTo(2, 2, 2)      -- Set absolute scale

-- Elevate
TOOL.elevate(10)            -- Raise by 10 units
TOOL.elevateTo(100)         -- Set absolute height

-- Undo/Redo
TOOL.undo()
TOOL.redo()
TOOL.reset()                -- Reset to original transform

-- Macros
TOOL.startMacro("myMacro")
TOOL.move(5, 0, 0)
TOOL.rotate(0, 90, 0)
TOOL.stopMacro()
TOOL.playMacro("myMacro")  -- Replay the sequence

-- Presets
TOOL.presets.faceNorth()
TOOL.presets.faceSouth()
TOOL.presets.nudge(1, "up")
```

## Configuration

Edit `tool_settings.json` in the mod folder, or use the in-game Settings panel (click the "Settings" header in the overlay).

Key settings:
- `moveGridSnap` — Set to `0` for free movement, or a value like `0.25` for grid snapping
- `rotateGridSnap` — Set to `0` for free rotation, or `15` for 15-degree snaps
- `minScale` / `maxScale` — Limit how small/large objects can be scaled
- `elevationStep` — How much Page Up/Down moves per press

## Project Structure

```
InZoiTOOL/
├── UE4SS/Mods/InZoiTOOL/
│   ├── main.lua              # Entry point - keybindings, global API, initialization
│   ├── enabled.txt           # UE4SS mod enabler flag
│   ├── manipulator.lua       # Core object manipulation (move/rotate/scale/elevate)
│   ├── ui.lua                # ImGui overlay (HUD, actor browser, settings panel)
│   ├── settings.lua          # Persistent JSON settings
│   ├── macros.lua            # Macro record/playback system
│   └── tool_settings.json    # Default settings file
└── README.md
```

## How It Works

Unlike the official ModKit (which only supports asset mods), T.O.O.L. uses **UE4SS** — a community-created runtime that injects into inZOI's process and exposes Unreal Engine's reflection system to Lua scripts. This gives access to:

- **Actor discovery** via `FindFirstOf()` / `FindAllOf()`
- **Transform manipulation** via `K2_SetActorLocation()`, `K2_SetActorRotation()`, `SetActorScale3D()`
- **Keyboard input** via `RegisterKeyBind()`
- **GUI rendering** via UE4SS's built-in ImGui integration

This is the same approach used by other advanced inZOI mods like the Mod Menu.

## Known Limitations

- **No mouse click selection** — UE4SS Lua doesn't expose line trace / raycast APIs, so you must select objects through the Actor Browser or console. This may improve as UE4SS and inZOI modding evolve.
- **Game may fight placement** — inZOI's build system may have its own placement validation that resets objects. If an object snaps back, try moving it during live mode (not build mode).
- **Official Lua scripting (Dec 2026)** — KRAFTON plans to add official Lua scripting support. When available, T.O.O.L. may be updated to use the official API.

## License

This project is open source. Free to use, modify, and distribute for non-commercial purposes.

## Credits

- Inspired by [TwistedMexi's T.O.O.L. mod](https://www.curseforge.com/sims4/mods/t-o-o-l) for The Sims 4
- Built on [UE4SS](https://github.com/UE4SS-RE/RE-UE4SS) by the RE-UE4SS team
- For the [inZOI](https://playinzoi.com/) modding community
