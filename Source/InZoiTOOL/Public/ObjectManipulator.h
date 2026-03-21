#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ObjectManipulator.generated.h"

/** The axis or axes currently being manipulated */
UENUM(BlueprintType)
enum class EManipulationAxis : uint8
{
    None        UMETA(DisplayName = "None"),
    X           UMETA(DisplayName = "X Axis"),
    Y           UMETA(DisplayName = "Y Axis"),
    Z           UMETA(DisplayName = "Z Axis"),
    XY          UMETA(DisplayName = "XY Plane"),
    XZ          UMETA(DisplayName = "XZ Plane"),
    YZ          UMETA(DisplayName = "YZ Plane"),
    XYZ         UMETA(DisplayName = "All Axes")
};

/** The current manipulation mode */
UENUM(BlueprintType)
enum class EManipulationMode : uint8
{
    None        UMETA(DisplayName = "None"),
    Move        UMETA(DisplayName = "Move"),
    Rotate      UMETA(DisplayName = "Rotate"),
    Scale       UMETA(DisplayName = "Scale"),
    Elevate     UMETA(DisplayName = "Elevate")
};

/** Snapshot of an object's transform for undo/redo */
USTRUCT(BlueprintType)
struct FTransformSnapshot
{
    GENERATED_BODY()

    UPROPERTY()
    TWeakObjectPtr<AActor> Target;

    UPROPERTY()
    FTransform Transform;

    UPROPERTY()
    float Timestamp = 0.f;
};

/**
 * Core object manipulation component.
 * Handles all move, rotate, scale, and elevate operations with full
 * undo/redo support, grid snapping, terrain snapping, and axis constraints.
 */
