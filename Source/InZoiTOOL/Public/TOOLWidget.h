#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ObjectManipulator.h"
#include "TOOLWidget.generated.h"

class UTextBlock;
class UButton;
class UEditableTextBox;
class UImage;
class UCanvasPanel;
class UBorder;
class UWidgetAnimation;

/**
 * Main HUD widget for the T.O.O.L. overlay.
 * Displays manipulation mode, axis, object info, coordinate input,
 * and visual gizmo indicators.
 *
 * The widget is draggable - click and hold the title bar to reposition.
 */
UCLASS()
class INZOITOOL_API UTOOLWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    /** Initialize the widget with a reference to the manipulator */
    UFUNCTION(BlueprintCallable, Category = "TOOL|UI")
    void InitializeWithManipulator(UObjectManipulator* Manipulator);

    /** Refresh all display fields from current manipulator state */
    UFUNCTION(BlueprintCallable, Category = "TOOL|UI")
    void RefreshDisplay();

    /** Show/hide the widget with animation */
    UFUNCTION(BlueprintCallable, Category = "TOOL|UI")
    void ShowTool();

    UFUNCTION(BlueprintCallable, Category = "TOOL|UI")
    void HideTool();

    UFUNCTION(BlueprintPure, Category = "TOOL|UI")
    bool IsToolVisible() const { return bIsVisible; }

    /** Open the numeric coordinate input dialog */
    UFUNCTION(BlueprintCallable, Category = "TOOL|UI")
    void OpenCoordinateInput();

    /** Process coordinate text and apply to the current mode */
    UFUNCTION(BlueprintCallable, Category = "TOOL|UI")
    void SubmitCoordinateInput(const FString& Input);

    // --- Gizmo Visualizer Colors ---

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TOOL|UI|Colors")
    FLinearColor XAxisColor = FLinearColor(1.f, 0.2f, 0.2f, 1.f); // Red

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TOOL|UI|Colors")
    FLinearColor YAxisColor = FLinearColor(0.2f, 1.f, 0.2f, 1.f); // Green

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TOOL|UI|Colors")
    FLinearColor ZAxisColor = FLinearColor(0.3f, 0.4f, 1.f, 1.f); // Blue

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TOOL|UI|Colors")
    FLinearColor ActiveAxisColor = FLinearColor(1.f, 1.f, 0.f, 1.f); // Yellow

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TOOL|UI|Colors")
    FLinearColor SelectionHighlightColor = FLinearColor(0.f, 0.8f, 1.f, 0.5f);

protected:
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
    virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry,
        const FPointerEvent& InMouseEvent) override;
    virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry,
        const FPointerEvent& InMouseEvent) override;
    virtual FReply NativeOnMouseMove(const FGeometry& InGeometry,
        const FPointerEvent& InMouseEvent) override;

    // --- Bound UI Widgets ---

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UCanvasPanel> RootCanvas;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UBorder> TitleBar;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> TitleText;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> ModeText;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> AxisText;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> ObjectNameText;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> PositionText;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> RotationText;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> ScaleText;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UEditableTextBox> CoordinateInputBox;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> ApplyButton;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> UndoButton;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> RedoButton;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> SnapTerrainButton;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> ToggleCameraButton;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> HelpText;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> StatusText;

    /** Optional fade-in animation (created in Widget Blueprint) */
    UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
    TObjectPtr<UWidgetAnimation> FadeIn;

    /** Optional fade-out animation (created in Widget Blueprint) */
    UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
    TObjectPtr<UWidgetAnimation> FadeOut;

private:
    UFUNCTION()
    void OnApplyClicked();

    UFUNCTION()
    void OnUndoClicked();

    UFUNCTION()
    void OnRedoClicked();

    UFUNCTION()
    void OnSnapTerrainClicked();

    UFUNCTION()
    void OnToggleCameraClicked();

    /** Format a vector as "X: 0.00  Y: 0.00  Z: 0.00" */
    FString FormatVector(const FVector& Vec) const;

    /** Format a rotator as "P: 0.00  Y: 0.00  R: 0.00" */
    FString FormatRotator(const FRotator& Rot) const;

    /** Get display name for the current manipulation mode */
    FString GetModeDisplayName(EManipulationMode Mode) const;

    /** Get display name for the current axis constraint */
    FString GetAxisDisplayName(EManipulationAxis Axis) const;

    UPROPERTY()
    TWeakObjectPtr<UObjectManipulator> LinkedManipulator;

    bool bIsVisible = false;
    bool bIsDraggingWindow = false;
    FVector2D DragOffset = FVector2D::ZeroVector;
};
