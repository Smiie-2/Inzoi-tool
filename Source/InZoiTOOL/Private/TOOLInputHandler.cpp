#include "TOOLInputHandler.h"
#include "InZoiTOOLModule.h"
#include "TOOLSettings.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"
#include "Camera/PlayerCameraManager.h"

UTOOLInputHandler::UTOOLInputHandler()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UTOOLInputHandler::BeginPlay()
{
    Super::BeginPlay();

    // Apply settings
    if (const UTOOLSettings* Settings = UTOOLSettings::Get())
    {
        DragSensitivity = Settings->MoveSpeed;
    }

    // Input bindings are deferred to the first tick to ensure the
    // PlayerController is fully initialized (may not be ready in BeginPlay)
}

void UTOOLInputHandler::TickComponent(float DeltaTime, ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Deferred input setup - wait for player controller to be ready
    if (!bInputBound)
    {
        if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
        {
            if (PC->InputComponent)
            {
                SetupInputBindings();
                bInputBound = true;
            }
        }
        return; // Skip tracking until input is bound
    }

    // Track mouse position for drag calculations
    if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
    {
        float MouseX, MouseY;
        if (PC->GetMousePosition(MouseX, MouseY))
        {
            LastMousePosition = FVector2D(MouseX, MouseY);
        }
    }
}

void UTOOLInputHandler::SetManipulator(UObjectManipulator* Manipulator)
{
    LinkedManipulator = Manipulator;
}

void UTOOLInputHandler::SetupInputBindings()
{
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC) return;

    // Add the TOOL mapping context
    if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
        ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
    {
        if (TOOLMappingContext)
        {
            Subsystem->AddMappingContext(TOOLMappingContext, MappingContextPriority);
        }
    }

    // Bind input actions
    if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PC->InputComponent))
    {
        if (IA_ToggleTool)
            EIC->BindAction(IA_ToggleTool, ETriggerEvent::Started, this,
                &UTOOLInputHandler::OnToggleTool);
        if (IA_Select)
            EIC->BindAction(IA_Select, ETriggerEvent::Started, this,
                &UTOOLInputHandler::OnSelect);
        if (IA_Deselect)
            EIC->BindAction(IA_Deselect, ETriggerEvent::Started, this,
                &UTOOLInputHandler::OnDeselect);
        if (IA_MoveMode)
            EIC->BindAction(IA_MoveMode, ETriggerEvent::Started, this,
                &UTOOLInputHandler::OnSetMoveMode);
        if (IA_RotateMode)
            EIC->BindAction(IA_RotateMode, ETriggerEvent::Started, this,
                &UTOOLInputHandler::OnSetRotateMode);
        if (IA_ScaleMode)
            EIC->BindAction(IA_ScaleMode, ETriggerEvent::Started, this,
                &UTOOLInputHandler::OnSetScaleMode);
        if (IA_ElevateMode)
            EIC->BindAction(IA_ElevateMode, ETriggerEvent::Started, this,
                &UTOOLInputHandler::OnSetElevateMode);
        if (IA_AxisX)
            EIC->BindAction(IA_AxisX, ETriggerEvent::Started, this,
                &UTOOLInputHandler::OnAxisX);
        if (IA_AxisY)
            EIC->BindAction(IA_AxisY, ETriggerEvent::Started, this,
                &UTOOLInputHandler::OnAxisY);
        if (IA_AxisZ)
            EIC->BindAction(IA_AxisZ, ETriggerEvent::Started, this,
                &UTOOLInputHandler::OnAxisZ);
        if (IA_CycleAxis)
            EIC->BindAction(IA_CycleAxis, ETriggerEvent::Started, this,
                &UTOOLInputHandler::OnCycleAxis);
        if (IA_MouseDrag)
            EIC->BindAction(IA_MouseDrag, ETriggerEvent::Triggered, this,
                &UTOOLInputHandler::OnMouseDrag);
        if (IA_ScrollAdjust)
            EIC->BindAction(IA_ScrollAdjust, ETriggerEvent::Triggered, this,
                &UTOOLInputHandler::OnScrollAdjust);
        if (IA_Undo)
            EIC->BindAction(IA_Undo, ETriggerEvent::Started, this,
                &UTOOLInputHandler::OnUndo);
        if (IA_Redo)
            EIC->BindAction(IA_Redo, ETriggerEvent::Started, this,
                &UTOOLInputHandler::OnRedo);
        if (IA_SnapTerrain)
            EIC->BindAction(IA_SnapTerrain, ETriggerEvent::Started, this,
                &UTOOLInputHandler::OnSnapTerrain);
        if (IA_ToggleCamera)
            EIC->BindAction(IA_ToggleCamera, ETriggerEvent::Started, this,
                &UTOOLInputHandler::OnToggleCamera);
        if (IA_NumericInput)
            EIC->BindAction(IA_NumericInput, ETriggerEvent::Started, this,
                &UTOOLInputHandler::OnNumericInput);
        if (IA_ResetTransform)
            EIC->BindAction(IA_ResetTransform, ETriggerEvent::Started, this,
                &UTOOLInputHandler::OnResetTransform);
    }
}

