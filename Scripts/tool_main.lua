-- ============================================================================
-- InZoi T.O.O.L. - Main Lua Script
-- Takes Objects Off Lot - Advanced Object Manipulation for inZOI
-- ============================================================================
--
-- This script provides the Lua API layer for the T.O.O.L. mod.
-- It wraps the native C++ TOOL functions and adds convenience utilities
-- for modders to create custom manipulation workflows.
--
-- Usage:
--   Place custom .lua scripts in the Scripts/ directory.
--   They will be auto-loaded when TOOL initializes.
--
-- API Reference:
--   TOOL.select(name)              Select object by name
--   TOOL.deselect()                Deselect current object
--   TOOL.move(x, y, z)            Move by delta
--   TOOL.moveTo(x, y, z)          Move to absolute position
--   TOOL.rotate(p, y, r)          Rotate by delta (degrees)
--   TOOL.rotateTo(p, y, r)        Set absolute rotation
--   TOOL.scale(factor)             Uniform scale multiplier
--   TOOL.scaleTo(x, y, z)         Set absolute scale
--   TOOL.elevate(delta)            Raise/lower by amount
--   TOOL.elevateTo(height)         Set absolute height
--   TOOL.snapTerrain()             Snap to ground
--   TOOL.undo()                    Undo last action
--   TOOL.redo()                    Redo last undone action
--   TOOL.setMode(mode)             "move"|"rotate"|"scale"|"elevate"
--   TOOL.setAxis(axis)             "x"|"y"|"z"|"xy"|"xz"|"yz"|"free"
--   TOOL.getPosition()             Returns x, y, z
--   TOOL.getRotation()             Returns pitch, yaw, roll
--   TOOL.getScale()                Returns x, y, z
--   TOOL.getSelectedName()         Returns name or nil
--   TOOL.log(msg)                  Print to TOOL log
-- ============================================================================

TOOL = TOOL or {}

-- ============================================================================
-- Utility Functions
-- ============================================================================

--- Log a message with TOOL prefix
---@param msg string
function TOOL.log(msg)
    print("[InZoi TOOL] " .. tostring(msg))
end

--- Check if an object is currently selected
---@return boolean
function TOOL.hasSelection()
    return TOOL.getSelectedName() ~= nil
end

-- ============================================================================
-- Macro System - Record and replay sequences of operations
-- ============================================================================

local macros = {}
local recording = false
local currentMacroName = nil
local currentMacroSteps = {}

--- Start recording a macro
---@param name string  Name for this macro
function TOOL.startMacro(name)
    if recording then
        TOOL.log("Already recording macro: " .. currentMacroName)
        return
    end
    recording = true
    currentMacroName = name
    currentMacroSteps = {}
    TOOL.log("Recording macro: " .. name)
end

--- Stop recording and save the macro
function TOOL.stopMacro()
    if not recording then
        TOOL.log("Not currently recording")
        return
    end
    macros[currentMacroName] = currentMacroSteps
    TOOL.log("Saved macro '" .. currentMacroName .. "' with " ..
             #currentMacroSteps .. " steps")
    recording = false
    currentMacroName = nil
    currentMacroSteps = {}
end

--- Play back a recorded macro
---@param name string  Name of the macro to play
function TOOL.playMacro(name)
    local steps = macros[name]
    if not steps then
        TOOL.log("Macro not found: " .. name)
        return
    end

    TOOL.log("Playing macro: " .. name .. " (" .. #steps .. " steps)")
    for i, step in ipairs(steps) do
        step.func(table.unpack(step.args))
    end
    TOOL.log("Macro complete: " .. name)
end

--- List all recorded macros
---@return table  Array of macro names
function TOOL.listMacros()
    local names = {}
    for name, steps in pairs(macros) do
        table.insert(names, name .. " (" .. #steps .. " steps)")
    end
    return names
end

-- Internal: record a step if recording is active
local function recordStep(func, ...)
    if recording then
        table.insert(currentMacroSteps, {
            func = func,
            args = {...}
        })
    end
end

-- ============================================================================
-- Wrapped API (with macro recording support)
-- ============================================================================

-- Store original native functions (will be set by C++ bridge)
local native = {
    move = TOOL.move,
    moveTo = TOOL.moveTo,
    rotate = TOOL.rotate,
    rotateTo = TOOL.rotateTo,
    scale = TOOL.scale,
    scaleTo = TOOL.scaleTo,
    elevate = TOOL.elevate,
    elevateTo = TOOL.elevateTo,
    snapTerrain = TOOL.snapTerrain,
    setMode = TOOL.setMode,
    setAxis = TOOL.setAxis,
}

-- Wrap native functions with macro recording
if native.move then
    local origMove = native.move
    TOOL.move = function(x, y, z)
        recordStep(origMove, x, y, z)
        return origMove(x, y, z)
    end
end

if native.rotate then
    local origRotate = native.rotate
    TOOL.rotate = function(p, y, r)
        recordStep(origRotate, p, y, r)
        return origRotate(p, y, r)
    end
end

if native.scale then
    local origScale = native.scale
    TOOL.scale = function(factor)
        recordStep(origScale, factor)
        return origScale(factor)
    end
end

if native.elevate then
    local origElevate = native.elevate
    TOOL.elevate = function(delta)
        recordStep(origElevate, delta)
        return origElevate(delta)
    end
end

-- ============================================================================
-- Presets - Common manipulation presets
-- ============================================================================

TOOL.presets = {}

--- Rotate object to face north (yaw = 0)
function TOOL.presets.faceNorth()
    if not TOOL.hasSelection() then return end
    local _, _, roll = TOOL.getRotation()
    TOOL.rotateTo(0, 0, roll)
    TOOL.log("Facing north")
end

--- Rotate object to face south (yaw = 180)
function TOOL.presets.faceSouth()
    if not TOOL.hasSelection() then return end
    local _, _, roll = TOOL.getRotation()
    TOOL.rotateTo(0, 180, roll)
    TOOL.log("Facing south")
end

--- Reset object to default transform (scale 1, rotation 0)
function TOOL.presets.resetTransform()
    if not TOOL.hasSelection() then return end
    TOOL.rotateTo(0, 0, 0)
    TOOL.scaleTo(1, 1, 1)
    TOOL.snapTerrain()
    TOOL.log("Transform reset to default")
end

--- Stack an object on top of another by elevating to match
---@param heightOffset number  Additional offset above the ground
function TOOL.presets.stackAbove(heightOffset)
    if not TOOL.hasSelection() then return end
    heightOffset = heightOffset or 0
    TOOL.snapTerrain()
    TOOL.elevate(heightOffset)
    TOOL.log("Stacked with offset: " .. heightOffset)
end

--- Nudge object by small increments in the current mode
---@param amount number  Nudge amount (default 0.5)
---@param direction string  "up"|"down"|"left"|"right"|"forward"|"back"
function TOOL.presets.nudge(amount, direction)
    if not TOOL.hasSelection() then return end
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
    if d then
        TOOL.move(d[1], d[2], d[3])
        TOOL.log("Nudged " .. direction .. " by " .. amount)
    end
end

-- ============================================================================
-- Initialization
-- ============================================================================

TOOL.log("T.O.O.L. Lua API loaded successfully")
TOOL.log("Type TOOL.log('hello') to test the API")
TOOL.log("Macro system ready - use TOOL.startMacro('name') to begin recording")
