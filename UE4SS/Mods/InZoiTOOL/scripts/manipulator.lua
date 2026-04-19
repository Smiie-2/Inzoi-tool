-- ============================================================================
-- InZoi T.O.O.L. - Object Manipulator Module
-- Core move/rotate/scale/elevate logic using UE4SS actor APIs
-- ============================================================================

local Settings = require("settings")

local Manipulator = {}

-- Cross-module ref injected by main.lua (avoids circular require of macros.lua).
Manipulator.macros = nil

local function recordStep(funcName, ...)
    local m = Manipulator.macros
    if m and m.recordStep then m.recordStep(funcName, ...) end
end

-- State
Manipulator.selectedActor = nil
Manipulator.selectedActorName = ""
Manipulator.originalTransform = nil -- {loc={x,y,z}, rot={pitch,yaw,roll}, scale={x,y,z}}

-- Modes and axes
Manipulator.Mode = {
    MOVE = "move",
    ROTATE = "rotate",
    SCALE = "scale",
    ELEVATE = "elevate",
}

Manipulator.Axis = {
    FREE = "free",
    X = "x",
    Y = "y",
    Z = "z",
    XY = "xy",
    XZ = "xz",
    YZ = "yz",
}

Manipulator.currentMode = Manipulator.Mode.MOVE
Manipulator.currentAxis = Manipulator.Axis.FREE

-- Undo/Redo stacks: each entry is {actor, loc={x,y,z}, rot={pitch,yaw,roll}, scale={x,y,z}}
Manipulator.undoStack = {}
Manipulator.redoStack = {}

-- ============================================================================
-- Helpers: Get/Set transforms via UE4SS reflection
-- ============================================================================

local function getActorLocation(actor)
    if not actor or not actor:IsValid() then return nil end
    local vec = actor:K2_GetActorLocation()
    return {x = vec.X, y = vec.Y, z = vec.Z}
end

local function setActorLocation(actor, loc)
    if not actor or not actor:IsValid() then return end
    local vec = {X = loc.x, Y = loc.y, Z = loc.z}
    -- K2_SetActorLocation(NewLocation, bSweep, SweepHitResult, bTeleport)
    actor:K2_SetActorLocation(vec, false, {}, true)
end

local function getActorRotation(actor)
    if not actor or not actor:IsValid() then return nil end
    local rot = actor:K2_GetActorRotation()
    return {pitch = rot.Pitch, yaw = rot.Yaw, roll = rot.Roll}
end

local function setActorRotation(actor, rot)
    if not actor or not actor:IsValid() then return end
    local r = {Pitch = rot.pitch, Yaw = rot.yaw, Roll = rot.roll}
    actor:K2_SetActorRotation(r, true)
end

local function getActorScale(actor)
    if not actor or not actor:IsValid() then return nil end
    local s = actor:GetActorScale3D()
    return {x = s.X, y = s.Y, z = s.Z}
end

local function setActorScale(actor, scale)
    if not actor or not actor:IsValid() then return end
    local s = {X = scale.x, Y = scale.y, Z = scale.z}
    actor:SetActorScale3D(s)
end

-- ============================================================================
-- Snapping helpers
-- ============================================================================

local function gridSnap(value, increment)
    if increment <= 0 then return value end
    return math.floor(value / increment + 0.5) * increment
end

local function applyMoveSnap(dx, dy, dz)
    local snap = Settings.get("moveGridSnap")
    if snap > 0 then
        dx = gridSnap(dx, snap)
        dy = gridSnap(dy, snap)
        dz = gridSnap(dz, snap)
    end
    return dx, dy, dz
end

local function applyRotateSnap(dp, dy, dr)
    local snap = Settings.get("rotateGridSnap")
    if snap > 0 then
        dp = gridSnap(dp, snap)
        dy = gridSnap(dy, snap)
        dr = gridSnap(dr, snap)
    end
    return dp, dy, dr
end

-- ============================================================================
-- Undo/Redo
-- ============================================================================

local function captureState(actor)
    if not actor or not actor:IsValid() then return nil end
    return {
        actor = actor,
        loc = getActorLocation(actor),
        rot = getActorRotation(actor),
        scale = getActorScale(actor),
    }
end

local function restoreState(state)
    if not state or not state.actor or not state.actor:IsValid() then return end
    setActorLocation(state.actor, state.loc)
    setActorRotation(state.actor, state.rot)
    setActorScale(state.actor, state.scale)
end

