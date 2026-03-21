#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "TOOLSettings.generated.h"

/**
 * Persistent settings for the T.O.O.L. mod.
 * Accessible from Project Settings > Plugins > InZoi TOOL.
 * Also saved/loaded from tool_settings.json for user overrides.
 */
UCLASS(config = TOOL, defaultconfig, meta = (DisplayName = "InZoi TOOL Settings"))
class INZOITOOL_API UTOOLSettings : public UDeveloperSettings
{
    GENERATED_BODY()

public:
    UTOOLSettings();

    static UTOOLSettings* Get();

    // --- General ---

    /** Key to toggle the TOOL overlay (default: F2) */
    UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "General")
    FKey ToggleKey = EKeys::F2;

    /** Whether TOOL starts active on game load */
    UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "General")
    bool bActiveOnStart = false;

    /** Keep the TOOL dialog open after applying a command (Stay Open mode) */
    UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "General")
    bool bStayOpen = true;

    /** Repeat the last command when Stay Open is enabled */
    UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "General")
    bool bRepeatLastCommand = true;

    // --- Movement ---

    /** Enable grid snapping for movement. Set to 0 for free movement. */
    UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "Movement",
        meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float MoveGridSnap = 0.0f;

    /** Movement speed multiplier for keyboard input */
    UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "Movement",
        meta = (ClampMin = "0.1", ClampMax = "50.0"))
    float MoveSpeed = 1.0f;

    /** Allow objects to be placed outside lot boundaries */
    UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "Movement")
    bool bAllowOffLot = true;

    /** Automatically snap moved objects to terrain height */
    UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "Movement")
    bool bSnapToTerrain = false;

    /** Move the camera to follow the object when manipulated */
    UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "Movement")
    bool bSnapCameraToObject = false;

    // --- Rotation ---

    /** Enable angle snapping for rotation. Set to 0 for free rotation. */
    UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "Rotation",
        meta = (ClampMin = "0.0", ClampMax = "90.0"))
    float RotateGridSnap = 0.0f;

    /** Rotation speed multiplier */
    UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "Rotation",
        meta = (ClampMin = "0.1", ClampMax = "10.0"))
    float RotateSpeed = 1.0f;

    // --- Scale ---

    /** Enable grid snapping for scaling. Set to 0 for free scaling. */
    UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "Scale",
        meta = (ClampMin = "0.0", ClampMax = "10.0"))
    float ScaleGridSnap = 0.0f;

    /** Minimum allowed scale */
    UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "Scale",
        meta = (ClampMin = "0.01", ClampMax = "1.0"))
    float MinScale = 0.1f;

    /** Maximum allowed scale */
    UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "Scale",
        meta = (ClampMin = "1.0", ClampMax = "100.0"))
    float MaxScale = 10.0f;

    /** Scale speed multiplier */
    UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "Scale",
        meta = (ClampMin = "0.1", ClampMax = "10.0"))
    float ScaleSpeed = 1.0f;

    // --- Elevation ---

    /** Elevation step size for scroll-wheel adjustments */
    UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "Elevation",
        meta = (ClampMin = "0.1", ClampMax = "50.0"))
    float ElevationStep = 0.5f;

    // --- Visuals ---

    /** Color for the X axis gizmo line */
    UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "Visuals")
    FLinearColor XAxisColor = FLinearColor(1.f, 0.2f, 0.2f, 1.f);

    /** Color for the Y axis gizmo line */
    UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "Visuals")
    FLinearColor YAxisColor = FLinearColor(0.2f, 1.f, 0.2f, 1.f);

    /** Color for the Z axis gizmo line */
    UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "Visuals")
    FLinearColor ZAxisColor = FLinearColor(0.3f, 0.4f, 1.f, 1.f);

    /** Color for the currently active/highlighted axis */
    UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "Visuals")
    FLinearColor ActiveAxisColor = FLinearColor(1.f, 1.f, 0.f, 1.f);

    /** Opacity of the selection highlight outline */
    UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "Visuals",
        meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float SelectionOutlineOpacity = 0.5f;

    /** Size of the gizmo arrows in world units */
    UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "Visuals",
        meta = (ClampMin = "10.0", ClampMax = "500.0"))
    float GizmoSize = 100.0f;

    // --- Undo ---

    /** Maximum number of undo steps stored */
    UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "Undo",
        meta = (ClampMin = "1", ClampMax = "500"))
    int32 MaxUndoSteps = 100;

    // --- Advanced ---

    /** Enable the Lua scripting interface for custom TOOL extensions */
    UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "Advanced")
    bool bEnableLuaScripting = true;

    /** Path to user Lua scripts directory */
    UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "Advanced")
    FString LuaScriptsPath = TEXT("Scripts/");

    // --- Save/Load ---

    /** Save current settings to JSON */
    UFUNCTION(BlueprintCallable, Category = "TOOL|Settings")
    void SaveToJson();

    /** Load settings from JSON, falling back to defaults */
    UFUNCTION(BlueprintCallable, Category = "TOOL|Settings")
    void LoadFromJson();

private:
    static const FString SettingsFilePath;
};
