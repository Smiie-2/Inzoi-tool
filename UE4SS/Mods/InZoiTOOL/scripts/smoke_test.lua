-- ============================================================================
-- InZoi T.O.O.L. - Smoke Test
--
-- Runs all five Lua modules under stubbed UE4SS globals so load-time errors
-- and TOOL-API wiring regressions surface without the inZOI game / UE4SS
-- runtime on Windows.
--
-- Run from this directory with:  lua smoke_test.lua
-- Prints "OK" on success; exits non-zero if any assertion fails.
-- ============================================================================

-- Resolve script's own directory so require() finds the sibling modules
-- regardless of the caller's cwd.
local scriptDir = (arg and arg[0] and arg[0]:gsub("[^/\\]+$", "")) or ""
if scriptDir ~= "" then
    package.path = scriptDir .. "?.lua;" .. package.path
end

-- ============================================================================
-- Stubs for UE4SS globals
-- ============================================================================

local fakeActor = {
    _loc   = {X = 0, Y = 0, Z = 0},
    _rot   = {Pitch = 0, Yaw = 0, Roll = 0},
    _scale = {X = 1, Y = 1, Z = 1},
    _name  = "/Game/Fake/Actor.Fake_C",
}
function fakeActor:IsValid() return true end
function fakeActor:K2_GetActorLocation() return self._loc end
function fakeActor:K2_SetActorLocation(loc) self._loc = loc end
function fakeActor:K2_GetActorRotation() return self._rot end
function fakeActor:K2_SetActorRotation(rot) self._rot = rot end
function fakeActor:GetActorScale3D() return self._scale end
function fakeActor:SetActorScale3D(s) self._scale = s end
function fakeActor:GetFullName() return self._name end

function RegisterKeyBind() end
function LoopAsync() end
function ExecuteInGameThread(fn) if fn then fn() end end
function FindFirstOf() return fakeActor end
function FindAllOf() return { fakeActor } end
function StaticFindObject(path) return { _path = path } end
function StaticConstructObject()
    return setmetatable({}, {
        __index = function() return function() end end,
        __newindex = function() end,
    })
end
function FName(n) return n end
function FText(t) return t end

-- Capture the handler RegisterConsoleCommandHandler so the smoke test can
-- invoke the "tool" subcommand dispatcher directly.
_registeredCmds = {}
function RegisterConsoleCommandHandler(name, fn)
    _registeredCmds[name] = fn
end

Key         = setmetatable({}, { __index = function(_, k) return k end })
ModifierKey = setmetatable({}, { __index = function(_, k) return k end })

ModRef = {
    _vars = {},
    GetModPath         = function(self) return "." end,
    SetSharedVariable  = function(self, k, v) self._vars[k] = v end,
    GetSharedVariable  = function(self, k) return self._vars[k] or "" end,
}

-- ============================================================================
-- Assertion helper
-- ============================================================================

local failures = 0
local function check(cond, msg)
    if not cond then
        failures = failures + 1
        io.stderr:write("FAIL: " .. msg .. "\n")
    end
end

-- ============================================================================
-- (0) Regression guard for UE4SS builds that expose ModRef.GetModPath as
--     a truthy non-function (crashed main.lua:20 on live inZOI install).
--     With the fix in place, load must still succeed even when GetModPath
--     is not a function.
-- ============================================================================

do
    local savedGetModPath = ModRef.GetModPath
    ModRef.GetModPath = "not-a-function"  -- truthy, non-callable
    local sentinelOk, sentinelErr = pcall(function()
        -- Drop any cached copy so require re-executes main.lua with the
        -- corrupted ModRef in scope.
        package.loaded["main"] = nil
        require("main")
    end)
    check(sentinelOk,
        "main.lua survives ModRef.GetModPath = non-function userdata: "
        .. tostring(sentinelErr))
    ModRef.GetModPath = savedGetModPath
    package.loaded["main"] = nil
end

-- ============================================================================
-- (1) All five modules load without error
-- ============================================================================

