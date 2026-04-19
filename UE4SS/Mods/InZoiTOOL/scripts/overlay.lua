-- ============================================================================
-- InZoi T.O.O.L. - In-Game UMG Overlay
-- Creates a real in-game HUD using UE's UMG widget system via Lua reflection.
-- No C++ required - uses StaticConstructObject to build widgets at runtime.
--
-- Based on the approach from joric/SupraTools, endorsed by the UE4SS team.
-- See: https://github.com/UE4SS-RE/RE-UE4SS/issues/1072
-- ============================================================================

local Overlay = {}

-- State
Overlay.widget = nil
Overlay.isCreated = false
Overlay.isVisible = false

-- Browser (in-overlay actor selection list)
Overlay.browserVisible = false
Overlay.browserItems = {}          -- list of {actor, name}
Overlay.browserIndex = 1           -- 1-based, index into browserItems
Overlay.browserOffset = 0          -- 0-based, index of first visible row
Overlay.browserPageSize = 10       -- rows rendered on screen at once

-- Widget references
local rootWidget = nil
local widgetTree = nil
local canvas = nil
local bgBorder = nil
local titleText = nil
local modeText = nil
local axisText = nil
local objectText = nil
local posText = nil
local rotText = nil
local scaleText = nil
local statusText = nil
local undoRedoText = nil
local helpText = nil
local browserHeader = nil
local browserRows = {}             -- #browserPageSize TextBlocks
local browserHint = nil

-- References set by main.lua
Overlay.manipulator = nil

-- ============================================================================
-- Widget Construction
-- Uses StaticConstructObject to build UMG widget tree from Lua
-- ============================================================================

local function findClass(path)
    return StaticFindObject(path)
end

local function createWidget(className, outer, name)
    local class = findClass("/Script/UMG." .. className)
    if not class then
        print(string.format("[InZoi TOOL] WARNING: UMG class '%s' not found\n", className))
        return nil
    end
    local obj = StaticConstructObject(class, outer, FName(name))
    if not obj then
        print(string.format(
            "[InZoi TOOL] WARNING: StaticConstructObject failed for UMG class '%s' (name='%s')\n",
            className, name))
    end
    return obj
end

local function createTextBlock(outer, name, text, color)
    local tb = createWidget("TextBlock", outer, name)
    if tb then
        tb:SetText(FText(text))
        if color then
            tb:SetColorAndOpacity({SpecifiedColor = color, ColorUseRule = 0})
        end
    end
    return tb
end

