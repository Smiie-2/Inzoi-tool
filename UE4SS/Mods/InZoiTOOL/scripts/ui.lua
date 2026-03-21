-- ============================================================================
-- InZoi T.O.O.L. - UI Module
-- Dual-mode: Console output + shared variables for C++ ImGui companion
--
-- The Lua mod pushes state to shared variables every tick. If the C++
-- companion mod (InZoiTOOL_GUI) is installed, it reads these variables
-- and renders a visual ImGui tab. Without the companion, everything
-- works via console commands and keybindings.
-- ============================================================================

local Settings = require("settings")

local UI = {}

-- State
UI.visible = false
UI.statusMessage = ""

-- References set by main.lua
UI.manipulator = nil
UI.macros = nil

-- Actor browser results (for console-based browsing)
UI.lastBrowseResults = {}

-- ============================================================================
-- Shared variable sync (for C++ ImGui companion mod)
-- Updates shared vars so the GUI tab can display current state
-- ============================================================================

local function trySetShared(key, value)
    -- ModRef:SetSharedVariable is available in UE4SS
    if ModRef and ModRef.SetSharedVariable then
        ModRef:SetSharedVariable(key, tostring(value))
    end
end

function UI.syncSharedState(isActive)
    local M = UI.manipulator
    if not M then return end

    trySetShared("TOOL_Active", isActive and "true" or "false")
    trySetShared("TOOL_Mode", M.currentMode)
    trySetShared("TOOL_Axis", M.currentAxis)
    trySetShared("TOOL_Status", UI.statusMessage)

    if M.hasSelection() then
        local name = M.selectedActorName
        if #name > 60 then name = "..." .. name:sub(-57) end
        trySetShared("TOOL_SelectedName", name)

        local pos = M.getPosition()
        if pos then
            trySetShared("TOOL_PosX", string.format("%.2f", pos.x))
            trySetShared("TOOL_PosY", string.format("%.2f", pos.y))
            trySetShared("TOOL_PosZ", string.format("%.2f", pos.z))
        end

        local rot = M.getRotation()
        if rot then
            trySetShared("TOOL_RotP", string.format("%.1f", rot.pitch))
            trySetShared("TOOL_RotY", string.format("%.1f", rot.yaw))
            trySetShared("TOOL_RotR", string.format("%.1f", rot.roll))
        end

        local scale = M.getScale()
        if scale then
            trySetShared("TOOL_ScaleX", string.format("%.2f", scale.x))
            trySetShared("TOOL_ScaleY", string.format("%.2f", scale.y))
            trySetShared("TOOL_ScaleZ", string.format("%.2f", scale.z))
        end

        trySetShared("TOOL_UndoCount", tostring(#M.undoStack))
        trySetShared("TOOL_RedoCount", tostring(#M.redoStack))
    else
        trySetShared("TOOL_SelectedName", "")
    end
end

-- ============================================================================
-- Check for commands from the C++ GUI companion
-- ============================================================================

local function tryGetShared(key)
    if ModRef and ModRef.GetSharedVariable then
        return ModRef:GetSharedVariable(key)
    end
    return ""
end

function UI.pollCommands()
    local cmd = tryGetShared("TOOL_Command")
    if not cmd or cmd == "" then return end

    -- Clear the command immediately
    trySetShared("TOOL_Command", "")

    local M = UI.manipulator

    if cmd == "toggle" then
        -- Handled by main.lua's toggleTool
        return "toggle"
    elseif cmd == "undo" then
        M.undo()
        UI.setStatus("Undo")
    elseif cmd == "redo" then
        M.redo()
        UI.setStatus("Redo")
    elseif cmd == "reset" then
        M.resetTransform()
        UI.setStatus("Reset")
    elseif cmd == "deselect" then
        M.deselect()
        UI.setStatus("Deselected")
    elseif cmd:sub(1, 5) == "mode:" then
        M.setMode(cmd:sub(6))
        UI.setStatus("Mode: " .. M.getModeDisplayName())
    elseif cmd:sub(1, 5) == "axis:" then
        M.setAxis(cmd:sub(6))
        UI.setStatus("Axis: " .. M.getAxisDisplayName())
    elseif cmd:sub(1, 7) == "browse:" then
        UI.browseActors(cmd:sub(8))
    elseif cmd:sub(1, 6) == "input:" then
        UI.applyCoordinateInput(cmd:sub(7))
    end

    return nil
end

-- ============================================================================
-- Console output
-- ============================================================================

function UI.setStatus(msg)
    UI.statusMessage = msg or ""
    print(string.format("[InZoi TOOL] %s\n", msg))
end

function UI.printStatus()
    local M = UI.manipulator
    if not M then return end

    print("=== InZoi T.O.O.L. Status ===\n")
    print(string.format("  Mode: %s  |  Axis: %s\n",
        M.getModeDisplayName(), M.getAxisDisplayName()))

    if M.hasSelection() then
        local name = M.selectedActorName
        if #name > 70 then name = "..." .. name:sub(-67) end
        print(string.format("  Selected: %s\n", name))

        local pos = M.getPosition()
        if pos then
            print(string.format("  Pos:   X:%.2f  Y:%.2f  Z:%.2f\n", pos.x, pos.y, pos.z))
        end

        local rot = M.getRotation()
        if rot then
            print(string.format("  Rot:   P:%.1f  Y:%.1f  R:%.1f\n",
                rot.pitch, rot.yaw, rot.roll))
        end

        local scale = M.getScale()
        if scale then
            print(string.format("  Scale: X:%.2f  Y:%.2f  Z:%.2f\n",
                scale.x, scale.y, scale.z))
        end

        print(string.format("  Undo: %d  |  Redo: %d\n",
            #M.undoStack, #M.redoStack))
    else
        print("  No object selected\n")
        print("  Use TOOL.selectByClass('ClassName') to select\n")
    end
    print("=============================\n")
end

function UI.printHelp()
    print("\n=== InZoi T.O.O.L. - Help ===\n\n")
    print("KEYBINDINGS:\n")
    print("  F2              Toggle TOOL on/off\n")
    print("  G / R / S / E   Move / Rotate / Scale / Elevate mode\n")
    print("  X / Y / Z       Constrain to axis\n")
    print("  Tab             Cycle axis\n")
    print("  Arrow Keys      Nudge (direction depends on mode)\n")
    print("  Page Up/Down    Elevate up/down\n")
    print("  Ctrl+Z / Ctrl+Y Undo / Redo\n")
    print("  Delete          Reset to original transform\n")
    print("  Escape          Deselect\n")
    print("  F3              Print status to console\n")
    print("\nCONSOLE COMMANDS:\n")
    print("  TOOL.selectByClass('ClassName')  Select by class\n")
    print("  TOOL.browseActors('Actor')       List actors\n")
    print("  TOOL.selectFromBrowse(3)         Select from list\n")
    print("  TOOL.move(x, y, z)               Move by delta\n")
    print("  TOOL.moveTo(x, y, z)             Move to position\n")
    print("  TOOL.rotate(pitch, yaw, roll)    Rotate by delta\n")
    print("  TOOL.scale(factor)               Uniform scale\n")
    print("  TOOL.elevate(delta)              Raise/lower\n")
    print("  TOOL.undo() / TOOL.redo()        Undo/Redo\n")
    print("  TOOL.reset()                     Reset transform\n")
    print("  TOOL.status()                    Print status\n")
    print("  TOOL.help()                      This help\n")
    print("\nMACROS:\n")
    print("  TOOL.startMacro('name')          Start recording\n")
    print("  TOOL.stopMacro()                 Stop recording\n")
    print("  TOOL.playMacro('name')           Replay macro\n")
    print("  TOOL.listMacros()                List all macros\n")
    print("=============================\n")
end

-- ============================================================================
-- Actor browser (console-based, results stored for selectFromBrowse)
-- ============================================================================

function UI.browseActors(className, maxResults)
    maxResults = maxResults or 20
    UI.lastBrowseResults = {}

    local results = FindAllOf(className)
    if not results then
        print(string.format("[InZoi TOOL] No instances of '%s' found\n", className))
        return
    end

    local count = 0
    print(string.format("\n=== Actors of class '%s' ===\n", className))
    for _, actor in pairs(results) do
        if actor:IsValid() and count < maxResults then
            count = count + 1
            table.insert(UI.lastBrowseResults, actor)
            local name = actor:GetFullName()
            if #name > 80 then name = "..." .. name:sub(-77) end
            print(string.format("  [%d] %s\n", count, name))
        end
    end
    print(string.format("=== %d results (max %d) ===\n", count, maxResults))
    print("Use TOOL.selectFromBrowse(N) to select one\n")
end

function UI.selectFromBrowse(index)
    if index < 1 or index > #UI.lastBrowseResults then
        print(string.format("[InZoi TOOL] Invalid index %d (1-%d available)\n",
            index, #UI.lastBrowseResults))
        return false
    end

    local actor = UI.lastBrowseResults[index]
    if not actor or not actor:IsValid() then
        print("[InZoi TOOL] Actor no longer valid\n")
        return false
    end

    UI.manipulator.selectActor(actor)
    UI.printStatus()
    return true
end

-- ============================================================================
-- Coordinate input (from console or GUI companion command)
-- ============================================================================

function UI.applyCoordinateInput(input)
    if not UI.manipulator or not UI.manipulator.hasSelection() then return end

    local M = UI.manipulator

    if M.currentMode == "move" then
        local parts = {}
        for num in input:gmatch("([%d%.%-]+)") do
            table.insert(parts, tonumber(num))
        end
        if #parts >= 3 then
            M.move(parts[1], parts[2], parts[3])
        elseif #parts >= 2 then
            M.move(parts[1], parts[2], 0)
        elseif #parts >= 1 then
            M.move(parts[1], 0, 0)
        end
    elseif M.currentMode == "rotate" then
        local angle = tonumber(input)
        if angle then M.rotateByAngle(angle) end
    elseif M.currentMode == "scale" then
        local factor = tonumber(input)
        if factor and factor > 0 then M.scaleUniform(factor) end
    elseif M.currentMode == "elevate" then
        local height = tonumber(input)
        if height then M.elevate(height) end
    end

    UI.setStatus("Applied: " .. input)
end

return UI
