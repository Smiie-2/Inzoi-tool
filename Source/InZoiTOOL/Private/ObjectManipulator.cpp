#include "ObjectManipulator.h"
#include "TOOLSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"

UObjectManipulator::UObjectManipulator()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UObjectManipulator::BeginPlay()
{
    Super::BeginPlay();

    // Apply settings from saved config
    if (const UTOOLSettings* Settings = UTOOLSettings::Get())
    {
        MoveSnapIncrement = Settings->MoveGridSnap;
        RotateSnapIncrement = Settings->RotateGridSnap;
        ScaleSnapIncrement = Settings->ScaleGridSnap;
        bSnapToTerrain = Settings->bSnapToTerrain;
        bSnapCameraToObject = Settings->bSnapCameraToObject;
        bAllowOffLotPlacement = Settings->bAllowOffLot;
        MaxUndoHistory = Settings->MaxUndoSteps;
    }
}

void UObjectManipulator::TickComponent(float DeltaTime, ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Camera following logic
    if (bSnapCameraToObject && SelectedActor.IsValid())
    {
        if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
        {
            // Smooth camera interpolation toward selected object
            FVector TargetLocation = SelectedActor->GetActorLocation();
            FVector CameraLocation = PC->GetPawn()
                ? PC->GetPawn()->GetActorLocation()
                : FVector::ZeroVector;

            FVector NewLocation = FMath::VInterpTo(CameraLocation, TargetLocation,
                DeltaTime, 3.0f);

            if (PC->GetPawn())
            {
                PC->GetPawn()->SetActorLocation(NewLocation);
            }
        }
    }
}

// ============================================================================
// Selection
// ============================================================================

void UObjectManipulator::SelectObject(AActor* Actor)
{
    if (!Actor) return;

    SelectedActor = Actor;
    ClearHistory();

    UE_LOG(LogTemp, Log, TEXT("[InZoi TOOL] Selected: %s"), *Actor->GetName());
}

void UObjectManipulator::DeselectObject()
{
    if (SelectedActor.IsValid())
    {
        UE_LOG(LogTemp, Log, TEXT("[InZoi TOOL] Deselected: %s"),
            *SelectedActor->GetName());
    }
    SelectedActor = nullptr;
}

AActor* UObjectManipulator::GetSelectedObject() const
{
    return SelectedActor.Get();
}

// ============================================================================
// Movement
// ============================================================================

void UObjectManipulator::MoveObject(FVector Delta)
{
    if (!SelectedActor.IsValid()) return;

    Delta = ApplyMoveSnap(Delta);

    // Constrain to active axis
    switch (CurrentAxis)
    {
        case EManipulationAxis::X:   Delta.Y = 0; Delta.Z = 0; break;
        case EManipulationAxis::Y:   Delta.X = 0; Delta.Z = 0; break;
        case EManipulationAxis::Z:   Delta.X = 0; Delta.Y = 0; break;
        case EManipulationAxis::XY:  Delta.Z = 0; break;
        case EManipulationAxis::XZ:  Delta.Y = 0; break;
        case EManipulationAxis::YZ:  Delta.X = 0; break;
        default: break; // XYZ = all axes free
    }

    PushUndoState();

    FVector NewLocation = SelectedActor->GetActorLocation() + Delta;

    // Terrain snapping
    if (bSnapToTerrain)
    {
        float TerrainHeight = 0.f;
        if (TraceTerrainHeight(FVector2D(NewLocation.X, NewLocation.Y), TerrainHeight))
        {
            NewLocation.Z = TerrainHeight;
        }
    }

    SelectedActor->SetActorLocation(NewLocation);
}

void UObjectManipulator::MoveObjectTo(FVector WorldPosition)
{
    if (!SelectedActor.IsValid()) return;

    PushUndoState();

    if (bSnapToTerrain)
    {
        float TerrainHeight = 0.f;
        if (TraceTerrainHeight(FVector2D(WorldPosition.X, WorldPosition.Y), TerrainHeight))
        {
            WorldPosition.Z = TerrainHeight;
        }
    }

    SelectedActor->SetActorLocation(WorldPosition);
}

void UObjectManipulator::MoveObjectByInput(const FString& CoordinateInput)
{
    // Parse "X, Y" or "X, Y, Z" format
    TArray<FString> Parts;
    CoordinateInput.ParseIntoArray(Parts, TEXT(","), true);

    FVector Delta = FVector::ZeroVector;

    if (Parts.Num() >= 2)
    {
        Delta.X = FCString::Atof(*Parts[0].TrimStartAndEnd());
        Delta.Y = FCString::Atof(*Parts[1].TrimStartAndEnd());
    }
    if (Parts.Num() >= 3)
    {
        Delta.Z = FCString::Atof(*Parts[2].TrimStartAndEnd());
    }

    MoveObject(Delta);
}

