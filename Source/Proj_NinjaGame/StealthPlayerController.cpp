// Fill out your copyright notice in the Description page of Project Settings.


#include "StealthPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "Proj_NinjaGameCameraManager.h"
#include "Blueprint/UserWidget.h"
#include "Proj_NinjaGame.h"
#include "Widgets/Input/SVirtualJoystick.h"

AStealthPlayerController::AStealthPlayerController()
{
	// set the player camera manager class
	PlayerCameraManagerClass = AProj_NinjaGameCameraManager::StaticClass();
}

void AStealthPlayerController::BeginPlay()
{
	Super::BeginPlay();
}

void AStealthPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// only add IMCs for local player controllers
	if (IsLocalPlayerController())
	{
		// Add Input Mapping Context
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}
		}
	}
	
}