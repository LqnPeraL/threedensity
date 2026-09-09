// Copyright Epic Games, Inc. All Rights Reserved.


#include "Variant_Combat/CombatPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "CombatCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Blueprint/UserWidget.h"
#include "TP_ThirdPerson.h"
#include "Widgets/Input/SVirtualJoystick.h"
#include "SThreeDensityHUDWidget.h"
#include "InputCoreTypes.h"
#include "Components/InputComponent.h"
#include "GameFramework/PlayerInput.h"

void ACombatPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// only spawn touch controls on local player controllers
	if (ShouldUseTouchControls() && IsLocalPlayerController())
	{
		// spawn the mobile controls widget
		MobileControlsWidget = CreateWidget<UUserWidget>(this, MobileControlsWidgetClass);

		if (MobileControlsWidget)
		{
			// add the controls to the player screen
			MobileControlsWidget->AddToPlayerScreen(0);

		} else {

			UE_LOG(LogTP_ThirdPerson, Error, TEXT("Could not spawn mobile controls widget."));

		}

	}

	if (IsLocalPlayerController() && GEngine && GEngine->GameViewport)
	{
		HUDOverlay = SNew(SThreeDensityHUDWidget).Owner(this);
		GEngine->GameViewport->AddViewportWidgetContent(HUDOverlay.ToSharedRef(), 100);
		TipTimeRemaining = 6.0f;
	}
}

void ACombatPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (HUDOverlay.IsValid() && GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->RemoveViewportWidgetContent(HUDOverlay.ToSharedRef());
	}
	HUDOverlay.Reset();
	Super::EndPlay(EndPlayReason);
}

void ACombatPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// only add IMCs for local player controllers
	if (IsLocalPlayerController())
	{
		// add the input mapping context
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}

			// only add these IMCs if we're not using mobile touch input
			if (!ShouldUseTouchControls())
			{
				for (UInputMappingContext* CurrentContext : MobileExcludedMappingContexts)
				{
					Subsystem->AddMappingContext(CurrentContext, 0);
				}
			}
		}

		FInputKeyBinding& EscapeBind = InputComponent->BindKey(EKeys::Escape, IE_Pressed, this, &ACombatPlayerController::TogglePauseMenu);
		EscapeBind.bExecuteWhenPaused = true;
		FInputKeyBinding& StartBind = InputComponent->BindKey(EKeys::Gamepad_Special_Right, IE_Pressed, this, &ACombatPlayerController::TogglePauseMenu);
		StartBind.bExecuteWhenPaused = true;

		// Space jump at controller level (IMC_Combat has no Jump mapping)
		InputComponent->BindKey(EKeys::SpaceBar, IE_Pressed, this, &ACombatPlayerController::JumpPressed);
		InputComponent->BindKey(EKeys::SpaceBar, IE_Released, this, &ACombatPlayerController::JumpReleased);
		InputComponent->BindKey(EKeys::Gamepad_FaceButton_Bottom, IE_Pressed, this, &ACombatPlayerController::JumpPressed);
		InputComponent->BindKey(EKeys::Gamepad_FaceButton_Bottom, IE_Released, this, &ACombatPlayerController::JumpReleased);
	}
}

void ACombatPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	if (!bTipsFinished && !bPauseMenuOpen)
	{
		TipTimeRemaining -= DeltaTime;
		if (TipTimeRemaining <= 0.0f)
		{
			AdvanceOrDismissTip();
		}
	}
}

void ACombatPlayerController::TogglePauseMenu()
{
	if (!IsLocalPlayerController())
	{
		return;
	}

	bPauseMenuOpen = !bPauseMenuOpen;
	if (bPauseMenuOpen)
	{
		bHasOpenedMenu = true;
		MenuTab = 0;
		SetPause(true);
		bShowMouseCursor = true;
		bEnableClickEvents = true;
		bEnableMouseOverEvents = true;
		FInputModeGameAndUI Mode;
		Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		Mode.SetHideCursorDuringCapture(false);
		SetInputMode(Mode);
	}
	else
	{
		SetPause(false);
		ApplyMenuInputMode();
	}
}

void ACombatPlayerController::ApplyMenuInputMode()
{
	bShowMouseCursor = false;
	FInputModeGameOnly GameOnly;
	SetInputMode(GameOnly);
}

void ACombatPlayerController::JumpPressed()
{
	if (bPauseMenuOpen)
	{
		return;
	}
	if (ACombatCharacter* CombatPawn = Cast<ACombatCharacter>(GetPawn()))
	{
		CombatPawn->JumpPressed();
	}
}

void ACombatPlayerController::JumpReleased()
{
	if (ACombatCharacter* CombatPawn = Cast<ACombatCharacter>(GetPawn()))
	{
		CombatPawn->JumpReleased();
	}
}

void ACombatPlayerController::SetMenuTab(int32 NewTab)
{
	MenuTab = NewTab;
}

void ACombatPlayerController::AdvanceOrDismissTip()
{
	++TipIndex;
	TipTimeRemaining = 6.0f;
	if (TipIndex >= 4)
	{
		bTipsFinished = true;
	}
}

FText ACombatPlayerController::GetCurrentTipText() const
{
	switch (TipIndex)
	{
	case 0:
		return FText::FromString(TEXT("Move with WASD. Look with the mouse."));
	case 1:
		return FText::FromString(TEXT("Left click chains a combo. Hold right click to charge a heavy strike."));
	case 2:
		return FText::FromString(TEXT("Space jumps. Your sensei waits near the start. Q swaps camera shoulder. Mouse wheel zooms."));
	default:
		return FText::FromString(TEXT("Press ESC anytime for the full control cookbook and graphics settings."));
	}
}

void ACombatPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// subscribe to the pawn's OnDestroyed delegate
	InPawn->OnDestroyed.AddDynamic(this, &ACombatPlayerController::OnPawnDestroyed);
}

void ACombatPlayerController::SetRespawnTransform(const FTransform& NewRespawn)
{
	// save the new respawn transform
	RespawnTransform = NewRespawn;
}

void ACombatPlayerController::OnPawnDestroyed(AActor* DestroyedActor)
{
	// spawn a new character at the respawn transform
	if (ACombatCharacter* RespawnedCharacter = GetWorld()->SpawnActor<ACombatCharacter>(CharacterClass, RespawnTransform))
	{
		// possess the character
		Possess(RespawnedCharacter);
	}
}

bool ACombatPlayerController::ShouldUseTouchControls() const
{
	// are we on a mobile platform? Should we force touch?
	return SVirtualJoystick::ShouldDisplayTouchInterface() || bForceTouchControls;
}