function Manipulator.pushUndo()
    if not Manipulator.selectedActor then return end
    local state = captureState(Manipulator.selectedActor)
    if not state then return end

    table.insert(Manipulator.undoStack, state)

    -- Trim to max
    local max = Settings.get("maxUndoSteps") or 100
    while #Manipulator.undoStack > max do
        table.remove(Manipulator.undoStack, 1)
    end

    -- New action invalidates redo
    Manipulator.redoStack = {}
end

function Manipulator.undo()
    if #Manipulator.undoStack == 0 then return end

    local state = table.remove(Manipulator.undoStack)
    if state.actor and state.actor:IsValid() then
        -- Save current state to redo
        local current = captureState(state.actor)
        if current then
            table.insert(Manipulator.redoStack, current)
        end
        restoreState(state)
    end
end

function Manipulator.redo()
    if #Manipulator.redoStack == 0 then return end

    local state = table.remove(Manipulator.redoStack)
    if state.actor and state.actor:IsValid() then
        -- Save current state to undo
        local current = captureState(state.actor)
        if current then
            table.insert(Manipulator.undoStack, current)
        end
        restoreState(state)
    end
end

function Manipulator.canUndo()
    return #Manipulator.undoStack > 0
end

function Manipulator.canRedo()
    return #Manipulator.redoStack > 0
end

-- ============================================================================
-- Selection
-- ============================================================================

function Manipulator.selectActor(actor)
    if not actor or not actor:IsValid() then return end

    Manipulator.selectedActor = actor
    Manipulator.selectedActorName = actor:GetFullName()
    Manipulator.undoStack = {}
    Manipulator.redoStack = {}

    -- Store original transform for reset
    Manipulator.originalTransform = captureState(actor)

    print(string.format("[InZoi TOOL] Selected: %s\n", Manipulator.selectedActorName))
end

function Manipulator.deselect()
    if Manipulator.selectedActor then
        print(string.format("[InZoi TOOL] Deselected: %s\n", Manipulator.selectedActorName))
    end
    Manipulator.selectedActor = nil
    Manipulator.selectedActorName = ""
    Manipulator.originalTransform = nil
    Manipulator.undoStack = {}
    Manipulator.redoStack = {}
end

function Manipulator.hasSelection()
    return Manipulator.selectedActor ~= nil and Manipulator.selectedActor:IsValid()
end

-- ============================================================================
-- Movement
-- ============================================================================

function Manipulator.move(dx, dy, dz)
    if not Manipulator.hasSelection() then return end

    recordStep("move", dx, dy, dz)

    dx, dy, dz = applyMoveSnap(dx, dy, dz)

    -- Apply axis constraints
    local axis = Manipulator.currentAxis
    if axis == "x" then dy = 0; dz = 0
    elseif axis == "y" then dx = 0; dz = 0
    elseif axis == "z" then dx = 0; dy = 0
    elseif axis == "xy" then dz = 0
    elseif axis == "xz" then dy = 0
    elseif axis == "yz" then dx = 0
    end

    Manipulator.pushUndo()

    local loc = getActorLocation(Manipulator.selectedActor)
    loc.x = loc.x + dx
    loc.y = loc.y + dy
    loc.z = loc.z + dz
    setActorLocation(Manipulator.selectedActor, loc)
end

function Manipulator.moveTo(x, y, z)
    if not Manipulator.hasSelection() then return end
    recordStep("moveTo", x, y, z)
    Manipulator.pushUndo()
    setActorLocation(Manipulator.selectedActor, {x = x, y = y, z = z})
end

-- ============================================================================
-- Rotation
-- ============================================================================

