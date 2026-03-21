#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ObjectManipulator.h"
#include "TOOLGizmoRenderer.generated.h"

/**
 * Renders the 3D manipulation gizmo (axis arrows, rotation rings, scale handles)
 * in the viewport around the selected object.
 * Updates each frame based on the current manipulation mode and axis.
 */
UCLASS(ClassGroup=(InZoiTOOL))
class INZOITOOL_API ATOOLGizmoRenderer : public AActor
{
    GENERATED_BODY()

public:
    ATOOLGizmoRenderer();

    virtual void Tick(float DeltaSeconds) override;

    /** Attach the gizmo to the selected object's world location */
    UFUNCTION(BlueprintCallable, Category = "TOOL|Gizmo")
    void AttachToTarget(AActor* Target);

    /** Hide the gizmo */
    UFUNCTION(BlueprintCallable, Category = "TOOL|Gizmo")
    void Detach();

    /** Update the gizmo to reflect the current mode and active axis */
    UFUNCTION(BlueprintCallable, Category = "TOOL|Gizmo")
    void UpdateGizmoState(EManipulationMode Mode, EManipulationAxis ActiveAxis);

    // --- Visual Settings ---

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TOOL|Gizmo")
    float ArrowLength = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TOOL|Gizmo")
    float ArrowThickness = 2.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TOOL|Gizmo")
    float RingRadius = 80.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TOOL|Gizmo")
    float ScaleHandleSize = 10.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TOOL|Gizmo")
    FLinearColor XColor = FLinearColor::Red;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TOOL|Gizmo")
    FLinearColor YColor = FLinearColor::Green;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TOOL|Gizmo")
    FLinearColor ZColor = FLinearColor(0.3f, 0.4f, 1.f, 1.f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TOOL|Gizmo")
    FLinearColor HighlightColor = FLinearColor::Yellow;

protected:
    virtual void BeginPlay() override;

private:
    void DrawMoveGizmo();
    void DrawRotateGizmo();
    void DrawScaleGizmo();
    void DrawElevateGizmo();

    /** Draw a single axis arrow via debug lines */
    void DrawAxisArrow(FVector Origin, FVector Direction, FLinearColor Color,
        bool bHighlighted) const;

    /** Draw a rotation ring around an axis */
    void DrawRotationRing(FVector Origin, FVector Axis, FLinearColor Color,
        bool bHighlighted) const;

    /** Draw a scale handle cube at the end of an axis */
    void DrawScaleHandle(FVector Position, FLinearColor Color,
        bool bHighlighted) const;

    TWeakObjectPtr<AActor> TargetActor;
    EManipulationMode CurrentMode = EManipulationMode::None;
    EManipulationAxis CurrentActiveAxis = EManipulationAxis::XYZ;
};