UCLASS(ClassGroup=(InZoiTOOL), meta=(BlueprintSpawnableComponent))
class INZOITOOL_API UObjectManipulator : public UActorComponent
{
    GENERATED_BODY()

public:
    UObjectManipulator();

    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
        FActorComponentTickFunction* ThisTickFunction) override;

    // --- Selection ---

    /** Select an actor for manipulation */
    UFUNCTION(BlueprintCallable, Category = "TOOL")
    void SelectObject(AActor* Actor);

    /** Deselect the current object */
    UFUNCTION(BlueprintCallable, Category = "TOOL")
    void DeselectObject();

    /** Get the currently selected actor */
    UFUNCTION(BlueprintPure, Category = "TOOL")
    AActor* GetSelectedObject() const;

    // --- Movement ---

    /** Move the selected object by a world-space delta */
    UFUNCTION(BlueprintCallable, Category = "TOOL|Move")
    void MoveObject(FVector Delta);

    /** Move the selected object to an exact world position */
    UFUNCTION(BlueprintCallable, Category = "TOOL|Move")
    void MoveObjectTo(FVector WorldPosition);

    /** Move via coordinate input string (e.g. "5.0, -3.2, 0") */
    UFUNCTION(BlueprintCallable, Category = "TOOL|Move")
    void MoveObjectByInput(const FString& CoordinateInput);

    // --- Rotation ---

    /** Rotate the selected object by a delta in degrees */
    UFUNCTION(BlueprintCallable, Category = "TOOL|Rotate")
    void RotateObject(FRotator DeltaRotation);

    /** Set the selected object to an exact rotation */
    UFUNCTION(BlueprintCallable, Category = "TOOL|Rotate")
    void RotateObjectTo(FRotator WorldRotation);

    /** Rotate by a specific angle around the active axis */
    UFUNCTION(BlueprintCallable, Category = "TOOL|Rotate")
    void RotateObjectByAngle(float Degrees);

    // --- Scaling ---

    /** Scale the selected object by a multiplier per axis */
    UFUNCTION(BlueprintCallable, Category = "TOOL|Scale")
    void ScaleObject(FVector ScaleMultiplier);

    /** Set the selected object to an exact scale */
    UFUNCTION(BlueprintCallable, Category = "TOOL|Scale")
    void ScaleObjectTo(FVector AbsoluteScale);

    /** Uniform scale by a single factor */
    UFUNCTION(BlueprintCallable, Category = "TOOL|Scale")
    void ScaleObjectUniform(float Factor);

    // --- Elevation ---

    /** Raise or lower the selected object by a delta height */
    UFUNCTION(BlueprintCallable, Category = "TOOL|Elevate")
    void ElevateObject(float DeltaHeight);

    /** Set the selected object to an exact height */
    UFUNCTION(BlueprintCallable, Category = "TOOL|Elevate")
    void ElevateObjectTo(float WorldHeight);

    /** Snap the selected object to terrain height at its current XY */
    UFUNCTION(BlueprintCallable, Category = "TOOL|Elevate")
    void SnapToTerrain();

    // --- Undo/Redo ---

    UFUNCTION(BlueprintCallable, Category = "TOOL|History")
    void Undo();

    UFUNCTION(BlueprintCallable, Category = "TOOL|History")
    void Redo();

    UFUNCTION(BlueprintPure, Category = "TOOL|History")
    bool CanUndo() const;

    UFUNCTION(BlueprintPure, Category = "TOOL|History")
    bool CanRedo() const;

    /** Clear all undo/redo history */
    UFUNCTION(BlueprintCallable, Category = "TOOL|History")
    void ClearHistory();

    // --- Mode & Axis ---

    UFUNCTION(BlueprintCallable, Category = "TOOL")
    void SetMode(EManipulationMode NewMode);

    UFUNCTION(BlueprintPure, Category = "TOOL")
    EManipulationMode GetMode() const { return CurrentMode; }

    UFUNCTION(BlueprintCallable, Category = "TOOL")
    void SetAxis(EManipulationAxis NewAxis);

    UFUNCTION(BlueprintPure, Category = "TOOL")
    EManipulationAxis GetAxis() const { return CurrentAxis; }

    // --- Settings ---

    /** Grid snap increment for movement (0 = free movement) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TOOL|Settings")
    float MoveSnapIncrement = 0.f;

    /** Rotation snap increment in degrees (0 = free rotation) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TOOL|Settings")
    float RotateSnapIncrement = 0.f;

    /** Scale snap increment (0 = free scaling) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TOOL|Settings")
    float ScaleSnapIncrement = 0.f;

    /** Sensitivity multiplier for mouse-drag operations */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TOOL|Settings",
        meta = (ClampMin = "0.01", ClampMax = "10.0"))
    float Sensitivity = 1.0f;

    /** Whether to snap objects to terrain when moved */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TOOL|Settings")
    bool bSnapToTerrain = false;

    /** Whether to move the camera to follow the selected object */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TOOL|Settings")
    bool bSnapCameraToObject = false;

    /** Allow placing objects outside lot boundaries */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TOOL|Settings")
    bool bAllowOffLotPlacement = true;

    /** Maximum undo history depth */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TOOL|Settings",
        meta = (ClampMin = "1", ClampMax = "500"))
    int32 MaxUndoHistory = 100;

protected:
    virtual void BeginPlay() override;

private:
    /** Push current transform onto the undo stack before a change */
    void PushUndoState();

    /** Apply grid snapping to a vector */
    FVector ApplyMoveSnap(FVector Value) const;

    /** Apply rotation snapping */
    FRotator ApplyRotateSnap(FRotator Value) const;

    /** Apply scale snapping */
    FVector ApplyScaleSnap(FVector Value) const;

    /** Trace for terrain height at a world XY position */
    bool TraceTerrainHeight(FVector2D XY, float& OutHeight) const;

    UPROPERTY()
    TWeakObjectPtr<AActor> SelectedActor;

    EManipulationMode CurrentMode = EManipulationMode::Move;
    EManipulationAxis CurrentAxis = EManipulationAxis::XYZ;

    TArray<FTransformSnapshot> UndoStack;
    TArray<FTransformSnapshot> RedoStack;
};
