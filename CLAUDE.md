# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

A UE4SS Lua mod for the game **inZOI** that allows free move/rotate/scale/elevate of any in-game actor, inspired by TwistedMexi's Sims 4 T.O.O.L. mod. UE4SS is the only currently supported code-mod runtime for inZOI, so this repo targets its Lua API (not a standalone Unreal project). An optional C++ companion (`InZoiTOOL_GUI`) adds an ImGui tab to the UE4SS debug window.

## Build / run / test

There is no build pipeline, test suite, or linter for the Lua mod — it is just source files that get copied into the player's UE4SS `Mods/` folder. To "run" it, copy `UE4SS/Mods/InZoiTOOL/` into `<inZOI>/BlueClient/Binaries/Win64/ue4ss/Mods/` and ensure `mods.txt` contains `InZoiTOOL : 1`. Launch inZOI, press **F2** in-game, use the UE4SS console (`~`) for the `TOOL.*` API.

The C++ companion (`UE4SS/Mods/InZoiTOOL_GUI/dlls/main.cpp`) builds against the UE4SS C++ mod SDK via CMake:

```
cmake -DUE4SS_PATH=<path-to-RE-UE4SS> ..
cmake --build .
```

Output `main.dll` lives next to `CMakeLists.txt` in `dlls/`. The companion is optional — the Lua mod is fully functional without it.

## Architecture

The Lua mod is split into modules loaded by `main.lua` via `require(...)`. Key wiring: `main.lua` holds the `toolActive` flag and the full `TOOL` global API, and injects cross-module references (`UI.manipulator = Manipulator`, `Overlay.manipulator = Manipulator`) — modules do not require each other directly for this dependency to avoid cycles.

- **`main.lua`** — entry point. Registers every `RegisterKeyBind`, builds the global `TOOL` table (console API), and runs a single `LoopAsync(200, ...)` tick that (a) pushes state to shared variables for the C++ GUI, (b) polls the `TOOL_Command` shared variable for GUI → Lua commands, and (c) updates the UMG overlay inside `ExecuteInGameThread`. All tick-based work goes through this one loop.
- **`manipulator.lua`** — all transform logic. Wraps UE4SS reflection calls (`K2_GetActorLocation`/`K2_SetActorLocation`, `K2_GetActorRotation`/`K2_SetActorRotation`, `GetActorScale3D`/`SetActorScale3D`). Owns `undoStack`/`redoStack` and the `originalTransform` snapshot used by reset. **Every mutation must call `Manipulator.pushUndo()` before applying the change** — this is the invariant that makes Ctrl+Z work.
- **`ui.lua`** — dual-purpose: console output (`print`, `printStatus`, `printHelp`) and **shared-variable bridge** to the C++ GUI. `syncSharedState` writes `TOOL_*` keys; `pollCommands` reads `TOOL_Command` and dispatches. It also owns the console actor browser (`lastBrowseResults`).
- **`overlay.lua`** — builds a real in-game HUD via UMG reflection from Lua (`StaticConstructObject` on `/Script/UMG.*` classes, then `AddToViewport`). No C++ required. Based on the pattern from joric/SupraTools (see [UE4SS issue #1072](https://github.com/UE4SS-RE/RE-UE4SS/issues/1072)). Widget creation needs a `GameInstance` outer, so `Overlay.create()` can legitimately return `false` early and should be retried — don't assume it succeeds the first time.
- **`settings.lua`** — persistent settings as JSON in `tool_settings.json`. Hand-rolled JSON parser/serializer (no dependency); it handles flat objects, numbers, strings, booleans, and number arrays only. Do not nest objects in the settings file.
- **`macros.lua`** — in-memory record/replay of `TOOL.*` calls. Macros are not persisted across sessions.

### The Lua ↔ C++ contract

The C++ companion and Lua mod do not call each other directly. They communicate only through **UE4SS shared variables** (`ModRef:SetSharedVariable` / `ModRef:GetSharedVariable`):

- Lua writes read-only display state under `TOOL_Active`, `TOOL_Mode`, `TOOL_Axis`, `TOOL_SelectedName`, `TOOL_Pos{X,Y,Z}`, `TOOL_Rot{P,Y,R}`, `TOOL_Scale{X,Y,Z}`, `TOOL_UndoCount`, `TOOL_RedoCount`, `TOOL_Status`.
- The C++ side writes commands into the single `TOOL_Command` key, which Lua reads and clears each tick. Command formats: bare strings (`"toggle"`, `"undo"`, `"redo"`, `"reset"`, `"deselect"`) and prefixed strings (`"mode:<name>"`, `"axis:<name>"`, `"browse:<className>"`, `"input:<text>"`).

Both sides must agree on these key names. The C++ side lists them as `SV_*` constants near the top of `main.cpp`; the Lua side uses the literal strings in `ui.lua`. When adding a new field, update both files.

### Why the C++ side registers in `on_ui_init`

`register_tab` is called from `on_ui_init`, not `on_program_start` — this avoids a documented race-condition crash ([UE4SS issue #481](https://github.com/UE4SS-RE/RE-UE4SS/issues/481)). Keep it there.

## Conventions and gotchas

- **File paths use backslashes** in `require`/file I/O because UE4SS runs on Windows; don't change these to forward slashes when editing from Linux/WSL.
- **`print` calls include trailing `\n`** — UE4SS's console does not append newlines automatically.
- **Axis constraint logic is duplicated** inside `Manipulator.move`/`Manipulator.rotate`. If you change the constraint semantics, update both.
- **Mode/Axis string values are the contract** shared with C++: `"move"/"rotate"/"scale"/"elevate"` and `"free"/"x"/"y"/"z"/"xy"/"xz"/"yz"`. Changing any literal breaks the GUI.
- **`README.md` is user-facing documentation** — keep installation/keybinding/API sections accurate when changing behavior, as users follow it verbatim.
- The `.gitignore` excludes `*.dll`, so compiled companion output (`main.dll`) is intentionally not committed; users build it themselves.
