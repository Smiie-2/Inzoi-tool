#include "TOOLWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/CanvasPanel.h"
#include "Components/Border.h"

void UTOOLWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Bind button clicks
    if (ApplyButton)
        ApplyButton->OnClicked.AddDynamic(this, &UTOOLWidget::OnApplyClicked);
    if (UndoButton)
        UndoButton->OnClicked.AddDynamic(this, &UTOOLWidget::OnUndoClicked);
    if (RedoButton)
        RedoButton->OnClicked.AddDynamic(this, &UTOOLWidget::OnRedoClicked);
    if (SnapTerrainButton)
        SnapTerrainButton->OnClicked.AddDynamic(this, &UTOOLWidget::OnSnapTerrainClicked);
    if (ToggleCameraButton)
        ToggleCameraButton->OnClicked.AddDynamic(this, &UTOOLWidget::OnToggleCameraClicked);

    // Set initial help text
    if (HelpText)
    {
        HelpText->SetText(FText::FromString(
            TEXT("G=Move  R=Rotate  S=Scale  E=Elevate\n")
            TEXT("X/Y/Z=Axis  Tab=Cycle  T=Terrain\n")
            TEXT("Ctrl+Z=Undo  Ctrl+Y=Redo  N=Numeric\n")
            TEXT("LMB=Select  Esc=Deselect  Del=Reset")
        ));
    }

    // Default title
    if (TitleText)
    {
        TitleText->SetText(FText::FromString(TEXT("InZoi T.O.O.L.")));
    }
}

void UTOOLWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    if (bIsVisible)
    {
        RefreshDisplay();
    }
}

void UTOOLWidget::InitializeWithManipulator(UObjectManipulator* Manipulator)
{
    LinkedManipulator = Manipulator;
}

void UTOOLWidget::RefreshDisplay()
{
    if (!LinkedManipulator.IsValid()) return;

    // Mode
    if (ModeText)
    {
        ModeText->SetText(FText::FromString(
            GetModeDisplayName(LinkedManipulator->GetMode())));
    }

    // Axis
    if (AxisText)
    {
        AxisText->SetText(FText::FromString(
            GetAxisDisplayName(LinkedManipulator->GetAxis())));
    }

    // Selected object info
    AActor* Selected = LinkedManipulator->GetSelectedObject();
    if (Selected)
    {
        if (ObjectNameText)
            ObjectNameText->SetText(FText::FromString(Selected->GetName()));

        if (PositionText)
            PositionText->SetText(FText::FromString(
                FString::Printf(TEXT("Pos: %s"),
                    *FormatVector(Selected->GetActorLocation()))));

        if (RotationText)
            RotationText->SetText(FText::FromString(
                FString::Printf(TEXT("Rot: %s"),
                    *FormatRotator(Selected->GetActorRotation()))));

        if (ScaleText)
            ScaleText->SetText(FText::FromString(
                FString::Printf(TEXT("Scale: %s"),
                    *FormatVector(Selected->GetActorScale3D()))));

        if (StatusText)
            StatusText->SetText(FText::FromString(TEXT("Object Selected")));
    }
    else
    {
        if (ObjectNameText)
            ObjectNameText->SetText(FText::FromString(TEXT("No Selection")));
        if (PositionText)
            PositionText->SetText(FText::GetEmpty());
        if (RotationText)
            RotationText->SetText(FText::GetEmpty());
        if (ScaleText)
            ScaleText->SetText(FText::GetEmpty());
        if (StatusText)
            StatusText->SetText(FText::FromString(
                TEXT("Click an object to select it")));
    }

    // Update button states
    if (UndoButton)
        UndoButton->SetIsEnabled(LinkedManipulator->CanUndo());
    if (RedoButton)
        RedoButton->SetIsEnabled(LinkedManipulator->CanRedo());
}

void UTOOLWidget::ShowTool()
{
    bIsVisible = true;
    SetVisibility(ESlateVisibility::Visible);
    if (FadeIn)
    {
        PlayAnimation(FadeIn);
    }
}

void UTOOLWidget::HideTool()
{
    bIsVisible = false;
    if (FadeOut)
    {
        PlayAnimation(FadeOut);
    }
    else
    {
        SetVisibility(ESlateVisibility::Collapsed);
    }
}

void UTOOLWidget::OpenCoordinateInput()
{
    if (CoordinateInputBox)
    {
        CoordinateInputBox->SetVisibility(ESlateVisibility::Visible);
        CoordinateInputBox->SetKeyboardFocus();
        CoordinateInputBox->SetText(FText::GetEmpty());
    }
}