// ============================================================================
// Input Callbacks
// ============================================================================

void UTOOLInputHandler::OnToggleTool(const FInputActionValue& Value)
{
    FInZoiTOOLModule::Get().ToggleTool();
}

void UTOOLInputHandler::OnSelect(const FInputActionValue& Value)
{
    if (!FInZoiTOOLModule::Get().bIsToolActive) return;
    if (!LinkedManipulator.IsValid()) return;

    AActor* HitActor = TraceObjectUnderCursor();
    if (HitActor)
    {
        // Store original transform for reset
        OriginalTransforms.FindOrAdd(HitActor) = HitActor->GetActorTransform();
        LinkedManipulator->SelectObject(HitActor);
    }
}

void UTOOLInputHandler::OnDeselect(const FInputActionValue& Value)
{
    if (!LinkedManipulator.IsValid()) return;
    LinkedManipulator->DeselectObject();
}

void UTOOLInputHandler::OnSetMoveMode(const FInputActionValue& Value)
{
    if (!LinkedManipulator.IsValid()) return;
    LinkedManipulator->SetMode(EManipulationMode::Move);
}

void UTOOLInputHandler::OnSetRotateMode(const FInputActionValue& Value)
{
    if (!LinkedManipulator.IsValid()) return;
    LinkedManipulator->SetMode(EManipulationMode::Rotate);
}

void UTOOLInputHandler::OnSetScaleMode(const FInputActionValue& Value)
{
    if (!LinkedManipulator.IsValid()) return;
    LinkedManipulator->SetMode(EManipulationMode::Scale);
}

void UTOOLInputHandler::OnSetElevateMode(const FInputActionValue& Value)
{
    if (!LinkedManipulator.IsValid()) return;
    LinkedManipulator->SetMode(EManipulationMode::Elevate);
}

void UTOOLInputHandler::OnAxisX(const FInputActionValue& Value)
{
    if (!LinkedManipulator.IsValid()) return;
    LinkedManipulator->SetAxis(EManipulationAxis::X);
}

void UTOOLInputHandler::OnAxisY(const FInputActionValue& Value)
{
    if (!LinkedManipulator.IsValid()) return;
    LinkedManipulator->SetAxis(EManipulationAxis::Y);
}

void UTOOLInputHandler::OnAxisZ(const FInputActionValue& Value)
{
    if (!LinkedManipulator.IsValid()) return;
    LinkedManipulator->SetAxis(EManipulationAxis::Z);
}

void UTOOLInputHandler::OnCycleAxis(const FInputActionValue& Value)
{
    if (!LinkedManipulator.IsValid()) return;

    EManipulationAxis Current = LinkedManipulator->GetAxis();
    EManipulationAxis Next;

    switch (Current)
    {
        case EManipulationAxis::XYZ: Next = EManipulationAxis::X; break;
        case EManipulationAxis::X:   Next = EManipulationAxis::Y; break;
        case EManipulationAxis::Y:   Next = EManipulationAxis::Z; break;
        case EManipulationAxis::Z:   Next = EManipulationAxis::XY; break;
        case EManipulationAxis::XY:  Next = EManipulationAxis::XZ; break;
        case EManipulationAxis::XZ:  Next = EManipulationAxis::YZ; break;
        case EManipulationAxis::YZ:  Next = EManipulationAxis::XYZ; break;
        default:                     Next = EManipulationAxis::XYZ; break;
    }

    LinkedManipulator->SetAxis(Next);
}

void UTOOLInputHandler::OnMouseDrag(const FInputActionValue& Value)
{
    if (!FInZoiTOOLModule::Get().bIsToolActive) return;
    if (!LinkedManipulator.IsValid()) return;
    if (!LinkedManipulator->GetSelectedObject()) return;

    FVector2D MouseDelta = Value.Get<FVector2D>();
    FVector WorldDelta = ComputeDragDelta(MouseDelta);

    switch (LinkedManipulator->GetMode())
    {
        case EManipulationMode::Move:
            LinkedManipulator->MoveObject(WorldDelta * DragSensitivity);
            break;

        case EManipulationMode::Rotate:
        {
            float RotAmount = MouseDelta.X * DragSensitivity;
            LinkedManipulator->RotateObjectByAngle(RotAmount);
            break;
        }

        case EManipulationMode::Scale:
        {
            float ScaleFactor = 1.0f + (MouseDelta.X * 0.01f * DragSensitivity);
            ScaleFactor = FMath::Max(ScaleFactor, 0.01f);
            LinkedManipulator->ScaleObjectUniform(ScaleFactor);
            break;
        }

        case EManipulationMode::Elevate:
            LinkedManipulator->ElevateObject(-MouseDelta.Y * DragSensitivity);
            break;

        default:
            break;
    }
}

