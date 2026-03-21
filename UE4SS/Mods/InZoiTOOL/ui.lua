-- ============================================================================
-- InZoi T.O.O.L. - UI Module
-- ImGui-based overlay for UE4SS
-- ============================================================================

local Settings = require("settings")

local UI = {}

-- State
UI.visible = false
UI.showCoordinateInput = false
UI.coordinateInput = ""
UI.statusMessage = "Click an object to select it"
UI.statusTimer = 0

-- References set by main.lua
UI.manipulator = nil
UI.macros = nil
UI.onSelectRequested = nil -- callback for triggering selection mode

-- Color helpers
local function axisColor(colorKey)
    local c = Settings.get(colorKey)
    if c and #c >= 4 then
        return c[1], c[2], c[3], c[4]
    end
    return 1, 1, 1, 1
end

function UI.setStatus(msg)
    UI.statusMessage = msg
    UI.statusTimer = 3.0 -- seconds to show
end

-- ============================================================================
-- Main draw function - called by RegisterDrawCallback in main.lua
-- ============================================================================

function UI.draw()
    if not UI.visible then return end
    if not UI.manipulator then return end

    local M = UI.manipulator
    local imgui = ImGui

    -- Main TOOL window
    local windowOpen = imgui.Begin("InZoi T.O.O.L.", true)
    if not windowOpen then
        UI.visible = false
        imgui.End()
        return
    end

    -- Mode selector
    imgui.Text("Mode:")
    imgui.SameLine()
    local modeNames = {"MOVE", "ROTATE", "SCALE", "ELEVATE"}
    local modeValues = {"move", "rotate", "scale", "elevate"}
    for i, name in ipairs(modeNames) do
        if i > 1 then imgui.SameLine() end
        local isActive = M.currentMode == modeValues[i]
        if isActive then
            imgui.PushStyleColor(ImGuiCol.Button, 0.8, 0.6, 0.0, 1.0)
        end
        if imgui.Button(name) then
            M.setMode(modeValues[i])
        end
        if isActive then
            imgui.PopStyleColor()
        end
    end

    -- Axis selector
    imgui.Text("Axis:")
    imgui.SameLine()
    local axisNames = {"FREE", "X", "Y", "Z", "XY", "XZ", "YZ"}
    local axisValues = {"free", "x", "y", "z", "xy", "xz", "yz"}
    for i, name in ipairs(axisNames) do
        if i > 1 then imgui.SameLine() end
        local isActive = M.currentAxis == axisValues[i]
        if isActive then
            imgui.PushStyleColor(ImGuiCol.Button, 0.8, 0.6, 0.0, 1.0)
        end
        if imgui.Button(name .. "##axis") then
            M.setAxis(axisValues[i])
        end
        if isActive then
            imgui.PopStyleColor()
        end
    end

    imgui.Separator()

    -- Selected object info
    if M.hasSelection() then
        -- Object name (truncate for display)
        local displayName = M.selectedActorName
        if #displayName > 60 then
            displayName = "..." .. displayName:sub(-57)
        end
        imgui.TextColored(0.4, 1.0, 0.4, 1.0, "Selected: " .. displayName)

        -- Position
        local pos = M.getPosition()
        if pos then
            local r, g, b, a = axisColor("xAxisColor")
            imgui.TextColored(r, g, b, a, string.format("X: %.2f", pos.x))
            imgui.SameLine()
            r, g, b, a = axisColor("yAxisColor")
            imgui.TextColored(r, g, b, a, string.format("  Y: %.2f", pos.y))
            imgui.SameLine()
            r, g, b, a = axisColor("zAxisColor")
            imgui.TextColored(r, g, b, a, string.format("  Z: %.2f", pos.z))
        end

        -- Rotation
        local rot = M.getRotation()
        if rot then
            imgui.Text(string.format("Rot: P:%.1f  Y:%.1f  R:%.1f",
                rot.pitch, rot.yaw, rot.roll))
        end

        -- Scale
        local scale = M.getScale()
        if scale then
            imgui.Text(string.format("Scale: X:%.2f  Y:%.2f  Z:%.2f",
                scale.x, scale.y, scale.z))
        end

        imgui.Separator()

        -- Action buttons
        if imgui.Button("Undo") and M.canUndo() then
            M.undo()
        end
        imgui.SameLine()
        if imgui.Button("Redo") and M.canRedo() then
            M.redo()
        end
        imgui.SameLine()
        if imgui.Button("Reset") then
            M.resetTransform()
        end
        imgui.SameLine()
        if imgui.Button("Deselect") then
            M.deselect()
        end

        -- Coordinate input
        imgui.Separator()
        if imgui.Button("Numeric Input (N)") then
            UI.showCoordinateInput = not UI.showCoordinateInput
        end

        if UI.showCoordinateInput then
            imgui.Text("Enter value (format depends on mode):")
            local changed, newText = imgui.InputText("##coord", UI.coordinateInput, 256)
            if changed then
                UI.coordinateInput = newText
            end
            imgui.SameLine()
            if imgui.Button("Apply") then
                UI.applyCoordinateInput()
                UI.showCoordinateInput = false
                UI.coordinateInput = ""
            end
        end
    else
        imgui.TextColored(0.7, 0.7, 0.7, 1.0, "No object selected")
        imgui.Text("Use the console to select actors by class name,")
        imgui.Text("or browse actors with the Actor Browser below.")
    end

    imgui.Separator()

    -- Status message
    if UI.statusMessage and UI.statusMessage ~= "" then
        imgui.TextColored(1.0, 0.8, 0.2, 1.0, UI.statusMessage)
    end

    -- Help
    if imgui.CollapsingHeader("Keybindings") then
        imgui.Text("F2          Toggle TOOL")
        imgui.Text("G/R/S/E     Move/Rotate/Scale/Elevate")
        imgui.Text("X/Y/Z       Constrain to axis")
        imgui.Text("Tab         Cycle axis")
        imgui.Text("N           Numeric input")
        imgui.Text("Ctrl+Z/Y    Undo/Redo")
        imgui.Text("Delete      Reset transform")
        imgui.Text("Escape      Deselect")
    end

    -- Macro controls
    if imgui.CollapsingHeader("Macros") then
        UI.drawMacroPanel()
    end

    -- Actor browser for selecting objects
    if imgui.CollapsingHeader("Actor Browser") then
        UI.drawActorBrowser()
    end

    -- Settings panel
    if imgui.CollapsingHeader("Settings") then
        UI.drawSettingsPanel()
    end

    imgui.End()