// ============================================================================
// Rotation
// ============================================================================

void UObjectManipulator::RotateObject(FRotator DeltaRotation)
{
    if (!SelectedActor.IsValid()) return;

    DeltaRotation = ApplyRotateSnap(DeltaRotation);

    // Constrain to active axis
    switch (CurrentAxis)
    {
        case EManipulationAxis::X:  DeltaRotation.Yaw = 0; DeltaRotation.Roll = 0; break;
        case EManipulationAxis::Y:  DeltaRotation.Pitch = 0; DeltaRotation.Roll = 0; break;
        case EManipulationAxis::Z:  DeltaRotation.Pitch = 0; DeltaRotation.Yaw = 0; break;
        default: break;
    }

    PushUndoState();

    FRotator NewRotation = SelectedActor->GetActorRotation() + DeltaRotation;
    SelectedActor->SetActorRotation(NewRotation);
}

void UObjectManipulator::RotateObjectTo(FRotator WorldRotation)
{
    if (!SelectedActor.IsValid()) return;

    PushUndoState();
    SelectedActor->SetActorRotation(WorldRotation);
}

void UObjectManipulator::RotateObjectByAngle(float Degrees)
{
    FRotator Delta = FRotator::ZeroRotator;

    switch (CurrentAxis)
    {
        case EManipulationAxis::X:  Delta.Pitch = Degrees; break;
        case EManipulationAxis::Y:  Delta.Yaw = Degrees; break;
        case EManipulationAxis::Z:
        default:                    Delta.Roll = Degrees; break;
    }

    RotateObject(Delta);
}

// ============================================================================
// Scaling
// ============================================================================

void UObjectManipulator::ScaleObject(FVector ScaleMultiplier)
{
    if (!SelectedActor.IsValid()) return;

    ScaleMultiplier = ApplyScaleSnap(ScaleMultiplier);

    PushUndoState();

    FVector CurrentScale = SelectedActor->GetActorScale3D();
    FVector NewScale = CurrentScale * ScaleMultiplier;

    // Clamp to min/max from settings
    const UTOOLSettings* Settings = UTOOLSettings::Get();
    float MinS = Settings ? Settings->MinScale : 0.1f;
    float MaxS = Settings ? Settings->MaxScale : 10.f;

    NewScale.X = FMath::Clamp(NewScale.X, MinS, MaxS);
    NewScale.Y = FMath::Clamp(NewScale.Y, MinS, MaxS);
    NewScale.Z = FMath::Clamp(NewScale.Z, MinS, MaxS);

    SelectedActor->SetActorScale3D(NewScale);
}

void UObjectManipulator::ScaleObjectTo(FVector AbsoluteScale)
{
    if (!SelectedActor.IsValid()) return;

    PushUndoState();

    const UTOOLSettings* Settings = UTOOLSettings::Get();
    float MinS = Settings ? Settings->MinScale : 0.1f;
    float MaxS = Settings ? Settings->MaxScale : 10.f;

    AbsoluteScale.X = FMath::Clamp(AbsoluteScale.X, MinS, MaxS);
    AbsoluteScale.Y = FMath::Clamp(AbsoluteScale.Y, MinS, MaxS);
    AbsoluteScale.Z = FMath::Clamp(AbsoluteScale.Z, MinS, MaxS);

    SelectedActor->SetActorScale3D(AbsoluteScale);
}

void UObjectManipulator::ScaleObjectUniform(float Factor)
{
    ScaleObject(FVector(Factor, Factor, Factor));
}

// ============================================================================
// Elevation
// ============================================================================

void UObjectManipulator::ElevateObject(float DeltaHeight)
{
    if (!SelectedActor.IsValid()) return;

    PushUndoState();

    FVector Location = SelectedActor->GetActorLocation();
    Location.Z += DeltaHeight;
    SelectedActor->SetActorLocation(Location);
}

void UObjectManipulator::ElevateObjectTo(float WorldHeight)
{
    if (!SelectedActor.IsValid()) return;

    PushUndoState();

    FVector Location = SelectedActor->GetActorLocation();
    Location.Z = WorldHeight;
    SelectedActor->SetActorLocation(Location);
}

void UObjectManipulator::SnapToTerrain()
{
    if (!SelectedActor.IsValid()) return;

    FVector Location = SelectedActor->GetActorLocation();
    float TerrainHeight = 0.f;

    if (TraceTerrainHeight(FVector2D(Location.X, Location.Y), TerrainHeight))
    {
        PushUndoState();
        Location.Z = TerrainHeight;
        SelectedActor->SetActorLocation(Location);

        UE_LOG(LogTemp, Log, TEXT("[InZoi TOOL] Snapped to terrain at height %.2f"),
            TerrainHeight);
    }
}

// ============================================================================
// Undo / Redo
// ============================================================================

