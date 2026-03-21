#include "TOOLGizmoRenderer.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"

ATOOLGizmoRenderer::ATOOLGizmoRenderer()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = false;
}

void ATOOLGizmoRenderer::BeginPlay()
{
    Super::BeginPlay();
}

void ATOOLGizmoRenderer::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (!TargetActor.IsValid())
    {
        SetActorTickEnabled(false);
        return;
    }

    // Update gizmo position to follow the target
    SetActorLocation(TargetActor->GetActorLocation());

    // Draw gizmo based on current mode
    switch (CurrentMode)
    {
        case EManipulationMode::Move:    DrawMoveGizmo(); break;
        case EManipulationMode::Rotate:  DrawRotateGizmo(); break;
        case EManipulationMode::Scale:   DrawScaleGizmo(); break;
        case EManipulationMode::Elevate: DrawElevateGizmo(); break;
        default: break;
    }
}

void ATOOLGizmoRenderer::AttachToTarget(AActor* Target)
{
    TargetActor = Target;
    if (Target)
    {
        SetActorLocation(Target->GetActorLocation());
        SetActorTickEnabled(true);
    }
}

void ATOOLGizmoRenderer::Detach()
{
    TargetActor = nullptr;
    SetActorTickEnabled(false);
}

void ATOOLGizmoRenderer::UpdateGizmoState(EManipulationMode Mode,
    EManipulationAxis ActiveAxis)
{
    CurrentMode = Mode;
    CurrentActiveAxis = ActiveAxis;
}

// ============================================================================
// Draw Methods
// ============================================================================

void ATOOLGizmoRenderer::DrawMoveGizmo()
{
    FVector Origin = GetActorLocation();

    bool bXActive = (CurrentActiveAxis == EManipulationAxis::X ||
                     CurrentActiveAxis == EManipulationAxis::XY ||
                     CurrentActiveAxis == EManipulationAxis::XZ ||
                     CurrentActiveAxis == EManipulationAxis::XYZ);
    bool bYActive = (CurrentActiveAxis == EManipulationAxis::Y ||
                     CurrentActiveAxis == EManipulationAxis::XY ||
                     CurrentActiveAxis == EManipulationAxis::YZ ||
                     CurrentActiveAxis == EManipulationAxis::XYZ);
    bool bZActive = (CurrentActiveAxis == EManipulationAxis::Z ||
                     CurrentActiveAxis == EManipulationAxis::XZ ||
                     CurrentActiveAxis == EManipulationAxis::YZ ||
                     CurrentActiveAxis == EManipulationAxis::XYZ);

    DrawAxisArrow(Origin, FVector::ForwardVector, XColor, bXActive);
    DrawAxisArrow(Origin, FVector::RightVector, YColor, bYActive);
    DrawAxisArrow(Origin, FVector::UpVector, ZColor, bZActive);
}

void ATOOLGizmoRenderer::DrawRotateGizmo()
{
    FVector Origin = GetActorLocation();

    bool bXActive = (CurrentActiveAxis == EManipulationAxis::X ||
                     CurrentActiveAxis == EManipulationAxis::XYZ);
    bool bYActive = (CurrentActiveAxis == EManipulationAxis::Y ||
                     CurrentActiveAxis == EManipulationAxis::XYZ);
    bool bZActive = (CurrentActiveAxis == EManipulationAxis::Z ||
                     CurrentActiveAxis == EManipulationAxis::XYZ);

    DrawRotationRing(Origin, FVector::ForwardVector, XColor, bXActive);
    DrawRotationRing(Origin, FVector::RightVector, YColor, bYActive);
    DrawRotationRing(Origin, FVector::UpVector, ZColor, bZActive);
}

void ATOOLGizmoRenderer::DrawScaleGizmo()
{
    FVector Origin = GetActorLocation();

    bool bXActive = (CurrentActiveAxis == EManipulationAxis::X ||
                     CurrentActiveAxis == EManipulationAxis::XYZ);
    bool bYActive = (CurrentActiveAxis == EManipulationAxis::Y ||
                     CurrentActiveAxis == EManipulationAxis::XYZ);
    bool bZActive = (CurrentActiveAxis == EManipulationAxis::Z ||
                     CurrentActiveAxis == EManipulationAxis::XYZ);

    // Draw axis lines with scale handles (cubes) at the ends
    FLinearColor XC = bXActive ? HighlightColor : XColor;
    FLinearColor YC = bYActive ? HighlightColor : YColor;
    FLinearColor ZC = bZActive ? HighlightColor : ZColor;

    FVector XEnd = Origin + FVector::ForwardVector * ArrowLength;
    FVector YEnd = Origin + FVector::RightVector * ArrowLength;
    FVector ZEnd = Origin + FVector::UpVector * ArrowLength;

    DrawDebugLine(GetWorld(), Origin, XEnd, XC.ToFColor(true), false, -1.f,
        0, ArrowThickness);
    DrawDebugLine(GetWorld(), Origin, YEnd, YC.ToFColor(true), false, -1.f,
        0, ArrowThickness);
    DrawDebugLine(GetWorld(), Origin, ZEnd, ZC.ToFColor(true), false, -1.f,
        0, ArrowThickness);

    DrawScaleHandle(XEnd, XC, bXActive);
    DrawScaleHandle(YEnd, YC, bYActive);
    DrawScaleHandle(ZEnd, ZC, bZActive);
}

