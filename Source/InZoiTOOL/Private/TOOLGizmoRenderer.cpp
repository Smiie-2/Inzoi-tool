#include "TOOLGizmoRenderer.h"
#include "Components/LineBatchComponent.h"
#include "Engine/World.h"

ATOOLGizmoRenderer::ATOOLGizmoRenderer()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = false;

    // LineBatchComponent renders lines in all build configs (unlike DrawDebugLine)
    LineBatcher = CreateDefaultSubobject<ULineBatchComponent>(TEXT("GizmoLines"));
    RootComponent = LineBatcher;
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

    // Clear previous frame's lines
    if (LineBatcher)
    {
        LineBatcher->Flush();
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
        SetActorHiddenInGame(false);
    }
}

void ATOOLGizmoRenderer::Detach()
{
    TargetActor = nullptr;
    SetActorTickEnabled(false);
    SetActorHiddenInGame(true);
    if (LineBatcher)
    {
        LineBatcher->Flush();
    }
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

    FLinearColor XC = bXActive ? HighlightColor : XColor;
    FLinearColor YC = bYActive ? HighlightColor : YColor;
    FLinearColor ZC = bZActive ? HighlightColor : ZColor;

    FVector XEnd = Origin + FVector::ForwardVector * ArrowLength;
    FVector YEnd = Origin + FVector::RightVector * ArrowLength;
    FVector ZEnd = Origin + FVector::UpVector * ArrowLength;

    DrawLine(Origin, XEnd, XC.ToFColor(true), ArrowThickness);
    DrawLine(Origin, YEnd, YC.ToFColor(true), ArrowThickness);
    DrawLine(Origin, ZEnd, ZC.ToFColor(true), ArrowThickness);

    DrawScaleHandle(XEnd, XC, bXActive);
    DrawScaleHandle(YEnd, YC, bYActive);
    DrawScaleHandle(ZEnd, ZC, bZActive);
}

void ATOOLGizmoRenderer::DrawElevateGizmo()
{
    FVector Origin = GetActorLocation();

    bool bActive = (CurrentActiveAxis == EManipulationAxis::Z ||
                    CurrentActiveAxis == EManipulationAxis::XYZ);

    FLinearColor Color = bActive ? HighlightColor : ZColor;
    FColor FlatColor = Color.ToFColor(true);

    FVector Top = Origin + FVector::UpVector * ArrowLength;
    FVector Bottom = Origin - FVector::UpVector * ArrowLength * 0.5f;

    DrawLine(Bottom, Top, FlatColor, ArrowThickness * 1.5f);

    // Draw arrowheads
    float HeadSize = ArrowLength * 0.1f;
    DrawLine(Top, Top - FVector::UpVector * HeadSize + FVector::ForwardVector * HeadSize,
        FlatColor, ArrowThickness);
    DrawLine(Top, Top - FVector::UpVector * HeadSize - FVector::ForwardVector * HeadSize,
        FlatColor, ArrowThickness);

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
            DrawLine(P0, P1, FlatColor, ArrowThickness * 0.5f);
        }
    }
}

// ============================================================================
// Primitive Drawing Helpers (using LineBatchComponent for shipping builds)
// ============================================================================

void ATOOLGizmoRenderer::DrawAxisArrow(FVector Origin, FVector Direction,
    FLinearColor Color, bool bHighlighted)
{
    FLinearColor DrawColor = bHighlighted ? HighlightColor : Color;
    FColor FlatColor = DrawColor.ToFColor(true);
    float Thickness = bHighlighted ? ArrowThickness * 2.f : ArrowThickness;

    FVector End = Origin + Direction * ArrowLength;

    // Main line
    DrawLine(Origin, End, FlatColor, Thickness);

    // Arrowhead
    float HeadSize = ArrowLength * 0.1f;
    FVector Right = FVector::CrossProduct(Direction, FVector::UpVector);
    if (Right.IsNearlyZero())
        Right = FVector::CrossProduct(Direction, FVector::ForwardVector);
    Right.Normalize();

    DrawLine(End, End - Direction * HeadSize + Right * HeadSize,
        FlatColor, Thickness);
    DrawLine(End, End - Direction * HeadSize - Right * HeadSize,
        FlatColor, Thickness);
}

void ATOOLGizmoRenderer::DrawRotationRing(FVector Origin, FVector Axis,
    FLinearColor Color, bool bHighlighted)
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

        DrawLine(P0, P1, FlatColor, Thickness);
    }
}

void ATOOLGizmoRenderer::DrawScaleHandle(FVector Position, FLinearColor Color,
    bool bHighlighted)
{
    FLinearColor DrawColor = bHighlighted ? HighlightColor : Color;
    float Size = bHighlighted ? ScaleHandleSize * 1.5f : ScaleHandleSize;

    DrawBox(Position, FVector(Size), DrawColor.ToFColor(true), ArrowThickness);
}

void ATOOLGizmoRenderer::DrawLine(FVector Start, FVector End, FColor Color,
    float Thickness)
{
    if (LineBatcher)
    {
        LineBatcher->DrawLine(Start, End, Color, 0, Thickness, 0.f);
    }
}

void ATOOLGizmoRenderer::DrawBox(FVector Center, FVector Extent, FColor Color,
    float Thickness)
{
    if (!LineBatcher) return;

    // Draw 12 edges of a box
    FVector Min = Center - Extent;
    FVector Max = Center + Extent;

    FVector Corners[8] = {
        FVector(Min.X, Min.Y, Min.Z), FVector(Max.X, Min.Y, Min.Z),
        FVector(Max.X, Max.Y, Min.Z), FVector(Min.X, Max.Y, Min.Z),
        FVector(Min.X, Min.Y, Max.Z), FVector(Max.X, Min.Y, Max.Z),
        FVector(Max.X, Max.Y, Max.Z), FVector(Min.X, Max.Y, Max.Z),
    };

    // Bottom face
    DrawLine(Corners[0], Corners[1], Color, Thickness);
    DrawLine(Corners[1], Corners[2], Color, Thickness);
    DrawLine(Corners[2], Corners[3], Color, Thickness);
    DrawLine(Corners[3], Corners[0], Color, Thickness);
    // Top face
    DrawLine(Corners[4], Corners[5], Color, Thickness);
    DrawLine(Corners[5], Corners[6], Color, Thickness);
    DrawLine(Corners[6], Corners[7], Color, Thickness);
    DrawLine(Corners[7], Corners[4], Color, Thickness);
    // Verticals
    DrawLine(Corners[0], Corners[4], Color, Thickness);
    DrawLine(Corners[1], Corners[5], Color, Thickness);
    DrawLine(Corners[2], Corners[6], Color, Thickness);
    DrawLine(Corners[3], Corners[7], Color, Thickness);
}
