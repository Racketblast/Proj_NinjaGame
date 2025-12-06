// Fill out your copyright notice in the Description page of Project Settings.


#include "ExtractObject.h"

#include "EnemyHandler.h"
#include "MissionHandler.h"
#include "StealthCharacter.h"
#include "Blueprint/UserWidget.h"

AExtractObject::AExtractObject()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

void AExtractObject::Use_Implementation(class AStealthCharacter* Player)
{
	Super::Use_Implementation(Player);
	
	if (!Player) return;

	if (EnemyHandler)
	{
		if (!EnemyHandler->GetEnemySeesPlayer())
		{
			if (MissionHandler) 
			{
				//MissionHandler->SetMissionTimerActive(false);
				MissionHandler->CalculateScore(MissionHandler->GetMissionTimeElapsed());
			}
			if (ExitWidgetClass)
			{
				if (UUserWidget* ExitWidget = CreateWidget<UUserWidget>(GetWorld(), ExitWidgetClass))
				{
					ExitWidget->AddToViewport();
				}
			}
		}
	}
	else
	{
		if (ExitWidgetClass)
		{
			if (UUserWidget* ExitWidget = CreateWidget<UUserWidget>(GetWorld(), ExitWidgetClass))
			{
				ExitWidget->AddToViewport();
			}
		}
	}
}

void AExtractObject::BeginPlay()
{
	Super::BeginPlay();
}
