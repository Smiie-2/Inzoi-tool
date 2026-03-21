#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

/**
 * InZoi T.O.O.L. - Takes Objects Off Lot
 *
 * Main module for the T.O.O.L. mod, providing advanced object manipulation
 * capabilities for inZOI including free movement, rotation, scaling, and
 * elevation control beyond normal build-mode constraints.
 */
class FInZoiTOOLModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

    static FInZoiTOOLModule& Get();
    static bool IsAvailable();

    /** Whether the TOOL overlay is currently active */
    bool bIsToolActive = false;

    /** Toggle the TOOL system on/off */
    void ToggleTool();
};