function Manipulator.rotate(dp, dy, dr)
    if not Manipulator.hasSelection() then return end

    recordStep("rotate", dp, dy, dr)

    dp, dy, dr = applyRotateSnap(dp, dy, dr)

    -- Apply axis constraints (single-axis zeros two components; compound-axis
    -- zeros the third, mirroring Manipulator.move's convention).
    local axis = Manipulator.currentAxis
    if axis == "x" then dy = 0; dr = 0
    elseif axis == "y" then dp = 0; dr = 0
    elseif axis == "z" then dp = 0; dy = 0
    elseif axis == "xy" then dr = 0
    elseif axis == "xz" then dy = 0
    elseif axis == "yz" then dp = 0
    end

    Manipulator.pushUndo()

    local rot = getActorRotation(Manipulator.selectedActor)
    rot.pitch = rot.pitch + dp
    rot.yaw = rot.yaw + dy
    rot.roll = rot.roll + dr
    setActorRotation(Manipulator.selectedActor, rot)
end

function Manipulator.rotateTo(pitch, yaw, roll)
    if not Manipulator.hasSelection() then return end
    recordStep("rotateTo", pitch, yaw, roll)
    Manipulator.pushUndo()
    setActorRotation(Manipulator.selectedActor, {pitch = pitch, yaw = yaw, roll = roll})
end

function Manipulator.rotateByAngle(degrees)
    local axis = Manipulator.currentAxis
    if axis == "x" then
        Manipulator.rotate(degrees, 0, 0)
    elseif axis == "y" then
        Manipulator.rotate(0, degrees, 0)
    else
        Manipulator.rotate(0, 0, degrees)
    end
end

-- ============================================================================
-- Scaling
-- ============================================================================

function Manipulator.scaleUniform(factor)
    if not Manipulator.hasSelection() then return end

    recordStep("scaleUniform", factor)
    Manipulator.pushUndo()

    local s = getActorScale(Manipulator.selectedActor)
    local minS = Settings.get("minScale") or 0.1
    local maxS = Settings.get("maxScale") or 10.0

    s.x = math.max(minS, math.min(maxS, s.x * factor))
    s.y = math.max(minS, math.min(maxS, s.y * factor))
    s.z = math.max(minS, math.min(maxS, s.z * factor))
    setActorScale(Manipulator.selectedActor, s)
end

function Manipulator.scaleTo(x, y, z)
    if not Manipulator.hasSelection() then return end

    recordStep("scaleTo", x, y, z)
    Manipulator.pushUndo()

    local minS = Settings.get("minScale") or 0.1
    local maxS = Settings.get("maxScale") or 10.0

    x = math.max(minS, math.min(maxS, x))
    y = math.max(minS, math.min(maxS, y))
    z = math.max(minS, math.min(maxS, z))
    setActorScale(Manipulator.selectedActor, {x = x, y = y, z = z})
end

-- ============================================================================
-- Elevation
-- ============================================================================

function Manipulator.elevate(delta)
    if not Manipulator.hasSelection() then return end
    recordStep("elevate", delta)
    Manipulator.pushUndo()

    local loc = getActorLocation(Manipulator.selectedActor)
    loc.z = loc.z + delta
    setActorLocation(Manipulator.selectedActor, loc)
end

function Manipulator.elevateTo(height)
    if not Manipulator.hasSelection() then return end
    recordStep("elevateTo", height)
    Manipulator.pushUndo()

    local loc = getActorLocation(Manipulator.selectedActor)
    loc.z = height
    setActorLocation(Manipulator.selectedActor, loc)
end

-- ============================================================================
-- Reset
-- ============================================================================

function Manipulator.resetTransform()
    if not Manipulator.hasSelection() then return end
    if not Manipulator.originalTransform then return end

    Manipulator.pushUndo()
    restoreState(Manipulator.originalTransform)
    print("[InZoi TOOL] Reset to original transform\n")
end

-- ============================================================================
-- Mode/Axis setters
-- ============================================================================

function Manipulator.setMode(mode)
    Manipulator.currentMode = mode
end

function Manipulator.setAxis(axis)
    Manipulator.currentAxis = axis
end

function Manipulator.cycleAxis()
    local order = {"free", "x", "y", "z", "xy", "xz", "yz"}
    local current = Manipulator.currentAxis
    for i, a in ipairs(order) do
        if a == current then
            Manipulator.currentAxis = order[(i % #order) + 1]
            return
        end
    end
    Manipulator.currentAxis = "free"
end

-- ============================================================================
-- Getters for UI display
-- ============================================================================

function Manipulator.getPosition()
    if not Manipulator.hasSelection() then return nil end
    return getActorLocation(Manipulator.selectedActor)
end

function Manipulator.getRotation()
    if not Manipulator.hasSelection() then return nil end
    return getActorRotation(Manipulator.selectedActor)
end

function Manipulator.getScale()
    if not Manipulator.hasSelection() then return nil end
    return getActorScale(Manipulator.selectedActor)
end

function Manipulator.getModeDisplayName()
    local names = {
        move = "MOVE",
        rotate = "ROTATE",
        scale = "SCALE",
        elevate = "ELEVATE",
    }
    return names[Manipulator.currentMode] or "NONE"
end

function Manipulator.getAxisDisplayName()
    local names = {
        free = "FREE",
        x = "X", y = "Y", z = "Z",
        xy = "XY", xz = "XZ", yz = "YZ",
    }
    return names[Manipulator.currentAxis] or "---"
end

return Manipulator
