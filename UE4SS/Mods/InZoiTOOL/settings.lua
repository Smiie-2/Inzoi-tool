-- ============================================================================
-- InZoi T.O.O.L. - Settings Module
-- Persistent configuration loaded/saved from JSON file
-- ============================================================================

local Settings = {}

-- Default settings
Settings.values = {
    -- General
    toggleKey = "F2",

    -- Movement
    moveGridSnap = 0.0,
    moveSpeed = 1.0,
    allowOffLot = true,
    snapToTerrain = false,

    -- Rotation
    rotateGridSnap = 0.0,
    rotateSpeed = 1.0,

    -- Scale
    scaleGridSnap = 0.0,
    minScale = 0.1,
    maxScale = 10.0,
    scaleSpeed = 1.0,

    -- Elevation
    elevationStep = 0.5,

    -- Undo
    maxUndoSteps = 100,

    -- Visuals (ImGui colors as {r, g, b, a} 0-1)
    xAxisColor = {1.0, 0.2, 0.2, 1.0},
    yAxisColor = {0.2, 1.0, 0.2, 1.0},
    zAxisColor = {0.3, 0.4, 1.0, 1.0},
    activeAxisColor = {1.0, 1.0, 0.0, 1.0},
    gizmoSize = 100.0,
}

-- Path to settings file (relative to mod directory)
local settingsPath = nil

function Settings.init(modDir)
    settingsPath = modDir .. "\\tool_settings.json"
    Settings.load()
end

-- Simple JSON parser for our flat-ish settings structure
local function parseJSON(str)
    local result = {}
    -- Remove outer braces
    str = str:match("^%s*{(.+)}%s*$")
    if not str then return result end

    -- Match key-value pairs (handles strings, numbers, booleans, arrays)
    for key, value in str:gmatch('"([^"]+)"%s*:%s*(%b[])') do
        -- Parse array values like [1.0, 0.2, 0.2, 1.0]
        local arr = {}
        for num in value:gmatch("([%d%.%-]+)") do
            table.insert(arr, tonumber(num))
        end
        if #arr > 0 then
            result[key] = arr
        end
    end

    for key, value in str:gmatch('"([^"]+)"%s*:%s*"([^"]*)"') do
        result[key] = value
    end

    for key, value in str:gmatch('"([^"]+)"%s*:%s*([%d%.%-]+)') do
        result[key] = tonumber(value)
    end

    for key, value in str:gmatch('"([^"]+)"%s*:%s*(true)') do
        result[key] = true
    end

    for key, value in str:gmatch('"([^"]+)"%s*:%s*(false)') do
        result[key] = false
    end

    return result
end

-- Simple JSON serializer for our settings
local function toJSON(tbl)
    local parts = {}
    table.insert(parts, "{")

    local keys = {}
    for k in pairs(tbl) do table.insert(keys, k) end
    table.sort(keys)

    for i, key in ipairs(keys) do
        local val = tbl[key]
        local line
        if type(val) == "string" then
            line = string.format('    "%s": "%s"', key, val)
        elseif type(val) == "boolean" then
            line = string.format('    "%s": %s', key, tostring(val))
        elseif type(val) == "number" then
            line = string.format('    "%s": %s', key, val)
        elseif type(val) == "table" then
            local nums = {}
            for _, v in ipairs(val) do
                table.insert(nums, tostring(v))
            end
            line = string.format('    "%s": [%s]', key, table.concat(nums, ", "))
        end
        if i < #keys then
            line = line .. ","
        end
        table.insert(parts, line)
    end

    table.insert(parts, "}")
    return table.concat(parts, "\n")
end

function Settings.load()
    if not settingsPath then return end

    local f = io.open(settingsPath, "r")
    if not f then
        print("[InZoi TOOL] No settings file found, using defaults\n")
        return
    end

    local content = f:read("*a")
    f:close()

    local parsed = parseJSON(content)
    -- Merge parsed values into settings (only override known keys)
    for key, val in pairs(parsed) do
        if Settings.values[key] ~= nil then
            Settings.values[key] = val
        end
    end

    print("[InZoi TOOL] Settings loaded from " .. settingsPath .. "\n")
end

function Settings.save()
    if not settingsPath then return end

    local json = toJSON(Settings.values)
    local f = io.open(settingsPath, "w")
    if not f then
        print("[InZoi TOOL] ERROR: Could not write settings file\n")
        return
    end
    f:write(json)
    f:close()
    print("[InZoi TOOL] Settings saved\n")
end

function Settings.get(key)
    return Settings.values[key]
end

function Settings.set(key, value)
    if Settings.values[key] ~= nil then
        Settings.values[key] = value
    end
end

return Settings