function Overlay.create()
    if Overlay.isCreated then return true end

    -- We need a game instance as the outer for the root widget
    local gi = nil

    -- Try to get the game instance
    local giClass = findClass("/Script/Engine.GameInstance")
    if giClass then
        local giInstance = FindFirstOf("GameInstance")
        if giInstance and giInstance:IsValid() then
            gi = giInstance
        end
    end

    if not gi then
        print("[InZoi TOOL] Waiting for GameInstance to create overlay...\n")
        return false
    end

    -- Create root UserWidget
    rootWidget = createWidget("UserWidget", gi, "TOOLOverlay")
    if not rootWidget then
        print("[InZoi TOOL] Failed to create UserWidget - UMG may not be available\n")
        return false
    end

    -- Create widget tree
    local wtClass = findClass("/Script/UMG.WidgetTree")
    if wtClass then
        widgetTree = StaticConstructObject(wtClass, rootWidget, FName("TOOLWidgetTree"))
        rootWidget.WidgetTree = widgetTree
    else
        print("[InZoi TOOL] WidgetTree class not found\n")
        return false
    end

    -- Create canvas panel as root
    canvas = createWidget("CanvasPanel", widgetTree, "TOOLCanvas")
    if not canvas then
        print("[InZoi TOOL] Failed to create CanvasPanel - aborting overlay\n")
        return false
    end
    widgetTree.RootWidget = canvas

    -- Create background border
    bgBorder = createWidget("Border", widgetTree, "TOOLBg")
    if bgBorder then
        -- Semi-transparent dark background
        bgBorder:SetBrushColor({R = 0.0, G = 0.0, B = 0.0, A = 0.7})
        bgBorder:SetPadding({Left = 15, Top = 10, Right = 15, Bottom = 10})
    end

    -- Create a vertical box to stack text elements
    local vbox = createWidget("VerticalBox", widgetTree, "TOOLVBox")

    -- Create text blocks
    titleText = createTextBlock(widgetTree, "TOOLTitle", "InZoi T.O.O.L.", {R=1.0, G=0.8, B=0.2, A=1.0})
    modeText = createTextBlock(widgetTree, "TOOLMode", "Mode: MOVE", {R=1.0, G=1.0, B=1.0, A=1.0})
    axisText = createTextBlock(widgetTree, "TOOLAxis", "Axis: FREE", {R=1.0, G=1.0, B=1.0, A=1.0})
    objectText = createTextBlock(widgetTree, "TOOLObject", "No Selection", {R=0.4, G=1.0, B=0.4, A=1.0})
    posText = createTextBlock(widgetTree, "TOOLPos", "", {R=0.8, G=0.8, B=0.8, A=1.0})
    rotText = createTextBlock(widgetTree, "TOOLRot", "", {R=0.8, G=0.8, B=0.8, A=1.0})
    scaleText = createTextBlock(widgetTree, "TOOLScale", "", {R=0.8, G=0.8, B=0.8, A=1.0})
    undoRedoText = createTextBlock(widgetTree, "TOOLUndoRedo", "", {R=0.6, G=0.6, B=0.6, A=1.0})
    statusText = createTextBlock(widgetTree, "TOOLStatus", "Press F2 to activate", {R=1.0, G=0.8, B=0.2, A=1.0})
    helpText = createTextBlock(widgetTree, "TOOLHelp", "G/R/S/E=Mode  X/Y/Z=Axis  Arrows=Nudge  F4=Browse", {R=0.5, G=0.5, B=0.5, A=1.0})

    -- Browser header + row widgets (collapsed by default; populated via
    -- Overlay.openBrowser / Overlay.renderBrowser at runtime).
    browserHeader = createTextBlock(widgetTree, "TOOLBrowseHeader", "",
        {R = 1.0, G = 0.7, B = 0.3, A = 1.0})
    for i = 1, Overlay.browserPageSize do
        browserRows[i] = createTextBlock(widgetTree, "TOOLBrowseRow" .. i, "",
            {R = 0.7, G = 0.7, B = 0.7, A = 1.0})
    end
    browserHint = createTextBlock(widgetTree, "TOOLBrowseHint",
        "Up/Down select  |  PgUp/PgDn page  |  Enter confirm  |  Esc cancel",
        {R = 0.6, G = 0.6, B = 0.6, A = 1.0})

    -- Build hierarchy: add text blocks to vbox, vbox to border, border to canvas
    if vbox then
        local textWidgets = {
            titleText, modeText, axisText, objectText,
            posText, rotText, scaleText, undoRedoText,
            statusText, helpText,
            browserHeader,
        }
        for _, tw in ipairs(textWidgets) do
            if tw then vbox:AddChildToVerticalBox(tw) end
        end
        for i = 1, Overlay.browserPageSize do
            if browserRows[i] then
                vbox:AddChildToVerticalBox(browserRows[i])
            end
        end
        if browserHint then vbox:AddChildToVerticalBox(browserHint) end
    end

    -- Browser rows / header / hint start hidden (ESlateVisibility::Collapsed = 1).
    if browserHeader then browserHeader:SetVisibility(1) end
    if browserHint then browserHint:SetVisibility(1) end
    for i = 1, Overlay.browserPageSize do
        if browserRows[i] then browserRows[i]:SetVisibility(1) end
    end

    if bgBorder and vbox then
        bgBorder:SetContent(vbox)
    end

    if canvas and bgBorder then
        local slot = canvas:AddChildToCanvas(bgBorder)
        if slot then
            -- Position in top-left area
            slot:SetAnchors({Minimum = {X = 0.0, Y = 0.0}, Maximum = {X = 0.0, Y = 0.0}})
            slot:SetPosition({X = 20, Y = 20})
            slot:SetAutoSize(true)
        end
    end

    -- Add to viewport at high Z-order
    rootWidget:AddToViewport(100)

    Overlay.widget = rootWidget
    Overlay.isCreated = true
    Overlay.isVisible = true

    print("[InZoi TOOL] In-game overlay created!\n")
    return true
