#include "InZoiTOOLModule.h"
#include "TOOLSettings.h"

#define LOCTEXT_NAMESPACE "FInZoiTOOLModule"

void FInZoiTOOLModule::StartupModule()
{
    UE_LOG(LogTemp, Log, TEXT("[InZoi TOOL] T.O.O.L. mod loaded - Takes Objects Off Lot v1.0.0"));

    // Load user settings from JSON
    if (UTOOLSettings* Settings = UTOOLSettings::Get())
    {
        Settings->LoadFromJson();

        if (Settings->bActiveOnStart)
        {
            bIsToolActive = true;
            UE_LOG(LogTemp, Log, TEXT("[InZoi TOOL] Auto-activated on startup"));
        }
    }
}

void FInZoiTOOLModule::ShutdownModule()
{
    // Save settings on shutdown
    if (UTOOLSettings* Settings = UTOOLSettings::Get())
    {
        Settings->SaveToJson();
    }

    UE_LOG(LogTemp, Log, TEXT("[InZoi TOOL] T.O.O.L. mod unloaded"));
}

FInZoiTOOLModule& FInZoiTOOLModule::Get()
{
    return FModuleManager::LoadModuleChecked<FInZoiTOOLModule>("InZoiTOOL");
}

bool FInZoiTOOLModule::IsAvailable()
{
    return FModuleManager::Get().IsModuleLoaded("InZoiTOOL");
}

void FInZoiTOOLModule::ToggleTool()
{
    bIsToolActive = !bIsToolActive;
    UE_LOG(LogTemp, Log, TEXT("[InZoi TOOL] TOOL %s"),
        bIsToolActive ? TEXT("activated") : TEXT("deactivated"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FInZoiTOOLModule, InZoiTOOL)
