#include "TOOLSettings.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "Misc/Paths.h"

const FString UTOOLSettings::SettingsFilePath =
    FPaths::ProjectPluginsDir() / TEXT("InZoiTOOL/Config/tool_settings.json");

UTOOLSettings::UTOOLSettings()
{
    CategoryName = TEXT("Plugins");
    SectionName = TEXT("InZoi TOOL");
}

UTOOLSettings* UTOOLSettings::Get()
{
    return GetMutableDefault<UTOOLSettings>();
}

void UTOOLSettings::SaveToJson()
{
    TSharedPtr<FJsonObject> Root = MakeShared<FJsonObject>();

    // General
    TSharedPtr<FJsonObject> General = MakeShared<FJsonObject>();
    General->SetStringField(TEXT("toggleKey"), ToggleKey.ToString());
    General->SetBoolField(TEXT("activeOnStart"), bActiveOnStart);
    General->SetBoolField(TEXT("stayOpen"), bStayOpen);
    General->SetBoolField(TEXT("repeatLastCommand"), bRepeatLastCommand);
    Root->SetObjectField(TEXT("general"), General);

    // Movement
    TSharedPtr<FJsonObject> Movement = MakeShared<FJsonObject>();
    Movement->SetNumberField(TEXT("gridSnap"), MoveGridSnap);
    Movement->SetNumberField(TEXT("speed"), MoveSpeed);
    Movement->SetBoolField(TEXT("allowOffLot"), bAllowOffLot);
    Movement->SetBoolField(TEXT("snapToTerrain"), bSnapToTerrain);
    Movement->SetBoolField(TEXT("snapCameraToObject"), bSnapCameraToObject);
    Root->SetObjectField(TEXT("movement"), Movement);

    // Rotation
    TSharedPtr<FJsonObject> Rotation = MakeShared<FJsonObject>();
    Rotation->SetNumberField(TEXT("gridSnap"), RotateGridSnap);
    Rotation->SetNumberField(TEXT("speed"), RotateSpeed);
    Root->SetObjectField(TEXT("rotation"), Rotation);

    // Scale
    TSharedPtr<FJsonObject> Scale = MakeShared<FJsonObject>();
    Scale->SetNumberField(TEXT("gridSnap"), ScaleGridSnap);
    Scale->SetNumberField(TEXT("minScale"), MinScale);
    Scale->SetNumberField(TEXT("maxScale"), MaxScale);
    Scale->SetNumberField(TEXT("speed"), ScaleSpeed);
    Root->SetObjectField(TEXT("scale"), Scale);

    // Elevation
    TSharedPtr<FJsonObject> Elevation = MakeShared<FJsonObject>();
    Elevation->SetNumberField(TEXT("step"), ElevationStep);
    Root->SetObjectField(TEXT("elevation"), Elevation);

    // Visuals
    TSharedPtr<FJsonObject> Visuals = MakeShared<FJsonObject>();
    auto ColorToJson = [](FLinearColor C) -> TSharedPtr<FJsonObject>
    {
        TSharedPtr<FJsonObject> Obj = MakeShared<FJsonObject>();
        Obj->SetNumberField(TEXT("r"), C.R);
        Obj->SetNumberField(TEXT("g"), C.G);
        Obj->SetNumberField(TEXT("b"), C.B);
        Obj->SetNumberField(TEXT("a"), C.A);
        return Obj;
    };
    Visuals->SetObjectField(TEXT("xAxisColor"), ColorToJson(XAxisColor));
    Visuals->SetObjectField(TEXT("yAxisColor"), ColorToJson(YAxisColor));
    Visuals->SetObjectField(TEXT("zAxisColor"), ColorToJson(ZAxisColor));
    Visuals->SetObjectField(TEXT("activeAxisColor"), ColorToJson(ActiveAxisColor));
    Visuals->SetNumberField(TEXT("selectionOutlineOpacity"), SelectionOutlineOpacity);
    Visuals->SetNumberField(TEXT("gizmoSize"), GizmoSize);
    Root->SetObjectField(TEXT("visuals"), Visuals);

    // Undo
    Root->SetNumberField(TEXT("maxUndoSteps"), MaxUndoSteps);

    // Advanced
    TSharedPtr<FJsonObject> Advanced = MakeShared<FJsonObject>();
    Advanced->SetBoolField(TEXT("enableLuaScripting"), bEnableLuaScripting);
    Advanced->SetStringField(TEXT("luaScriptsPath"), LuaScriptsPath);
    Root->SetObjectField(TEXT("advanced"), Advanced);

    // Serialize to string
    FString OutputString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
    FJsonSerializer::Serialize(Root.ToSharedRef(), Writer);

    // Write to file
    FFileHelper::SaveStringToFile(OutputString, *SettingsFilePath);
    UE_LOG(LogTemp, Log, TEXT("[InZoi TOOL] Settings saved to %s"), *SettingsFilePath);
}

