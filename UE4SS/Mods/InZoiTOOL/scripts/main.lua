-- ============================================================================
-- InZoi T.O.O.L. - Takes Objects Off Lot
-- UE4SS Lua Mod - Main Entry Point
--
-- Advanced object manipulation for inZOI: freely move, rotate, scale, and
-- elevate any in-game object with precision controls.
--
-- Requires: UE4SS (https://github.com/UE4SS-RE/RE-UE4SS)
--           inZOI Mod Enabler (Nexus Mods)
--
-- Optional: InZoiTOOL_GUI C++ companion mod for ImGui visual overlay
-- ============================================================================

local ModName = "InZoiTOOL"
print(string.format("[%s] Loading T.O.O.L. - Takes Objects Off Lot...\n", ModName))

-- Resolve mod directory for file I/O.
-- Some UE4SS builds expose ModRef.GetModPath as a non-function userdata; the
-- naive `if ModRef.GetModPath then ... ModRef:GetModPath() end` guard passes
-- the truthy check and then errors with "attempt to call a ModRef value",
-- which aborts the whole mod. Gate on type() and wrap the call in pcall so
-- we always fall through to the relative path on builds that don't expose it.
local modDir = nil
if ModRef and type(ModRef.GetModPath) == "function" then
    local ok, path = pcall(function() return ModRef:GetModPath() end)
    if ok and type(path) == "string" and path ~= "" then
        modDir = path
    end
end
if not modDir then
    modDir = ".\\ue4ss\\Mods\\" .. ModName .. "\\scripts"
end

-- Load modules
local Settings = require("settings")
local Manipulator = require("manipulator")
local Macros = require("macros")
local UI = require("ui")
local Overlay = require("overlay")

-- Initialize settings
Settings.init(modDir)

-- Wire up module references
UI.manipulator = Manipulator
UI.macros = Macros
Overlay.manipulator = Manipulator
Manipulator.macros = Macros

-- ============================================================================
-- Tool State
-- ============================================================================

local toolActive = false

local overlayAvailable = false

local function toggleTool()
    toolActive = not toolActive
    UI.visible = toolActive
    if toolActive then
        print("[InZoi TOOL] === ACTIVATED ===\n")
        print("[InZoi TOOL] Mode: MOVE | Axis: FREE\n")
        print("[InZoi TOOL] Use TOOL.selectByClass('ClassName') to select an object\n")
        print("[InZoi TOOL] Or TOOL.browseActors('Actor') to browse\n")
        print("[InZoi TOOL] Type TOOL.help() for full command list\n")
        UI.setStatus("T.O.O.L. activated")

        -- Try to show in-game UMG overlay
        if Overlay.show() then
            overlayAvailable = true
            Overlay.setStatus("T.O.O.L. activated")
        end
    else
        print("[InZoi TOOL] === DEACTIVATED ===\n")
        UI.setStatus("T.O.O.L. deactivated")
        Overlay.hide()
    end
end

-- Helper: send status to both console and overlay
local function setStatus(msg)
    UI.setStatus(msg)
    if overlayAvailable then
        Overlay.setStatus(msg)
    end
end

-- ============================================================================
-- Keybindings (UE4SS RegisterKeyBind API)
-- ============================================================================

-- F2: Toggle tool
RegisterKeyBind(Key.F2, function()
    toggleTool()
end)

-- F3: Print status
RegisterKeyBind(Key.F3, function()
    if not toolActive then return end
    UI.printStatus()
end)

-- Mode keys (only when tool is active)
RegisterKeyBind(Key.G, function()
    if not toolActive then return end
    Manipulator.setMode("move")
    setStatus("Mode: MOVE")
end)

RegisterKeyBind(Key.R, function()
    if not toolActive then return end
    Manipulator.setMode("rotate")
    setStatus("Mode: ROTATE")
end)

RegisterKeyBind(Key.S, function()
    if not toolActive then return end
    Manipulator.setMode("scale")
    setStatus("Mode: SCALE")
end)

RegisterKeyBind(Key.E, function()
    if not toolActive then return end
    Manipulator.setMode("elevate")
    setStatus("Mode: ELEVATE")
end)

-- Axis keys
RegisterKeyBind(Key.X, function()
    if not toolActive then return end
    Manipulator.setAxis("x")
    setStatus("Axis: X")
end)

-- Plain Y/Z axis keys register an empty modifier list so UE4SS does NOT
-- dispatch them when Ctrl is held (the Ctrl+Y / Ctrl+Z bindings below
-- handle undo/redo).
RegisterKeyBind(Key.Y, {}, function()
    if not toolActive then return end
    Manipulator.setAxis("y")
    setStatus("Axis: Y")
end)

RegisterKeyBind(Key.Z, {}, function()
    if not toolActive then return end
    Manipulator.setAxis("z")
    setStatus("Axis: Z")
end)

RegisterKeyBind(Key.TAB, function()
    if not toolActive then return end
    Manipulator.cycleAxis()
    setStatus("Axis: " .. Manipulator.getAxisDisplayName())
end)

-- Undo/Redo (with modifier keys)
RegisterKeyBind(Key.Z, {ModifierKey.CONTROL}, function()
    if not toolActive then return end
    Manipulator.undo()
    setStatus("Undo")
end)

RegisterKeyBind(Key.Y, {ModifierKey.CONTROL}, function()
    if not toolActive then return end
    Manipulator.redo()
    setStatus("Redo")
end)

-- Reset transform (UE4SS names this key DEL, not DELETE)
RegisterKeyBind(Key.DEL, function()
    if not toolActive then return end
    Manipulator.resetTransform()
    setStatus("Transform reset")
end)

-- Deselect (or close the in-overlay browser if it's open)
RegisterKeyBind(Key.ESCAPE, function()
    if not toolActive then return end
    if Overlay.browserVisible then
        Overlay.closeBrowser()
        setStatus("Browse canceled")
        return
    end
    Manipulator.deselect()
    setStatus("Deselected")
end)

-- Browser: F4 = StaticMeshActor list, F5 = any Actor list.
-- While the browser is open, arrows navigate the list (see arrow-key
-- handlers below) and Enter confirms.
RegisterKeyBind(Key.F4, function()
    if not toolActive then return end
    if Overlay.openBrowser then
        Overlay.openBrowser("StaticMeshActor", 50)
    end
end)

RegisterKeyBind(Key.F5, function()
    if not toolActive then return end
    if Overlay.openBrowser then
        Overlay.openBrowser("Actor", 50)
    end
end)

-- Enter confirms browser selection when the list is open.
RegisterKeyBind(Key.RETURN, function()
    if not toolActive then return end
    if Overlay.browserVisible and Overlay.browserConfirm then
        Overlay.browserConfirm()
        setStatus("Selected via browser")
    end
end)

-- Arrow keys: browser navigation when list is open, otherwise nudge
RegisterKeyBind(Key.UP_ARROW, function()
    if not toolActive then return end
    if Overlay.browserVisible then
        Overlay.browserMove(-1); return
    end
    if not Manipulator.hasSelection() then return end
    local speed = Settings.get("moveSpeed") or 1.0
    if Manipulator.currentMode == "move" then
        Manipulator.move(speed, 0, 0)
    elseif Manipulator.currentMode == "rotate" then
        Manipulator.rotateByAngle(5.0)
    elseif Manipulator.currentMode == "elevate" then
        Manipulator.elevate(Settings.get("elevationStep") or 0.5)
    elseif Manipulator.currentMode == "scale" then
        Manipulator.scaleUniform(1.05)
    end
end)

RegisterKeyBind(Key.DOWN_ARROW, function()
    if not toolActive then return end
    if Overlay.browserVisible then
        Overlay.browserMove(1); return
    end
    if not Manipulator.hasSelection() then return end
    local speed = Settings.get("moveSpeed") or 1.0
    if Manipulator.currentMode == "move" then
        Manipulator.move(-speed, 0, 0)
    elseif Manipulator.currentMode == "rotate" then
        Manipulator.rotateByAngle(-5.0)
    elseif Manipulator.currentMode == "elevate" then
        Manipulator.elevate(-(Settings.get("elevationStep") or 0.5))
    elseif Manipulator.currentMode == "scale" then
        Manipulator.scaleUniform(0.95)
    end
end)

RegisterKeyBind(Key.LEFT_ARROW, function()
    if not toolActive then return end
    if Overlay.browserVisible then
        -- Left arrow in browser jumps to the first entry
        Overlay.browserMove(-(#Overlay.browserItems + 1)); return
    end
    if not Manipulator.hasSelection() then return end
    local speed = Settings.get("moveSpeed") or 1.0
    if Manipulator.currentMode == "move" then
        Manipulator.move(0, -speed, 0)
    elseif Manipulator.currentMode == "rotate" then
        Manipulator.rotateByAngle(-15.0)
    end
end)

RegisterKeyBind(Key.RIGHT_ARROW, function()
    if not toolActive then return end
    if Overlay.browserVisible then
        -- Right arrow in browser jumps to the last entry
        Overlay.browserMove(#Overlay.browserItems + 1); return
    end
    if not Manipulator.hasSelection() then return end
    local speed = Settings.get("moveSpeed") or 1.0
    if Manipulator.currentMode == "move" then
        Manipulator.move(0, speed, 0)
    elseif Manipulator.currentMode == "rotate" then
        Manipulator.rotateByAngle(15.0)
    end
end)

-- Page Up / Page Down: browser page-jump when list is open, else elevation
RegisterKeyBind(Key.PAGE_UP, function()
    if not toolActive then return end
    if Overlay.browserVisible then
        Overlay.browserMove(-(Overlay.browserPageSize or 10)); return
    end
    if not Manipulator.hasSelection() then return end
    Manipulator.elevate(Settings.get("elevationStep") or 0.5)
end)

RegisterKeyBind(Key.PAGE_DOWN, function()
    if not toolActive then return end
    if Overlay.browserVisible then
        Overlay.browserMove(Overlay.browserPageSize or 10); return
    end
    if not Manipulator.hasSelection() then return end
    Manipulator.elevate(-(Settings.get("elevationStep") or 0.5))
end)

-- ============================================================================
-- Periodic sync: push state to shared variables for C++ GUI companion
-- Also poll for commands from the GUI
-- ============================================================================

LoopAsync(200, function()
    -- Sync state for C++ ImGui companion (shared variables)
    UI.syncSharedState(toolActive)

    -- Poll for commands from C++ GUI companion
    local cmd = UI.pollCommands()
    if cmd == "toggle" then
        toggleTool()
    end

    -- Update in-game UMG overlay (if active and created)
    if toolActive and overlayAvailable then
        ExecuteInGameThread(function()
            Overlay.update()
        end)
    end

    return false -- keep looping (return true to stop)
end)

-- ============================================================================
-- Global TOOL API (for UE4SS console and other mods)
-- ============================================================================

TOOL = {}

TOOL.toggle = toggleTool
TOOL.isActive = function() return toolActive end
TOOL.status = function() UI.printStatus() end
TOOL.help = function() UI.printHelp() end

-- Selection
TOOL.selectByClass = function(className)
    local actor = FindFirstOf(className)
    if actor and actor:IsValid() then
        Manipulator.selectActor(actor)
        UI.printStatus()
        return true
    end
    print("[InZoi TOOL] No instance of '" .. className .. "' found\n")
    return false
end

TOOL.browseActors = function(className, maxResults)
    UI.browseActors(className, maxResults)
end

TOOL.selectFromBrowse = function(index)
    return UI.selectFromBrowse(index)
end

TOOL.deselect = function() Manipulator.deselect() end
TOOL.hasSelection = function() return Manipulator.hasSelection() end
TOOL.getSelectedName = function() return Manipulator.selectedActorName end

-- Manipulation
TOOL.move = function(x, y, z) Manipulator.move(x or 0, y or 0, z or 0) end
TOOL.moveTo = function(x, y, z) Manipulator.moveTo(x, y, z) end
TOOL.rotate = function(p, y, r) Manipulator.rotate(p or 0, y or 0, r or 0) end
TOOL.rotateTo = function(p, y, r) Manipulator.rotateTo(p, y, r) end
TOOL.scale = function(f) Manipulator.scaleUniform(f) end
TOOL.scaleTo = function(x, y, z) Manipulator.scaleTo(x, y, z) end
TOOL.elevate = function(d) Manipulator.elevate(d) end
TOOL.elevateTo = function(h) Manipulator.elevateTo(h) end
TOOL.reset = function() Manipulator.resetTransform() end
TOOL.undo = function() Manipulator.undo() end
TOOL.redo = function() Manipulator.redo() end

-- Mode/Axis
TOOL.setMode = function(m) Manipulator.setMode(m) end
TOOL.setAxis = function(a) Manipulator.setAxis(a) end

-- Transform getters
TOOL.getPosition = function() return Manipulator.getPosition() end
TOOL.getRotation = function() return Manipulator.getRotation() end
TOOL.getScale = function() return Manipulator.getScale() end

-- Macros
TOOL.startMacro = function(name) return Macros.startRecording(name) end
TOOL.stopMacro = function() return Macros.stopRecording() end
TOOL.playMacro = function(name) return Macros.play(name, Manipulator) end
TOOL.listMacros = function() return Macros.list() end

-- Presets
TOOL.presets = {}
TOOL.presets.faceNorth = function()
    if not Manipulator.hasSelection() then return end
    local rot = Manipulator.getRotation()
    Manipulator.rotateTo(0, 0, rot and rot.roll or 0)
end
TOOL.presets.faceSouth = function()
    if not Manipulator.hasSelection() then return end
    local rot = Manipulator.getRotation()
    Manipulator.rotateTo(0, 180, rot and rot.roll or 0)
end
TOOL.presets.resetTransform = function()
    Manipulator.resetTransform()
end
TOOL.presets.nudge = function(amount, direction)
    if not Manipulator.hasSelection() then return end
    amount = amount or 0.5
    direction = direction or "up"
    local deltas = {
        up      = {0, 0, amount},
        down    = {0, 0, -amount},
        left    = {-amount, 0, 0},
        right   = {amount, 0, 0},
        forward = {0, amount, 0},
        back    = {0, -amount, 0},
    }
    local d = deltas[direction]
    if d then Manipulator.move(d[1], d[2], d[3]) end
end

TOOL.log = function(msg)
    print("[InZoi TOOL] " .. tostring(msg) .. "\n")
end

-- ============================================================================
-- In-game console command: `tool <subcommand> [args...]`
--
-- Exposes TOOL.* to inZOI's own console (opened with `~`, `/`, or F10 via
-- ConsoleEnablerMod). Each subcommand is dispatched below; output from
-- inner `print` calls is captured and routed back into the in-game
-- console's FOutputDevice so users see results where they typed.
--
-- UE4SS ships RegisterConsoleCommandHandler on most builds; the guard
-- keeps the mod working on builds that lack it.
-- ============================================================================

if RegisterConsoleCommandHandler then
    local function num(s, default)
        local n = tonumber(s)
        if n == nil then return default end
        return n
    end

    local function emit(ar, line)
        -- FOutputDevice.Log is the usual UE4SS binding; fall back to
        -- `print` (goes to UE4SS log) if the method isn't available on
        -- this build.
        local ok = pcall(function() ar:Log(line) end)
        if not ok then print(line .. "\n") end
    end

    local function captureOutput(fn)
        local buf = {}
        local originalPrint = print
        print = function(...)
            local parts = {}
            for i = 1, select("#", ...) do
                parts[#parts + 1] = tostring((select(i, ...)))
            end
            table.insert(buf, (table.concat(parts, "\t"):gsub("\n$", "")))
        end
        local ok, err = pcall(fn)
        print = originalPrint
        return ok, err, buf
    end

    local dispatch = {}
    dispatch.help = function() UI.printHelp() end
    dispatch.status = function() UI.printStatus() end
    dispatch.toggle = function() toggleTool() end
    dispatch.browse = function(a)
        UI.browseActors(a[2] or "Actor", num(a[3], 20))
    end
    dispatch.select = function(a)
        UI.selectFromBrowse(num(a[2], 1))
    end
    dispatch.selectclass = function(a)
        TOOL.selectByClass(a[2] or "Actor")
    end
    dispatch.deselect = function() Manipulator.deselect() end
    dispatch.move = function(a)
        Manipulator.move(num(a[2], 0), num(a[3], 0), num(a[4], 0))
    end
    dispatch.moveto = function(a)
        Manipulator.moveTo(num(a[2], 0), num(a[3], 0), num(a[4], 0))
    end
    dispatch.rotate = function(a)
        Manipulator.rotate(num(a[2], 0), num(a[3], 0), num(a[4], 0))
    end
    dispatch.rotateto = function(a)
        Manipulator.rotateTo(num(a[2], 0), num(a[3], 0), num(a[4], 0))
    end
    dispatch.scale = function(a) Manipulator.scaleUniform(num(a[2], 1)) end
    dispatch.scaleto = function(a)
        Manipulator.scaleTo(num(a[2], 1), num(a[3], 1), num(a[4], 1))
    end
    dispatch.elevate = function(a) Manipulator.elevate(num(a[2], 0)) end
    dispatch.elevateto = function(a) Manipulator.elevateTo(num(a[2], 0)) end
    dispatch.reset = function() Manipulator.resetTransform() end
    dispatch.undo = function() Manipulator.undo() end
    dispatch.redo = function() Manipulator.redo() end
    dispatch.mode = function(a) Manipulator.setMode(a[2] or "move") end
    dispatch.axis = function(a) Manipulator.setAxis(a[2] or "free") end
    dispatch.pos = function()
        local p = Manipulator.getPosition()
        if p then
            print(string.format("Pos: X=%.2f Y=%.2f Z=%.2f", p.x, p.y, p.z))
        else
            print("No selection")
        end
    end
    dispatch.rot = function()
        local r = Manipulator.getRotation()
        if r then
            print(string.format("Rot: P=%.1f Y=%.1f R=%.1f",
                r.pitch, r.yaw, r.roll))
        else
            print("No selection")
        end
    end
    dispatch.sc = function()
        local s = Manipulator.getScale()
        if s then
            print(string.format("Scale: X=%.2f Y=%.2f Z=%.2f", s.x, s.y, s.z))
        else
            print("No selection")
        end
    end

    RegisterConsoleCommandHandler("tool", function(full, args, ar)
        local sub = args and args[1]
        if not sub or sub == "" then
            emit(ar, "Usage: tool <subcommand> [args]   (try: tool help)")
            return true
        end

        local fn = dispatch[string.lower(sub)]
        if not fn then
            emit(ar, "Unknown subcommand '" .. tostring(sub)
                .. "'. Try: tool help")
            return true
        end

        local ok, err, buf = captureOutput(function() fn(args) end)
        for _, line in ipairs(buf) do emit(ar, line) end
        if not ok then emit(ar, "ERROR: " .. tostring(err)) end
        if ok and #buf == 0 then emit(ar, "OK") end
        return true
    end)

    print(string.format("[%s] In-game console command 'tool' registered.\n",
        ModName))
end

-- ============================================================================
-- Done
-- ============================================================================

print(string.format("[%s] T.O.O.L. loaded successfully!\n", ModName))
print(string.format("[%s] Press F2 to activate, F3 for status.\n", ModName))
print(string.format("[%s] Type TOOL.help() for full command list.\n", ModName))
print(string.format("[%s] GUI: Install InZoiTOOL_GUI companion mod for visual overlay.\n", ModName))