void UObjectManipulator::Undo()
{
    if (!CanUndo()) return;

    FTransformSnapshot Snapshot = UndoStack.Pop();

    if (Snapshot.Target.IsValid())
    {
        // Push current state to redo stack
        FTransformSnapshot RedoSnapshot;
        RedoSnapshot.Target = Snapshot.Target;
        RedoSnapshot.Transform = Snapshot.Target->GetActorTransform();
        RedoSnapshot.Timestamp = GetWorld()->GetTimeSeconds();
        RedoStack.Push(RedoSnapshot);

        // Restore the undo state
        Snapshot.Target->SetActorTransform(Snapshot.Transform);
    }
}

void UObjectManipulator::Redo()
{
    if (!CanRedo()) return;

    FTransformSnapshot Snapshot = RedoStack.Pop();

    if (Snapshot.Target.IsValid())
    {
        // Push current state to undo stack
        FTransformSnapshot UndoSnapshot;
        UndoSnapshot.Target = Snapshot.Target;
        UndoSnapshot.Transform = Snapshot.Target->GetActorTransform();
        UndoSnapshot.Timestamp = GetWorld()->GetTimeSeconds();
        UndoStack.Push(UndoSnapshot);

        // Restore the redo state
        Snapshot.Target->SetActorTransform(Snapshot.Transform);
    }
}

bool UObjectManipulator::CanUndo() const
{
    return UndoStack.Num() > 0;
}

bool UObjectManipulator::CanRedo() const
{
    return RedoStack.Num() > 0;
}

void UObjectManipulator::ClearHistory()
{
    UndoStack.Empty();
    RedoStack.Empty();
}

// ============================================================================
// Mode & Axis
// ============================================================================

void UObjectManipulator::SetMode(EManipulationMode NewMode)
{
    CurrentMode = NewMode;
    UE_LOG(LogTemp, Verbose, TEXT("[InZoi TOOL] Mode changed to %d"),
        static_cast<int32>(NewMode));
}

void UObjectManipulator::SetAxis(EManipulationAxis NewAxis)
{
    CurrentAxis = NewAxis;
    UE_LOG(LogTemp, Verbose, TEXT("[InZoi TOOL] Axis changed to %d"),
        static_cast<int32>(NewAxis));
}

// ============================================================================
// Private Helpers
// ============================================================================

void UObjectManipulator::PushUndoState()
{
    if (!SelectedActor.IsValid()) return;

    FTransformSnapshot Snapshot;
    Snapshot.Target = SelectedActor;
    Snapshot.Transform = SelectedActor->GetActorTransform();
    Snapshot.Timestamp = GetWorld()->GetTimeSeconds();

    UndoStack.Push(Snapshot);

    // Trim undo stack to max size
    while (UndoStack.Num() > MaxUndoHistory)
    {
        UndoStack.RemoveAt(0);
    }

    // Any new change invalidates the redo stack
    RedoStack.Empty();
}

FVector UObjectManipulator::ApplyMoveSnap(FVector Value) const
{
    if (MoveSnapIncrement > 0.f)
    {
        Value.X = FMath::GridSnap(Value.X, MoveSnapIncrement);
        Value.Y = FMath::GridSnap(Value.Y, MoveSnapIncrement);
        Value.Z = FMath::GridSnap(Value.Z, MoveSnapIncrement);
    }
    return Value;
}

FRotator UObjectManipulator::ApplyRotateSnap(FRotator Value) const
{
    if (RotateSnapIncrement > 0.f)
    {
        Value.Pitch = FMath::GridSnap(Value.Pitch, RotateSnapIncrement);
        Value.Yaw = FMath::GridSnap(Value.Yaw, RotateSnapIncrement);
        Value.Roll = FMath::GridSnap(Value.Roll, RotateSnapIncrement);
    }
    return Value;
}

FVector UObjectManipulator::ApplyScaleSnap(FVector Value) const
{
    if (ScaleSnapIncrement > 0.f)
    {
        Value.X = FMath::GridSnap(Value.X, ScaleSnapIncrement);
        Value.Y = FMath::GridSnap(Value.Y, ScaleSnapIncrement);
        Value.Z = FMath::GridSnap(Value.Z, ScaleSnapIncrement);
    }
    return Value;
}

bool UObjectManipulator::TraceTerrainHeight(FVector2D XY, float& OutHeight) const
{
    if (!GetWorld()) return false;

    FVector Start(XY.X, XY.Y, 100000.f);
    FVector End(XY.X, XY.Y, -100000.f);

    FHitResult HitResult;
    FCollisionQueryParams Params;
    Params.bTraceComplex = true;

    if (SelectedActor.IsValid())
    {
        Params.AddIgnoredActor(SelectedActor.Get());
    }

    // Trace against the landscape/terrain channel
    if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End,
        ECC_WorldStatic, Params))
    {
        OutHeight = HitResult.ImpactPoint.Z;
        return true;
    }

    return false;
}
