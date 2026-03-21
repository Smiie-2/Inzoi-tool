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
- **Coordinate Input** — Enter exact values via console commands
- **Grid Snapping** — Optional snap increments for movement, rotation, and scale
- **Axis Constraints** — Lock manipulation to a single axis (X/Y/Z) or plane (XY/XZ/YZ)

### User Interface
- **Console Commands** — Full TOOL API accessible from the UE4SS console
- **Actor Browser** — Search and select any actor by class name
- **Visual GUI** — Optional ImGui overlay via the C++ companion mod (see below)
- **Persistent Settings** — Preferences saved to JSON and restored on load

### Quality of Life
- **Full Undo/Redo** — Up to 100 levels of undo history (configurable)
- **Reset to Original** — Restore any object to its original transform with Delete key
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
2. Copy `UE4SS/Mods/InZoiTOOL/` into your UE4SS Mods directory:
   ```
   <inZOI>/BlueClient/Binaries/Win64/ue4ss/Mods/InZoiTOOL/
   ```
3. Add this line to `ue4ss/Mods/mods.txt`:
   ```
   InZoiTOOL : 1
   ```
4. Verify the `scripts/` subfolder contains: `main.lua`, `settings.lua`, `manipulator.lua`, `macros.lua`, `ui.lua`

### Step 4: Launch
1. Start inZOI — UE4SS loads automatically
2. Open the UE4SS console (` ~ ` key by default)
3. Press **F2** to toggle T.O.O.L. on
4. Use `TOOL.help()` in the console for the full command list

## Optional: Visual GUI Overlay

The Lua mod works entirely through keyboard shortcuts and the UE4SS console. For a visual ImGui overlay tab inside the UE4SS window, install the **InZoiTOOL_GUI** companion mod:

1. Build the C++ companion mod from `UE4SS/Mods/InZoiTOOL_GUI/dlls/main.cpp` (requires the UE4SS C++ mod SDK)
2. Place the compiled `main.dll` in `ue4ss/Mods/InZoiTOOL_GUI/dlls/`
3. Add `InZoiTOOL_GUI : 1` to `mods.txt`
4. A "T.O.O.L." tab will appear in the UE4SS debug window with full visual controls

See the [UE4SS C++ Mod Guide](https://docs.ue4ss.com/guides/creating-a-c++-mod.html) for build instructions.

## Keybindings

| Key | Action |
|-----|--------|
| `F2` | Toggle TOOL on/off |
| `F3` | Print status to console |
| `G` | Move mode |
| `R` | Rotate mode |
| `S` | Scale mode |
| `E` | Elevate mode |
| `X` / `Y` / `Z` | Constrain to axis |
| `Tab` | Cycle through axes |
| `Arrow Keys` | Nudge object (direction depends on mode) |
| `Page Up/Down` | Elevate up/down |
| `Ctrl+Z` | Undo |
| `Ctrl+Y` | Redo |
| `Delete` | Reset object to original transform |
| `Escape` | Deselect |

## Console API

Open the UE4SS console (`~` key by default) to use the TOOL API:

```lua
-- Browse and select objects
TOOL.browseActors("Actor")           -- List actors by class
TOOL.browseActors("StaticMeshActor") -- More specific class
TOOL.selectFromBrowse(3)             -- Select #3 from results
TOOL.selectByClass("BP_Furniture_C") -- Select first of class

-- Move
TOOL.move(10, 0, 0)        -- Move +10 on X axis
TOOL.moveTo(100, 200, 50)  -- Absolute position

-- Rotate
TOOL.rotate(0, 45, 0)      -- Rotate 45 degrees yaw
TOOL.rotateTo(0, 0, 0)     -- Reset rotation

-- Scale
TOOL.scale(1.5)             -- Scale to 150%
TOOL.scaleTo(2, 2, 2)      -- Absolute scale

-- Elevate
TOOL.elevate(10)            -- Raise by 10 units
TOOL.elevateTo(100)         -- Absolute height

-- Utilities
TOOL.undo()                 -- Undo last change
TOOL.redo()                 -- Redo
TOOL.reset()                -- Reset to original
TOOL.status()               -- Print current state
TOOL.help()                 -- Full help

-- Macros
TOOL.startMacro("myMacro")
TOOL.move(5, 0, 0)
TOOL.rotate(0, 90, 0)
TOOL.stopMacro()
TOOL.playMacro("myMacro")

-- Presets
TOOL.presets.faceNorth()
TOOL.presets.faceSouth()
TOOL.presets.nudge(1, "up")
```

## Configuration

Edit `scripts/tool_settings.json` in the mod folder or modify in-game via console.

Key settings:
- `moveGridSnap` — `0` for free movement, or `0.25` for grid snapping
- `rotateGridSnap` — `0` for free rotation, or `15` for 15-degree snaps
- `minScale` / `maxScale` — Scale limits
- `elevationStep` — How much Page Up/Down moves per press
- `moveSpeed` — Arrow key nudge amount

## Project Structure

```
UE4SS/Mods/
├── mods.txt                          # Add "InZoiTOOL : 1" here
├── InZoiTOOL/                        # Main Lua mod
│   └── scripts/
│       ├── main.lua                  # Entry point, keybindings, TOOL API
│       ├── manipulator.lua           # Object manipulation (move/rotate/scale)
│       ├── ui.lua                    # Console output + shared var sync
│       ├── settings.lua              # Persistent JSON settings
│       ├── macros.lua                # Macro record/playback
│       └── tool_settings.json        # Default settings
└── InZoiTOOL_GUI/                    # Optional C++ companion (ImGui)
    └── dlls/
        ├── main.cpp                  # ImGui tab source
        ├── CMakeLists.txt            # Build config
        └── main.dll                  # Compiled (you build this)
```

## How It Works

T.O.O.L. uses **UE4SS** — a community runtime that injects into inZOI and exposes Unreal Engine's reflection system to Lua. This gives access to:

- **Actor discovery** via `FindFirstOf()` / `FindAllOf()`
- **Transform manipulation** via `K2_SetActorLocation()`, `K2_SetActorRotation()`, `SetActorScale3D()`
- **Keyboard input** via `RegisterKeyBind()`
- **State synchronization** via `SetSharedVariable()` / `GetSharedVariable()` for the GUI companion

The optional C++ companion mod registers an ImGui tab in the UE4SS window and reads shared variables to display a visual overlay with clickable buttons.

## Known Limitations

- **No mouse click selection** — UE4SS Lua doesn't easily expose line trace APIs, so object selection is through the console Actor Browser (`TOOL.browseActors()`).
- **Game may fight placement** — inZOI's build system may validate placement and reset objects. Try manipulating during live mode (not build mode).
- **GUI requires C++ companion** — UE4SS Lua mods don't have direct ImGui access. The visual overlay needs the companion mod compiled from source.
- **Official Lua scripting (Dec 2026)** — KRAFTON plans official Lua support. T.O.O.L. may be updated to use the official API when available.

## License

Open source. Free to use, modify, and distribute for non-commercial purposes.

## Credits

- Inspired by [TwistedMexi's T.O.O.L. mod](https://www.curseforge.com/sims4/mods/t-o-o-l) for The Sims 4
- Built on [UE4SS](https://github.com/UE4SS-RE/RE-UE4SS) by the RE-UE4SS team
- For the [inZOI](https://playinzoi.com/) modding community
