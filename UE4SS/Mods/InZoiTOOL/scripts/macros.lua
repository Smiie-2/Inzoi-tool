-- ============================================================================
-- InZoi T.O.O.L. - Macro System
-- Record, save, and replay sequences of manipulation operations
-- ============================================================================

local Macros = {}

local macroStore = {}
local recording = false
local currentMacroName = nil
local currentMacroSteps = {}

function Macros.isRecording()
    return recording
end

function Macros.getRecordingName()
    return currentMacroName
end

--- Start recording a macro
function Macros.startRecording(name)
    if recording then
        print("[InZoi TOOL] Already recording: " .. currentMacroName .. "\n")
        return false
    end
    recording = true
    currentMacroName = name
    currentMacroSteps = {}
    print("[InZoi TOOL] Recording macro: " .. name .. "\n")
    return true
end

--- Stop recording and save
function Macros.stopRecording()
    if not recording then
        print("[InZoi TOOL] Not recording\n")
        return false
    end
    macroStore[currentMacroName] = currentMacroSteps
    print(string.format("[InZoi TOOL] Saved macro '%s' (%d steps)\n",
        currentMacroName, #currentMacroSteps))
    recording = false
    currentMacroName = nil
    currentMacroSteps = {}
    return true
end

--- Record a single step (called by manipulator functions)
function Macros.recordStep(funcName, ...)
    if not recording then return end
    table.insert(currentMacroSteps, {
        func = funcName,
        args = {...},
    })
end

--- Play back a recorded macro
function Macros.play(name, manipulator)
    local steps = macroStore[name]
    if not steps then
        print("[InZoi TOOL] Macro not found: " .. name .. "\n")
        return false
    end

    print(string.format("[InZoi TOOL] Playing macro: %s (%d steps)\n", name, #steps))

    for _, step in ipairs(steps) do
        local fn = manipulator[step.func]
        if fn then
            fn(table.unpack(step.args))
        end
    end

    print("[InZoi TOOL] Macro complete: " .. name .. "\n")
    return true
end

--- List all macros
function Macros.list()
    local names = {}
    for name, steps in pairs(macroStore) do
        table.insert(names, {name = name, stepCount = #steps})
    end
    return names
end

--- Delete a macro
function Macros.delete(name)
    macroStore[name] = nil
end

return Macros
