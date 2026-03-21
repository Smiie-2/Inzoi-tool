#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InputActionValue.h"
#include "ObjectManipulator.h"
#include "TOOLInputHandler.generated.h"

class UInputAction;
class UInputMappingContext;
class UEnhancedInputComponent;

/**
 * Handles all keyboard/mouse input for the T.O.O.L. system.
 * Binds Enhanced Input actions to ObjectManipulator operations.
 *
 * Default Keybindings:
 *   F2          - Toggle TOOL on/off
 *   G           - Move mode
 *   R           - Rotate mode
 *   S           - Scale mode
 *   E           - Elevate mode
 *   X/Y/Z       - Constrain to axis
 *   Shift+X/Y/Z - Constrain to plane (other two axes)
 *   Ctrl+Z      - Undo
 *   Ctrl+Y      - Redo
 *   Mouse Drag  - Manipulate along current axis
 *   Scroll      - Fine adjust (elevate in elevate mode, scale in scale mode)
 *   Tab         - Cycle axis
 *   T           - Snap to terrain
 *   C           - Toggle snap camera to object
 *   N           - Enter numeric coordinate input
 *   Escape      - Deselect / Cancel
 *   LMB         - Select object under cursor
 *   Delete      - Reset object to original transform
 */
UCLASS(ClassGroup=(InZoiTOOL), meta=(BlueprintSpawnableComponent))
class INZOITOOL_API UTOOLInputHandler : public UActorComponent
{
    GENERATED_BODY()

public:
    UTOOLInputHandler();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
        FActorComponentTickFunction* ThisTickFunction) override;

    /** Set the manipulator this input handler drives */
    UFUNCTION(BlueprintCallable, Category = "TOOL|Input")
    void SetManipulator(UObjectManipulator* Manipulator);

    // --- Input Mapping Context ---

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TOOL|Input")
    TObjectPtr<UInputMappingContext> TOOLMappingContext;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TOOL|Input")
    int32 MappingContextPriority = 1;

    // --- Input Actions ---

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TOOL|Input|Actions")
    TObjectPtr<UInputAction> IA_ToggleTool;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TOOL|Input|Actions")
    TObjectPtr<UInputAction> IA_Select;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TOOL|Input|Actions")
    TObjectPtr<UInputAction> IA_Deselect;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TOOL|Input|Actions")
    TObjectPtr<UInputAction> IA_MoveMode;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TOOL|Input|Actions")
    TObjectPtr<UInputAction> IA_RotateMode;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TOOL|Input|Actions")
    TObjectPtr<UInputAction> IA_ScaleMode;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TOOL|Input|Actions")
    TObjectPtr<UInputAction> IA_ElevateMode;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TOOL|Input|Actions")
    TObjectPtr<UInputAction> IA_AxisX;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TOOL|Input|Actions")
    TObjectPtr<UInputAction> IA_AxisY;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TOOL|Input|Actions")
    TObjectPtr<UInputAction> IA_AxisZ;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TOOL|Input|Actions")
    TObjectPtr<UInputAction> IA_CycleAxis;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TOOL|Input|Actions")
    TObjectPtr<UInputAction> IA_MouseDrag;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TOOL|Input|Actions")
    TObjectPtr<UInputAction> IA_ScrollAdjust;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TOOL|Input|Actions")
    TObjectPtr<UInputAction> IA_Undo;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TOOL|Input|Actions")
    TObjectPtr<UInputAction> IA_Redo;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TOOL|Input|Actions")
    TObjectPtr<UInputAction> IA_SnapTerrain;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TOOL|Input|Actions")
    TObjectPtr<UInputAction> IA_ToggleCamera;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TOOL|Input|Actions")
    TObjectPtr<UInputAction> IA_NumericInput;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TOOL|Input|Actions")
    TObjectPtr<UInputAction> IA_ResetTransform;

    // --- Settings ---

    /** Mouse drag sensitivity for manipulations */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TOOL|Input|Settings",
        meta = (ClampMin = "0.01", ClampMax = "10.0"))
    float DragSensitivity = 1.0f;

    /** Scroll wheel sensitivity for fine adjustments */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TOOL|Input|Settings",
        meta = (ClampMin = "0.01", ClampMax = "10.0"))
    float ScrollSensitivity = 0.5f;

private:
    void SetupInputBindings();

    // Input callbacks
    void OnToggleTool(const FInputActionValue& Value);
    void OnSelect(const FInputActionValue& Value);
    void OnDeselect(const FInputActionValue& Value);
    void OnSetMoveMode(const FInputActionValue& Value);
    void OnSetRotateMode(const FInputActionValue& Value);
    void OnSetScaleMode(const FInputActionValue& Value);
    void OnSetElevateMode(const FInputActionValue& Value);
    void OnAxisX(const FInputActionValue& Value);
    void OnAxisY(const FInputActionValue& Value);
    void OnAxisZ(const FInputActionValue& Value);
    void OnCycleAxis(const FInputActionValue& Value);
    void OnMouseDrag(const FInputActionValue& Value);
    void OnScrollAdjust(const FInputActionValue& Value);
    void OnUndo(const FInputActionValue& Value);
    void OnRedo(const FInputActionValue& Value);
    void OnSnapTerrain(const FInputActionValue& Value);
    void OnToggleCamera(const FInputActionValue& Value);
    void OnNumericInput(const FInputActionValue& Value);
    void OnResetTransform(const FInputActionValue& Value);

    /** Trace from the mouse cursor into the world to find an actor */
    AActor* TraceObjectUnderCursor() const;

    /** Compute a manipulation delta from a 2D mouse drag */
    FVector ComputeDragDelta(FVector2D ScreenDelta) const;

    UPROPERTY()
    TWeakObjectPtr<UObjectManipulator> LinkedManipulator;

    FVector2D LastMousePosition = FVector2D::ZeroVector;
    bool bIsDragging = false;
    bool bInputBound = false;

    /** Original transform stored for the Delete/Reset action */
    TMap<TWeakObjectPtr<AActor>, FTransform> OriginalTransforms;
};
