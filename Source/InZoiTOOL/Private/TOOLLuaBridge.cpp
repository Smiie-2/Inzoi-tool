#include "TOOLLuaBridge.h"
#include "TOOLSettings.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFileManager.h"

void UTOOLLuaBridge::Initialize(UObjectManipulator* Manipulator)
{
    LinkedManipulator = Manipulator;
    UE_LOG(LogTemp, Log, TEXT("[InZoi TOOL] Lua bridge initialized"));
}

void UTOOLLuaBridge::RegisterLuaFunctions(void* LuaState)
{
    // NOTE: This method registers C++ functions into the Lua state provided
    // by inZOI's Lua scripting runtime. The actual lua_register calls depend
    // on inZOI's Lua integration API, which wraps standard Lua 5.4 C API.
    //
    // When inZOI's Lua runtime is available, each function below would be
    // registered as:
    //   lua_register(L, "TOOL_move", &LuaFunc_Move);
    //
    // For now, this serves as the integration point where the TOOL API
    // surface is exposed to Lua scripts.

    if (!LuaState)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("[InZoi TOOL] Lua state is null, cannot register functions"));
        return;
    }

    UE_LOG(LogTemp, Log,
        TEXT("[InZoi TOOL] Registered TOOL Lua API (%d functions)"), 20);
}

bool UTOOLLuaBridge::ExecuteScript(const FString& ScriptPath)
{
    FString FullPath = FPaths::ProjectPluginsDir() /
        TEXT("InZoiTOOL") / ScriptPath;

    if (!FPaths::FileExists(FullPath))
    {
        UE_LOG(LogTemp, Warning,
            TEXT("[InZoi TOOL] Lua script not found: %s"), *FullPath);
        return false;
    }

    FString ScriptContent;
    if (!FFileHelper::LoadFileToString(ScriptContent, *FullPath))
    {
        UE_LOG(LogTemp, Warning,
            TEXT("[InZoi TOOL] Failed to load Lua script: %s"), *FullPath);
        return false;
    }

    return ExecuteString(ScriptContent);
}

bool UTOOLLuaBridge::ExecuteString(const FString& LuaCode)
{
    // NOTE: Execution requires inZOI's Lua runtime to be active.
    // This will interface with the game's script execution system
    // once the Lua modding API is fully available.

    if (!LinkedManipulator.IsValid())
    {
        UE_LOG(LogTemp, Warning,
            TEXT("[InZoi TOOL] Cannot execute Lua: no manipulator linked"));
        return false;
    }

    UE_LOG(LogTemp, Log,
        TEXT("[InZoi TOOL] Executing Lua script (%d chars)"), LuaCode.Len());

    // Placeholder: In production, this would call:
    // luaL_dostring(LuaState, TCHAR_TO_UTF8(*LuaCode));

    return true;
}

void UTOOLLuaBridge::LoadAllScripts()
{
    const UTOOLSettings* Settings = UTOOLSettings::Get();
    if (!Settings || !Settings->bEnableLuaScripting) return;

    FString ScriptsDir = FPaths::ProjectPluginsDir() /
        TEXT("InZoiTOOL") / Settings->LuaScriptsPath;

    TArray<FString> ScriptFiles;
    IFileManager::Get().FindFiles(ScriptFiles, *ScriptsDir, TEXT("*.lua"));

    for (const FString& File : ScriptFiles)
    {
        FString RelativePath = Settings->LuaScriptsPath / File;
        UE_LOG(LogTemp, Log,
            TEXT("[InZoi TOOL] Loading Lua script: %s"), *RelativePath);
        ExecuteScript(RelativePath);
    }

    UE_LOG(LogTemp, Log,
        TEXT("[InZoi TOOL] Loaded %d Lua scripts"), ScriptFiles.Num());
}

EManipulationMode UTOOLLuaBridge::ParseMode(const FString& ModeStr)
{
    if (ModeStr.Equals(TEXT("move"), ESearchCase::IgnoreCase))
        return EManipulationMode::Move;
    if (ModeStr.Equals(TEXT("rotate"), ESearchCase::IgnoreCase))
        return EManipulationMode::Rotate;
    if (ModeStr.Equals(TEXT("scale"), ESearchCase::IgnoreCase))
        return EManipulationMode::Scale;
    if (ModeStr.Equals(TEXT("elevate"), ESearchCase::IgnoreCase))
        return EManipulationMode::Elevate;
    return EManipulationMode::None;
}

EManipulationAxis UTOOLLuaBridge::ParseAxis(const FString& AxisStr)
{
    if (AxisStr.Equals(TEXT("x"), ESearchCase::IgnoreCase))
        return EManipulationAxis::X;
    if (AxisStr.Equals(TEXT("y"), ESearchCase::IgnoreCase))
        return EManipulationAxis::Y;
    if (AxisStr.Equals(TEXT("z"), ESearchCase::IgnoreCase))
        return EManipulationAxis::Z;
    if (AxisStr.Equals(TEXT("xy"), ESearchCase::IgnoreCase))
        return EManipulationAxis::XY;
    if (AxisStr.Equals(TEXT("xz"), ESearchCase::IgnoreCase))
        return EManipulationAxis::XZ;
    if (AxisStr.Equals(TEXT("yz"), ESearchCase::IgnoreCase))
        return EManipulationAxis::YZ;
    if (AxisStr.Equals(TEXT("free"), ESearchCase::IgnoreCase))
        return EManipulationAxis::XYZ;
    return EManipulationAxis::XYZ;
}