end

-- ============================================================================
-- Update the overlay display
-- ============================================================================

function Overlay.update()
    if not Overlay.isCreated or not Overlay.isVisible then return end

    local M = Overlay.manipulator
    if not M then return end

    -- Mode and axis
    if modeText then
        modeText:SetText(FText("Mode: " .. M.getModeDisplayName()))
    end
    if axisText then
        axisText:SetText(FText("Axis: " .. M.getAxisDisplayName()))
    end

    -- Selected object
    if M.hasSelection() then
        local name = M.selectedActorName
        if #name > 50 then name = "..." .. name:sub(-47) end

        if objectText then objectText:SetText(FText(name)) end

        local pos = M.getPosition()
        if pos and posText then
            posText:SetText(FText(string.format("Pos: X:%.1f Y:%.1f Z:%.1f",
                pos.x, pos.y, pos.z)))
        end

        local rot = M.getRotation()
        if rot and rotText then
            rotText:SetText(FText(string.format("Rot: P:%.1f Y:%.1f R:%.1f",
                rot.pitch, rot.yaw, rot.roll)))
        end

        local scale = M.getScale()
        if scale and scaleText then
            scaleText:SetText(FText(string.format("Scale: X:%.2f Y:%.2f Z:%.2f",
                scale.x, scale.y, scale.z)))
        end

        if undoRedoText then
            undoRedoText:SetText(FText(string.format("Undo: %d | Redo: %d",
                #M.undoStack, #M.redoStack)))
        end
    else
        if objectText then objectText:SetText(FText("No Selection")) end
        if posText then posText:SetText(FText("")) end
        if rotText then rotText:SetText(FText("")) end
        if scaleText then scaleText:SetText(FText("")) end
        if undoRedoText then undoRedoText:SetText(FText("TOOL.browseActors('Actor') to find objects")) end
    end
end

-- ============================================================================
-- Show / Hide
-- ============================================================================

function Overlay.show()
    if not Overlay.isCreated then
        if not Overlay.create() then
            return false
        end
    end

    if rootWidget then
        rootWidget:SetVisibility(0) -- ESlateVisibility::Visible
        Overlay.isVisible = true
    end
    return true
end

function Overlay.hide()
    if rootWidget then
        rootWidget:SetVisibility(1) -- ESlateVisibility::Collapsed
        Overlay.isVisible = false
    end
end

function Overlay.setStatus(msg)
    if statusText then
        statusText:SetText(FText(msg or ""))
    end
end

function Overlay.destroy()
    if rootWidget then
        rootWidget:RemoveFromViewport()
        rootWidget = nil
    end
    Overlay.isCreated = false
    Overlay.isVisible = false
end

-- ============================================================================
-- In-overlay actor browser (keyboard-driven list)
-- Driven from main.lua: F4/F5 open, arrows/page navigate, Enter confirms,
-- Esc cancels. Writes Overlay.browser* state and re-renders rows.
-- ============================================================================

local HIGHLIGHT_COLOR = {R = 1.0, G = 0.9, B = 0.4, A = 1.0}
local DIM_COLOR       = {R = 0.7, G = 0.7, B = 0.7, A = 1.0}

local function truncateName(name, max)
    max = max or 55
    if #name > max then return "..." .. name:sub(-(max - 3)) end
    return name
end

local function setColor(widget, color)
    if widget and color then
        pcall(function()
            widget:SetColorAndOpacity({
                SpecifiedColor = color,
                ColorUseRule = 0,
            })
        end)
    end
end

function Overlay.renderBrowser()
    if not Overlay.isCreated then return end

    local visible = Overlay.browserVisible
    local items = Overlay.browserItems
    local idx = Overlay.browserIndex
    local offset = Overlay.browserOffset
    local pageSize = Overlay.browserPageSize

    if browserHeader then
        if visible then
            browserHeader:SetText(FText(string.format(
                "Browse: %d / %d    (class: %s)",
                idx, #items,
                tostring(Overlay.browserClassName or ""))))
            browserHeader:SetVisibility(0)
        else
            browserHeader:SetVisibility(1)
        end
    end

    if browserHint then
        browserHint:SetVisibility(visible and 0 or 1)
    end

    for i = 1, pageSize do
        local tb = browserRows[i]
        if tb then
            if visible then
                local itemIdx = offset + i
                local item = items[itemIdx]
                if item then
                    local marker = (itemIdx == idx) and "> " or "  "
                    tb:SetText(FText(string.format(
                        "%s%d. %s",
                        marker, itemIdx,
                        truncateName(item.name or ""))))
                    setColor(tb, (itemIdx == idx) and HIGHLIGHT_COLOR or DIM_COLOR)
                    tb:SetVisibility(0)
                else
                    tb:SetVisibility(1)
                end
            else
                tb:SetVisibility(1)
            end
        end
    end
end

function Overlay.openBrowser(className, maxResults)
    className = className or "StaticMeshActor"
    maxResults = maxResults or 50

    if not Overlay.isCreated then
        if not Overlay.create() then
            print("[InZoi TOOL] Cannot open browser: overlay not ready yet\n")
            return false
        end
    end

    local found = FindAllOf(className)
    if type(found) ~= "table" then found = {} end

    local items = {}
    for _, a in ipairs(found) do
        if a and a.IsValid and a:IsValid() then
            table.insert(items, {actor = a, name = a:GetFullName()})
            if #items >= maxResults then break end
        end
    end

    if #items == 0 then
        print(string.format(
            "[InZoi TOOL] Browser: no instances of '%s' found\n",
            tostring(className)))
        Overlay.browserVisible = false
        Overlay.renderBrowser()
        return false
    end

    Overlay.browserClassName = className
    Overlay.browserItems = items
    Overlay.browserIndex = 1
    Overlay.browserOffset = 0
    Overlay.browserVisible = true
    Overlay.renderBrowser()
    print(string.format(
        "[InZoi TOOL] Browser: %d instances of '%s' - arrows to navigate, Enter to select\n",
        #items, className))
    return true
end

function Overlay.closeBrowser()
    Overlay.browserVisible = false
    Overlay.browserItems = {}
    Overlay.browserIndex = 1
    Overlay.browserOffset = 0
    Overlay.renderBrowser()
end

function Overlay.browserMove(delta)
    if not Overlay.browserVisible then return end
    local n = #Overlay.browserItems
    if n == 0 then return end

    local idx = Overlay.browserIndex + delta
    if idx < 1 then idx = 1 end
    if idx > n then idx = n end
    Overlay.browserIndex = idx

    local pageSize = Overlay.browserPageSize
    if (idx - 1) < Overlay.browserOffset then
        Overlay.browserOffset = idx - 1
    elseif (idx - 1) >= Overlay.browserOffset + pageSize then
        Overlay.browserOffset = idx - pageSize
    end
    Overlay.renderBrowser()
end

function Overlay.browserConfirm()
    if not Overlay.browserVisible then return false end
    local item = Overlay.browserItems[Overlay.browserIndex]
    local M = Overlay.manipulator
    if item and item.actor and M and M.selectActor then
        M.selectActor(item.actor)
    end
    Overlay.closeBrowser()
    return true
end

return Overlay