void UTOOLInputHandler::OnScrollAdjust(const FInputActionValue& Value)
{
    if (!FInZoiTOOLModule::Get().bIsToolActive) return;
    if (!LinkedManipulator.IsValid()) return;
    if (!LinkedManipulator->GetSelectedObject()) return;

    float ScrollValue = Value.Get<float>() * ScrollSensitivity;

    const UTOOLSettings* Settings = UTOOLSettings::Get();
    float ElevStep = Settings ? Settings->ElevationStep : 0.5f;

    switch (LinkedManipulator->GetMode())
    {
        case EManipulationMode::Elevate:
            LinkedManipulator->ElevateObject(ScrollValue * ElevStep);
            break;

        case EManipulationMode::Scale:
        {
            float ScaleFactor = 1.0f + (ScrollValue * 0.05f);
            LinkedManipulator->ScaleObjectUniform(ScaleFactor);
            break;
        }

        case EManipulationMode::Rotate:
            LinkedManipulator->RotateObjectByAngle(ScrollValue * 5.f);
            break;

        case EManipulationMode::Move:
            LinkedManipulator->ElevateObject(ScrollValue * ElevStep);
            break;

        default:
            break;
    }
}

void UTOOLInputHandler::OnUndo(const FInputActionValue& Value)
{
    if (!LinkedManipulator.IsValid()) return;
    LinkedManipulator->Undo();
}

void UTOOLInputHandler::OnRedo(const FInputActionValue& Value)
{
    if (!LinkedManipulator.IsValid()) return;
    LinkedManipulator->Redo();
}

void UTOOLInputHandler::OnSnapTerrain(const FInputActionValue& Value)
{
    if (!LinkedManipulator.IsValid()) return;
    LinkedManipulator->SnapToTerrain();
}

void UTOOLInputHandler::OnToggleCamera(const FInputActionValue& Value)
{
    if (!LinkedManipulator.IsValid()) return;
    LinkedManipulator->bSnapCameraToObject = !LinkedManipulator->bSnapCameraToObject;
}

void UTOOLInputHandler::OnNumericInput(const FInputActionValue& Value)
{
    // This triggers the UI to open the coordinate input box
    // The actual numeric processing happens through TOOLWidget::SubmitCoordinateInput
    UE_LOG(LogTemp, Log, TEXT("[InZoi TOOL] Numeric input mode activated"));
}

void UTOOLInputHandler::OnResetTransform(const FInputActionValue& Value)
{
    if (!LinkedManipulator.IsValid()) return;

    AActor* Selected = LinkedManipulator->GetSelectedObject();
    if (!Selected) return;

    TWeakObjectPtr<AActor> WeakSelected(Selected);
    if (FTransform* Original = OriginalTransforms.Find(WeakSelected))
    {
        Selected->SetActorTransform(*Original);
        UE_LOG(LogTemp, Log, TEXT("[InZoi TOOL] Reset %s to original transform"),
            *Selected->GetName());
    }
}

// ============================================================================
// Helpers
// ============================================================================

AActor* UTOOLInputHandler::TraceObjectUnderCursor() const
{
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC) return nullptr;

    FVector WorldLocation, WorldDirection;
    if (!PC->DeprojectMousePositionToWorld(WorldLocation, WorldDirection))
        return nullptr;

    FVector Start = WorldLocation;
    FVector End = Start + WorldDirection * 100000.f;

    FHitResult HitResult;
    FCollisionQueryParams Params;
    Params.bTraceComplex = false;
    Params.bReturnPhysicalMaterial = false;

    // Trace against all visible objects
    if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End,
        ECC_Visibility, Params))
    {
        return HitResult.GetActor();
    }

    return nullptr;
}

FVector UTOOLInputHandler::ComputeDragDelta(FVector2D ScreenDelta) const
{
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC || !PC->PlayerCameraManager) return FVector::ZeroVector;

    // Get camera orientation to compute world-space drag direction
    FRotator CamRotation = PC->PlayerCameraManager->GetCameraRotation();

    FVector Forward = FRotationMatrix(CamRotation).GetUnitAxis(EAxis::X);
    FVector Right = FRotationMatrix(CamRotation).GetUnitAxis(EAxis::Y);
    FVector Up = FVector::UpVector;

    // Project screen delta into world-space movement
    // X screen delta -> right direction, Y screen delta -> forward direction
    FVector WorldDelta = (Right * ScreenDelta.X + Forward * -ScreenDelta.Y);

    return WorldDelta;
}