void UTOOLWidget::SubmitCoordinateInput(const FString& Input)
{
    if (!LinkedManipulator.IsValid()) return;

    switch (LinkedManipulator->GetMode())
    {
        case EManipulationMode::Move:
            LinkedManipulator->MoveObjectByInput(Input);
            break;

        case EManipulationMode::Rotate:
        {
            float Angle = FCString::Atof(*Input);
            LinkedManipulator->RotateObjectByAngle(Angle);
            break;
        }

        case EManipulationMode::Scale:
        {
            float Factor = FCString::Atof(*Input);
            if (Factor > 0.f)
                LinkedManipulator->ScaleObjectUniform(Factor);
            break;
        }

        case EManipulationMode::Elevate:
        {
            float Height = FCString::Atof(*Input);
            LinkedManipulator->ElevateObject(Height);
            break;
        }

        default:
            break;
    }

    // Hide input box after submission
    if (CoordinateInputBox)
    {
        CoordinateInputBox->SetVisibility(ESlateVisibility::Collapsed);
    }
}

// ============================================================================
// Draggable Window
// ============================================================================

FReply UTOOLWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry,
    const FPointerEvent& InMouseEvent)
{
    if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
    {
        // Check if clicking on the title bar for dragging
        if (TitleBar)
        {
            FGeometry TitleGeometry = TitleBar->GetCachedGeometry();
            FVector2D LocalPos = TitleGeometry.AbsoluteToLocal(
                InMouseEvent.GetScreenSpacePosition());

            if (LocalPos.X >= 0 && LocalPos.Y >= 0 &&
                LocalPos.X <= TitleGeometry.GetLocalSize().X &&
                LocalPos.Y <= TitleGeometry.GetLocalSize().Y)
            {
                bIsDraggingWindow = true;
                DragOffset = InMouseEvent.GetScreenSpacePosition() -
                    GetCachedGeometry().GetAbsolutePosition();
                return FReply::Handled().CaptureMouse(SharedThis(this));
            }
        }
    }

    return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

FReply UTOOLWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry,
    const FPointerEvent& InMouseEvent)
{
    if (bIsDraggingWindow)
    {
        bIsDraggingWindow = false;
        return FReply::Handled().ReleaseMouseCapture();
    }

    return Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);
}

FReply UTOOLWidget::NativeOnMouseMove(const FGeometry& InGeometry,
    const FPointerEvent& InMouseEvent)
{
    if (bIsDraggingWindow)
    {
        FVector2D NewPosition = InMouseEvent.GetScreenSpacePosition() - DragOffset;
        SetPositionInViewport(NewPosition);
        return FReply::Handled();
    }

    return Super::NativeOnMouseMove(InGeometry, InMouseEvent);
}

// ============================================================================
// Button Callbacks
// ============================================================================

void UTOOLWidget::OnApplyClicked()
{
    if (CoordinateInputBox)
    {
        SubmitCoordinateInput(CoordinateInputBox->GetText().ToString());
    }
}

void UTOOLWidget::OnUndoClicked()
{
    if (LinkedManipulator.IsValid())
        LinkedManipulator->Undo();
}

void UTOOLWidget::OnRedoClicked()
{
    if (LinkedManipulator.IsValid())
        LinkedManipulator->Redo();
}

void UTOOLWidget::OnSnapTerrainClicked()
{
    if (LinkedManipulator.IsValid())
        LinkedManipulator->SnapToTerrain();
}

void UTOOLWidget::OnToggleCameraClicked()
{
    if (LinkedManipulator.IsValid())
    {
        LinkedManipulator->bSnapCameraToObject =
            !LinkedManipulator->bSnapCameraToObject;
    }
}

// ============================================================================
// Formatting Helpers
// ============================================================================

FString UTOOLWidget::FormatVector(const FVector& Vec) const
{
    return FString::Printf(TEXT("X: %.2f  Y: %.2f  Z: %.2f"),
        Vec.X, Vec.Y, Vec.Z);
}

FString UTOOLWidget::FormatRotator(const FRotator& Rot) const
{
    return FString::Printf(TEXT("P: %.1f  Y: %.1f  R: %.1f"),
        Rot.Pitch, Rot.Yaw, Rot.Roll);
}

FString UTOOLWidget::GetModeDisplayName(EManipulationMode Mode) const
{
    switch (Mode)
    {
        case EManipulationMode::Move:    return TEXT("MOVE");
        case EManipulationMode::Rotate:  return TEXT("ROTATE");
        case EManipulationMode::Scale:   return TEXT("SCALE");
        case EManipulationMode::Elevate: return TEXT("ELEVATE");
        default:                         return TEXT("NONE");
    }
}

FString UTOOLWidget::GetAxisDisplayName(EManipulationAxis Axis) const
{
    switch (Axis)
    {
        case EManipulationAxis::X:   return TEXT("X");
        case EManipulationAxis::Y:   return TEXT("Y");
        case EManipulationAxis::Z:   return TEXT("Z");
        case EManipulationAxis::XY:  return TEXT("XY");
        case EManipulationAxis::XZ:  return TEXT("XZ");
        case EManipulationAxis::YZ:  return TEXT("YZ");
        case EManipulationAxis::XYZ: return TEXT("FREE");
        default:                     return TEXT("---");
    }
}