local ok, err = pcall(function() require("main") end)
check(ok, "main.lua loaded without error: " .. tostring(err))

-- ============================================================================
-- (2) TOOL global is populated with README's documented Console API
-- ============================================================================

check(type(TOOL) == "table", "TOOL global populated")

local apiFns = {
    "toggle", "isActive", "status", "help",
    "selectByClass", "browseActors", "selectFromBrowse",
    "deselect", "hasSelection", "getSelectedName",
    "move", "moveTo", "rotate", "rotateTo",
    "scale", "scaleTo", "elevate", "elevateTo",
    "reset", "undo", "redo",
    "setMode", "setAxis",
    "getPosition", "getRotation", "getScale",
    "startMacro", "stopMacro", "playMacro", "listMacros",
}
for _, fn in ipairs(apiFns) do
    check(type(TOOL[fn]) == "function", "TOOL." .. fn .. " is a function")
end

-- ============================================================================
-- (3) Manipulation round-trip: select → move mutates actor + undo stack
-- ============================================================================

check(TOOL.selectByClass("Anything"), "TOOL.selectByClass succeeded against fake actor")

local Manipulator = require("manipulator")
check(#Manipulator.undoStack == 0, "undo stack starts empty after selection")
check(fakeActor._loc.X == 0, "fake actor starts at X=0")

TOOL.move(1, 0, 0)
check(fakeActor._loc.X == 1,
    "TOOL.move(1,0,0) advanced X to 1, got " .. tostring(fakeActor._loc.X))
check(#Manipulator.undoStack == 1,
    "TOOL.move appended one undo entry, got " .. tostring(#Manipulator.undoStack))

-- ============================================================================
-- (4) C1 regression guard: TOOL.move inside a macro records a step
-- ============================================================================

TOOL.startMacro("t")
local Macros = require("macros")
check(Macros.isRecording(), "macro recording is active after TOOL.startMacro")

TOOL.move(1, 0, 0)
TOOL.rotate(0, 10, 0)

TOOL.stopMacro()

local macroList = TOOL.listMacros()
local found
for _, m in ipairs(macroList) do
    if m.name == "t" then found = m end
end
check(found ~= nil, "macro 't' exists in listMacros after stopMacro")
check(found and found.stepCount == 2,
    "macro 't' has 2 recorded steps, got " ..
    tostring(found and found.stepCount))

-- ============================================================================
-- (5) In-game console command "tool" dispatches subcommands correctly
-- ============================================================================

do
    check(_registeredCmds["tool"] ~= nil,
        "RegisterConsoleCommandHandler('tool', ...) was called")

    local handler = _registeredCmds["tool"]
    if handler then
        local sink = {}
        local fakeAr = {
            Log = function(self, line) table.insert(sink, line) end,
        }

        -- Reset actor state so we can check that `tool move` mutates it.
        fakeActor._loc = { X = 0, Y = 0, Z = 0 }

        handler("tool help", { [0] = "tool", [1] = "help" }, fakeAr)
        check(#sink > 0, "'tool help' produced at least one output line")

        sink = {}
        handler("tool move 7 0 0",
            { [0] = "tool", [1] = "move", [2] = "7", [3] = "0", [4] = "0" },
            fakeAr)
        check(fakeActor._loc.X == 7,
            "'tool move 7 0 0' advanced X to 7, got "
            .. tostring(fakeActor._loc.X))

        sink = {}
        handler("tool nope-bad", { [0] = "tool", [1] = "nope-bad" }, fakeAr)
        local joined = table.concat(sink, "\n")
        check(joined:find("Unknown subcommand") ~= nil,
            "'tool nope-bad' returned an Unknown subcommand message")
    end
end

-- ============================================================================
-- Report
-- ============================================================================

if failures == 0 then
    print("OK - smoke test passed")
    os.exit(0)
else
    io.stderr:write(string.format(
        "FAIL - %d assertion(s) failed\n", failures))
    os.exit(1)
end
