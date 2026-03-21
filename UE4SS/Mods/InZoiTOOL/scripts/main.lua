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

-- Resolve mod directory for file I/O
local modDir = nil
if ModRef and ModRef.GetModPath then
    modDir = ModRef:GetModPath()
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

RegisterKeyBind(Key.Y, function()
    if not toolActive then return end
    Manipulator.setAxis("y")
    setStatus("Axis: Y")
end)

RegisterKeyBind(Key.Z, function()
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

-- Reset transform
RegisterKeyBind(Key.DELETE, function()
    if not toolActive then return end
    Manipulator.resetTransform()
    setStatus("Transform reset")
end)

-- Deselect
RegisterKeyBind(Key.ESCAPE, function()
    if not toolActive then return end
    Manipulator.deselect()
    setStatus("Deselected")
end)

-- Arrow keys for nudging
RegisterKeyBind(Key.UP_ARROW, function()
    if not toolActive or not Manipulator.hasSelection() then return end
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
    if not toolActive or not Manipulator.hasSelection() then return end
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
    if not toolActive or not Manipulator.hasSelection() then return end
    local speed = Settings.get("moveSpeed") or 1.0
    if Manipulator.currentMode == "move" then
        Manipulator.move(0, -speed, 0)
    elseif Manipulator.currentMode == "rotate" then
        Manipulator.rotateByAngle(-15.0)
    end
end)

RegisterKeyBind(Key.RIGHT_ARROW, function()
    if not toolActive or not Manipulator.hasSelection() then return end
    local speed = Settings.get("moveSpeed") or 1.0
    if Manipulator.currentMode == "move" then
        Manipulator.move(0, speed, 0)
    elseif Manipulator.currentMode == "rotate" then
        Manipulator.rotateByAngle(15.0)
    end
end)

-- Page Up / Page Down for elevation
RegisterKeyBind(Key.PAGE_UP, function()
    if not toolActive or not Manipulator.hasSelection() then return end
    Manipulator.elevate(Settings.get("elevationStep") or 0.5)
end)

RegisterKeyBind(Key.PAGE_DOWN, function()
    if not toolActive or not Manipulator.hasSelection() then return end
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
-- Done
-- ============================================================================

print(string.format("[%s] T.O.O.L. loaded successfully!\n", ModName))
print(string.format("[%s] Press F2 to activate, F3 for status.\n", ModName))
print(string.format("[%s] Type TOOL.help() for full command list.\n", ModName))
print(string.format("[%s] GUI: Install InZoiTOOL_GUI companion mod for visual overlay.\n", ModName))
