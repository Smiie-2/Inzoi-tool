#pragma once

#include "CoreMinimal.h"
#include "ObjectManipulator.h"
#include "TOOLLuaBridge.generated.h"

/**
 * Lua scripting bridge for the T.O.O.L. mod.
 *
 * Exposes TOOL functions to Lua scripts so modders can create custom
 * manipulation workflows, macros, and automation. Uses inZOI's Lua
 * scripting integration (available since the Dec 2025 ModKit update).
 *
 * Lua API:
 *   TOOL.select(actorName)           - Select an actor by name
 *   TOOL.deselect()                  - Deselect current object
 *   TOOL.move(x, y, z)              - Move by delta
 *   TOOL.moveTo(x, y, z)            - Move to world position
 *   TOOL.rotate(pitch, yaw, roll)    - Rotate by delta degrees
 *   TOOL.rotateTo(pitch, yaw, roll)  - Set absolute rotation
 *   TOOL.scale(factor)               - Uniform scale
 *   TOOL.scaleTo(x, y, z)           - Set absolute scale
 *   TOOL.elevate(delta)              - Raise/lower by amount
 *   TOOL.elevateTo(height)           - Set absolute height
 *   TOOL.snapTerrain()               - Snap to terrain
 *   TOOL.undo()                      - Undo last action
 *   TOOL.redo()                      - Redo last undone action
 *   TOOL.setMode(mode)               - "move"|"rotate"|"scale"|"elevate"
 *   TOOL.setAxis(axis)               - "x"|"y"|"z"|"xy"|"xz"|"yz"|"free"
 *   TOOL.getPosition()               - Returns x, y, z
 *   TOOL.getRotation()               - Returns pitch, yaw, roll
 *   TOOL.getScale()                  - Returns x, y, z
 *   TOOL.getSelectedName()           - Returns actor name or nil
 *   TOOL.log(message)                - Print to TOOL log
 */
UCLASS(ClassGroup=(InZoiTOOL))
class INZOITOOL_API UTOOLLuaBridge : public UObject
{
    GENERATED_BODY()

public:
    /** Initialize the Lua bridge with a manipulator reference */
    void Initialize(UObjectManipulator* Manipulator);

    /** Register all TOOL functions into a Lua state */
    void RegisterLuaFunctions(void* LuaState);

    /** Execute a Lua script file */
    UFUNCTION(BlueprintCallable, Category = "TOOL|Lua")
    bool ExecuteScript(const FString& ScriptPath);

    /** Execute a Lua string directly */
    UFUNCTION(BlueprintCallable, Category = "TOOL|Lua")
    bool ExecuteString(const FString& LuaCode);

    /** Load and run all scripts from the configured scripts directory */
    UFUNCTION(BlueprintCallable, Category = "TOOL|Lua")
    void LoadAllScripts();

private:
    /** Helper to convert Lua mode strings to enum */
    static EManipulationMode ParseMode(const FString& ModeStr);

    /** Helper to convert Lua axis strings to enum */
    static EManipulationAxis ParseAxis(const FString& AxisStr);

    UPROPERTY()
    TWeakObjectPtr<UObjectManipulator> LinkedManipulator;
};