void ATOOLGizmoRenderer::DrawElevateGizmo()
{
    FVector Origin = GetActorLocation();

    bool bActive = (CurrentActiveAxis == EManipulationAxis::Z ||
                    CurrentActiveAxis == EManipulationAxis::XYZ);

    // Draw a vertical line with arrows at both ends
    FLinearColor Color = bActive ? HighlightColor : ZColor;
    FColor FlatColor = Color.ToFColor(true);

    FVector Top = Origin + FVector::UpVector * ArrowLength;
    FVector Bottom = Origin - FVector::UpVector * ArrowLength * 0.5f;

    DrawDebugLine(GetWorld(), Bottom, Top, FlatColor, false, -1.f,
        0, ArrowThickness * 1.5f);

    // Draw arrowheads
    float HeadSize = ArrowLength * 0.1f;
    DrawDebugLine(GetWorld(), Top,
        Top - FVector::UpVector * HeadSize + FVector::ForwardVector * HeadSize,
        FlatColor, false, -1.f, 0, ArrowThickness);
    DrawDebugLine(GetWorld(), Top,
        Top - FVector::UpVector * HeadSize - FVector::ForwardVector * HeadSize,
        FlatColor, false, -1.f, 0, ArrowThickness);

    // Draw a ground plane indicator (dashed circle at origin)
    const int32 Segments = 32;
    for (int32 i = 0; i < Segments; ++i)
    {
        float Angle0 = (2.f * PI * i) / Segments;
        float Angle1 = (2.f * PI * (i + 1)) / Segments;

        if (i % 2 == 0) // dashed
        {
            FVector P0 = Origin + FVector(
                FMath::Cos(Angle0) * RingRadius * 0.5f,
                FMath::Sin(Angle0) * RingRadius * 0.5f, 0.f);
            FVector P1 = Origin + FVector(
                FMath::Cos(Angle1) * RingRadius * 0.5f,
                FMath::Sin(Angle1) * RingRadius * 0.5f, 0.f);
            DrawDebugLine(GetWorld(), P0, P1, FlatColor, false, -1.f,
                0, ArrowThickness * 0.5f);
        }
    }
}

// ============================================================================
// Primitive Drawing Helpers
// ============================================================================

void ATOOLGizmoRenderer::DrawAxisArrow(FVector Origin, FVector Direction,
    FLinearColor Color, bool bHighlighted) const
{
    FLinearColor DrawColor = bHighlighted ? HighlightColor : Color;
    FColor FlatColor = DrawColor.ToFColor(true);
    float Thickness = bHighlighted ? ArrowThickness * 2.f : ArrowThickness;

    FVector End = Origin + Direction * ArrowLength;

    // Main line
    DrawDebugLine(GetWorld(), Origin, End, FlatColor, false, -1.f, 0, Thickness);

    // Arrowhead
    float HeadSize = ArrowLength * 0.1f;
    FVector Right = FVector::CrossProduct(Direction, FVector::UpVector);
    if (Right.IsNearlyZero())
        Right = FVector::CrossProduct(Direction, FVector::ForwardVector);
    Right.Normalize();

    DrawDebugLine(GetWorld(), End, End - Direction * HeadSize + Right * HeadSize,
        FlatColor, false, -1.f, 0, Thickness);
    DrawDebugLine(GetWorld(), End, End - Direction * HeadSize - Right * HeadSize,
        FlatColor, false, -1.f, 0, Thickness);
}

void ATOOLGizmoRenderer::DrawRotationRing(FVector Origin, FVector Axis,
    FLinearColor Color, bool bHighlighted) const
{
    FLinearColor DrawColor = bHighlighted ? HighlightColor : Color;
    FColor FlatColor = DrawColor.ToFColor(true);
    float Thickness = bHighlighted ? ArrowThickness * 2.f : ArrowThickness;

    // Compute two vectors perpendicular to the axis
    FVector Perp1, Perp2;
    Axis.FindBestAxisVectors(Perp1, Perp2);

    const int32 Segments = 48;
    for (int32 i = 0; i < Segments; ++i)
    {
        float Angle0 = (2.f * PI * i) / Segments;
        float Angle1 = (2.f * PI * (i + 1)) / Segments;

        FVector P0 = Origin + (Perp1 * FMath::Cos(Angle0) +
            Perp2 * FMath::Sin(Angle0)) * RingRadius;
        FVector P1 = Origin + (Perp1 * FMath::Cos(Angle1) +
            Perp2 * FMath::Sin(Angle1)) * RingRadius;

        DrawDebugLine(GetWorld(), P0, P1, FlatColor, false, -1.f, 0, Thickness);
    }
}

void ATOOLGizmoRenderer::DrawScaleHandle(FVector Position, FLinearColor Color,
    bool bHighlighted) const
{
    FLinearColor DrawColor = bHighlighted ? HighlightColor : Color;
    float Size = bHighlighted ? ScaleHandleSize * 1.5f : ScaleHandleSize;

    DrawDebugBox(GetWorld(), Position, FVector(Size), DrawColor.ToFColor(true),
        false, -1.f, 0, ArrowThickness);
}
