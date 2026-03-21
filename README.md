# InZoi T.O.O.L. - Takes Objects Off Lot

Advanced object manipulation mod for **inZOI**, inspired by [TwistedMexi's T.O.O.L. mod](https://www.curseforge.com/sims4/mods/t-o-o-l) for The Sims 4. Freely move, rotate, scale, and elevate any in-game object with precision controls — including placement beyond lot boundaries.

## Features

### Core Manipulation
- **Free Movement** — Move objects along any axis or combination of axes with no grid restrictions
- **Free Rotation** — Rotate objects on all three axes (pitch, yaw, roll) with optional angle snapping
- **Free Scaling** — Scale objects up or down uniformly or per-axis, with configurable min/max limits
- **Elevation Control** — Raise or lower objects with fine scroll-wheel adjustments
- **Off-Lot Placement** — Place objects anywhere in the world, beyond normal lot boundaries

### Precision Controls
- **Coordinate Input** — Enter exact X, Y, Z values for mathematical precision
- **Grid Snapping** — Optional snap increments for movement, rotation, and scale
- **Axis Constraints** — Lock manipulation to a single axis (X/Y/Z) or plane (XY/XZ/YZ)
- **Snap to Terrain** — Automatically match ground height at any position

### User Interface
- **Draggable HUD Overlay** — Shows current mode, axis, object info, and transform values
- **3D Gizmo Visualizer** — Color-coded axis arrows, rotation rings, and scale handles
- **Customizable Colors** — Change gizmo axis colors to your preference
- **Stay Open Mode** — Keep the dialog open and repeat the last command automatically

### Quality of Life
- **Full Undo/Redo** — Up to 100 levels of undo history (configurable)
- **Camera Follow** — Optionally snap the camera to follow the selected object
- **Reset to Original** — Restore any object to its original transform with one key
- **Persistent Settings** — All preferences saved to JSON and restored on load

### Extensibility
- **Lua Scripting API** — Full TOOL API exposed to Lua for custom scripts and automation
- **Macro System** — Record, save, and replay sequences of manipulation operations
- **Presets** — Built-in presets for common operations (face north/south, reset, nudge, stack)

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
| `LMB` | Select object under cursor |
| `Mouse Drag` | Manipulate along current axis |
| `Scroll Wheel` | Fine adjust (elevate/scale/rotate) |
| `N` | Open numeric coordinate input |
| `T` | Snap to terrain |
| `C` | Toggle camera follow |
| `Ctrl+Z` | Undo |
| `Ctrl+Y` | Redo |
| `Delete` | Reset object to original transform |
| `Escape` | Deselect / Cancel |

## Installation

1. Download the latest release
2. Extract the `InZoiTOOL` folder into your inZOI mods directory:
   ```
   <inZOI Install>/Game/Plugins/InZoiTOOL/
   ```
3. Launch inZOI — the mod loads automatically
4. Press `F2` in-game to activate the TOOL overlay

### Via CurseForge
The mod can also be installed through CurseForge's in-game mod browser (when published).

## Configuration

Edit `Config/tool_settings.json` to customize all settings, or modify them in-game through the settings panel. Changes persist between sessions.

Key settings:
- `movement.gridSnap` — Set to `0` for free movement, or a value like `0.25` for grid snapping
- `movement.allowOffLot` — Enable/disable off-lot placement
- `scale.minScale` / `scale.maxScale` — Limit how small/large objects can be scaled
- `visuals.*Color` — Customize gizmo axis colors (RGBA, 0.0–1.0)

## Lua Scripting

Place `.lua` scripts in the `Scripts/` directory. They auto-load when TOOL initializes.

```lua
-- Example: Arrange objects in a circle
TOOL.setMode("move")
for i = 1, 12 do
    local angle = (i / 12) * 2 * math.pi
    local x = math.cos(angle) * 5
    local y = math.sin(angle) * 5
    TOOL.moveTo(x, y, 0)
    TOOL.snapTerrain()
end

-- Example: Record a macro
TOOL.startMacro("spin45")
TOOL.rotate(0, 45, 0)
TOOL.stopMacro()
-- Later: TOOL.playMacro("spin45")
```

## Project Structure

```
InZoiTOOL/
├── InZoiTOOL.uplugin              # Unreal Engine plugin descriptor
├── Source/InZoiTOOL/
│   ├── InZoiTOOL.Build.cs         # Build configuration
│   ├── Public/
│   │   ├── InZoiTOOLModule.h      # Plugin module interface
│   │   ├── ObjectManipulator.h    # Core move/rotate/scale/elevate logic
│   │   ├── TOOLInputHandler.h     # Keyboard/mouse input bindings
│   │   ├── TOOLWidget.h           # HUD overlay widget
│   │   ├── TOOLGizmoRenderer.h    # 3D axis gizmo visualization
│   │   ├── TOOLSettings.h         # Persistent configuration
│   │   └── TOOLLuaBridge.h        # Lua scripting interface
│   └── Private/
│       ├── InZoiTOOLModule.cpp
│       ├── ObjectManipulator.cpp
│       ├── TOOLInputHandler.cpp
│       ├── TOOLWidget.cpp
│       ├── TOOLGizmoRenderer.cpp
│       ├── TOOLSettings.cpp
│       └── TOOLLuaBridge.cpp
├── Scripts/
│   └── tool_main.lua              # Main Lua API and utilities
├── Config/
│   ├── tool_settings.json         # User-editable settings
│   └── DefaultTOOL.ini            # UE config defaults
└── Content/UI/Textures/           # UI assets (widget blueprints)
```

## Requirements

- inZOI (Early Access or later)
- inZOI ModKit (for building from source)
- Unreal Engine 5.4+ (matching inZOI's engine version)

## Building from Source

1. Install the [inZOI ModKit](https://store.epicgames.com/) from the Epic Games Store
2. Clone this repository into your ModKit's Plugins directory
3. Open the ModKit project and build the `InZoiTOOL` plugin
4. Package the mod through the ModKit's CurseForge export wizard

## License

This project is open source. Free to use, modify, and distribute for non-commercial purposes.

## Credits

- Inspired by [TwistedMexi's T.O.O.L. mod](https://www.curseforge.com/sims4/mods/t-o-o-l) for The Sims 4
- Built for the [inZOI](https://playinzoi.com/) modding ecosystem by KRAFTON