void UTOOLSettings::LoadFromJson()
{
    FString JsonString;
    if (!FFileHelper::LoadFileToString(JsonString, *SettingsFilePath))
    {
        UE_LOG(LogTemp, Log,
            TEXT("[InZoi TOOL] No settings file found, using defaults"));
        return;
    }

    TSharedPtr<FJsonObject> Root;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
    if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
    {
        UE_LOG(LogTemp, Warning,
            TEXT("[InZoi TOOL] Failed to parse settings JSON, using defaults"));
        return;
    }

    auto JsonToColor = [](const TSharedPtr<FJsonObject>& Obj) -> FLinearColor
    {
        if (!Obj.IsValid()) return FLinearColor::White;
        return FLinearColor(
            Obj->GetNumberField(TEXT("r")),
            Obj->GetNumberField(TEXT("g")),
            Obj->GetNumberField(TEXT("b")),
            Obj->GetNumberField(TEXT("a"))
        );
    };

    // General
    if (const TSharedPtr<FJsonObject>* General; Root->TryGetObjectField(TEXT("general"), General))
    {
        FString KeyStr;
        if ((*General)->TryGetStringField(TEXT("toggleKey"), KeyStr))
            ToggleKey = FKey(*KeyStr);
        (*General)->TryGetBoolField(TEXT("activeOnStart"), bActiveOnStart);
        (*General)->TryGetBoolField(TEXT("stayOpen"), bStayOpen);
        (*General)->TryGetBoolField(TEXT("repeatLastCommand"), bRepeatLastCommand);
    }

    // Movement
    if (const TSharedPtr<FJsonObject>* Movement; Root->TryGetObjectField(TEXT("movement"), Movement))
    {
        (*Movement)->TryGetNumberField(TEXT("gridSnap"), MoveGridSnap);
        (*Movement)->TryGetNumberField(TEXT("speed"), MoveSpeed);
        (*Movement)->TryGetBoolField(TEXT("allowOffLot"), bAllowOffLot);
        (*Movement)->TryGetBoolField(TEXT("snapToTerrain"), bSnapToTerrain);
        (*Movement)->TryGetBoolField(TEXT("snapCameraToObject"), bSnapCameraToObject);
    }

    // Rotation
    if (const TSharedPtr<FJsonObject>* Rotation; Root->TryGetObjectField(TEXT("rotation"), Rotation))
    {
        (*Rotation)->TryGetNumberField(TEXT("gridSnap"), RotateGridSnap);
        (*Rotation)->TryGetNumberField(TEXT("speed"), RotateSpeed);
    }

    // Scale
    if (const TSharedPtr<FJsonObject>* Scale; Root->TryGetObjectField(TEXT("scale"), Scale))
    {
        (*Scale)->TryGetNumberField(TEXT("gridSnap"), ScaleGridSnap);
        (*Scale)->TryGetNumberField(TEXT("minScale"), MinScale);
        (*Scale)->TryGetNumberField(TEXT("maxScale"), MaxScale);
        (*Scale)->TryGetNumberField(TEXT("speed"), ScaleSpeed);
    }

    // Elevation
    if (const TSharedPtr<FJsonObject>* Elevation; Root->TryGetObjectField(TEXT("elevation"), Elevation))
    {
        (*Elevation)->TryGetNumberField(TEXT("step"), ElevationStep);
    }

    // Visuals
    if (const TSharedPtr<FJsonObject>* Visuals; Root->TryGetObjectField(TEXT("visuals"), Visuals))
    {
        const TSharedPtr<FJsonObject>* ColorObj;
        if ((*Visuals)->TryGetObjectField(TEXT("xAxisColor"), ColorObj))
            XAxisColor = JsonToColor(*ColorObj);
        if ((*Visuals)->TryGetObjectField(TEXT("yAxisColor"), ColorObj))
            YAxisColor = JsonToColor(*ColorObj);
        if ((*Visuals)->TryGetObjectField(TEXT("zAxisColor"), ColorObj))
            ZAxisColor = JsonToColor(*ColorObj);
        if ((*Visuals)->TryGetObjectField(TEXT("activeAxisColor"), ColorObj))
            ActiveAxisColor = JsonToColor(*ColorObj);
        (*Visuals)->TryGetNumberField(TEXT("selectionOutlineOpacity"), SelectionOutlineOpacity);
        double GizmoSizeD;
        if ((*Visuals)->TryGetNumberField(TEXT("gizmoSize"), GizmoSizeD))
            GizmoSize = static_cast<float>(GizmoSizeD);
    }

    // Undo
    double MaxUndoD;
    if (Root->TryGetNumberField(TEXT("maxUndoSteps"), MaxUndoD))
        MaxUndoSteps = static_cast<int32>(MaxUndoD);

    // Advanced
    if (const TSharedPtr<FJsonObject>* Advanced; Root->TryGetObjectField(TEXT("advanced"), Advanced))
    {
        (*Advanced)->TryGetBoolField(TEXT("enableLuaScripting"), bEnableLuaScripting);
        (*Advanced)->TryGetStringField(TEXT("luaScriptsPath"), LuaScriptsPath);
    }

    UE_LOG(LogTemp, Log, TEXT("[InZoi TOOL] Settings loaded from %s"), *SettingsFilePath);
}