end

-- ============================================================================
-- Coordinate input
-- ============================================================================

function UI.applyCoordinateInput()
    if not UI.manipulator or not UI.manipulator.hasSelection() then return end

    local M = UI.manipulator
    local input = UI.coordinateInput

    if M.currentMode == "move" then
        -- Parse "x, y, z" or "x, y"
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

-- ============================================================================
-- Actor Browser - find and select actors in the world
-- ============================================================================

local browserClassFilter = "Actor"
local browserResults = {}
local browserSearchDirty = true

function UI.drawActorBrowser()
    local imgui = ImGui

    imgui.Text("Class filter:")
    local changed, newFilter = imgui.InputText("##classfilter", browserClassFilter, 128)
    if changed then
        browserClassFilter = newFilter
        browserSearchDirty = true
    end
    imgui.SameLine()
    if imgui.Button("Search") then
        browserSearchDirty = true
    end

    if browserSearchDirty then
        browserSearchDirty = false
        browserResults = {}
        local results = FindAllOf(browserClassFilter)
        if results then
            local count = 0
            for _, actor in pairs(results) do
                if count < 50 then -- Limit to 50 results for performance
                    table.insert(browserResults, actor)
                    count = count + 1
                end
            end
        end
    end

    imgui.Text(string.format("Found: %d actors (showing max 50)", #browserResults))

    for i, actor in ipairs(browserResults) do
        if actor:IsValid() then
            local name = actor:GetFullName()
            -- Truncate long names
            if #name > 80 then
                name = "..." .. name:sub(-77)
            end
            if imgui.Selectable(string.format("[%d] %s", i, name)) then
                UI.manipulator.selectActor(actor)
                UI.setStatus("Selected: " .. name)
            end
        end
    end
end

-- ============================================================================
-- Macro Panel
-- ============================================================================

local macroNameInput = ""

function UI.drawMacroPanel()
    if not UI.macros then return end
    local imgui = ImGui

    if UI.macros.isRecording() then
        imgui.TextColored(1.0, 0.3, 0.3, 1.0,
            "RECORDING: " .. (UI.macros.getRecordingName() or ""))
        if imgui.Button("Stop Recording") then
            UI.macros.stopRecording()
        end
    else
        local changed, newName = imgui.InputText("##macroname", macroNameInput, 64)
        if changed then macroNameInput = newName end
        imgui.SameLine()
        if imgui.Button("Record") and macroNameInput ~= "" then
            UI.macros.startRecording(macroNameInput)
        end
    end

    -- List macros
    local macroList = UI.macros.list()
    if #macroList > 0 then
        imgui.Separator()
        for _, macro in ipairs(macroList) do
            imgui.Text(string.format("%s (%d steps)", macro.name, macro.stepCount))
            imgui.SameLine()
            if imgui.SmallButton("Play##" .. macro.name) then
                UI.macros.play(macro.name, UI.manipulator)
            end
            imgui.SameLine()
            if imgui.SmallButton("Delete##" .. macro.name) then
                UI.macros.delete(macro.name)
            end
        end
    end
end

-- ============================================================================
-- Settings Panel
-- ============================================================================

function UI.drawSettingsPanel()
    local imgui = ImGui
    local S = Settings

    imgui.Text("Movement")
    local changed, val

    changed, val = imgui.SliderFloat("Move Grid Snap", S.get("moveGridSnap"), 0.0, 100.0)
    if changed then S.set("moveGridSnap", val) end

    changed, val = imgui.SliderFloat("Move Speed", S.get("moveSpeed"), 0.1, 50.0)
    if changed then S.set("moveSpeed", val) end

    imgui.Separator()
    imgui.Text("Rotation")

    changed, val = imgui.SliderFloat("Rotate Grid Snap", S.get("rotateGridSnap"), 0.0, 90.0)
    if changed then S.set("rotateGridSnap", val) end

    imgui.Separator()
    imgui.Text("Scale")

    changed, val = imgui.SliderFloat("Min Scale", S.get("minScale"), 0.01, 1.0)
    if changed then S.set("minScale", val) end

    changed, val = imgui.SliderFloat("Max Scale", S.get("maxScale"), 1.0, 100.0)
    if changed then S.set("maxScale", val) end

    imgui.Separator()
    imgui.Text("Elevation")

    changed, val = imgui.SliderFloat("Step Size", S.get("elevationStep"), 0.1, 50.0)
    if changed then S.set("elevationStep", val) end

    imgui.Separator()
    changed, val = imgui.SliderInt("Max Undo Steps", S.get("maxUndoSteps"), 1, 500)
    if changed then S.set("maxUndoSteps", val) end

    imgui.Separator()
    if imgui.Button("Save Settings") then
        S.save()
        UI.setStatus("Settings saved!")
    end
    imgui.SameLine()
    if imgui.Button("Reset Defaults") then
        -- Reload defaults by re-requiring
        S.values = {
            moveGridSnap = 0.0, moveSpeed = 1.0, allowOffLot = true,
            snapToTerrain = false, rotateGridSnap = 0.0, rotateSpeed = 1.0,
            scaleGridSnap = 0.0, minScale = 0.1, maxScale = 10.0, scaleSpeed = 1.0,
            elevationStep = 0.5, maxUndoSteps = 100, gizmoSize = 100.0,
            xAxisColor = {1.0, 0.2, 0.2, 1.0}, yAxisColor = {0.2, 1.0, 0.2, 1.0},
            zAxisColor = {0.3, 0.4, 1.0, 1.0}, activeAxisColor = {1.0, 1.0, 0.0, 1.0},
        }
        UI.setStatus("Settings reset to defaults")
    end
end

return UI
