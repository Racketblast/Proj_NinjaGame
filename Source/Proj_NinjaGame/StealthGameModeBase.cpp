// Fill out your copyright notice in the Description page of Project Settings.


#include "StealthGameModeBase.h"

#include "StealthGameInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"

AStealthGameModeBase::AStealthGameModeBase()
{
}

void AStealthGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	//Pushes the sound override when the level has loaded
	if (UStealthGameInstance* GI = Cast<UStealthGameInstance>(GetGameInstance()))
	{
		GI->SetAllSoundClassOverride();
	}
}

AActor* AStealthGameModeBase::ChoosePlayerStart_Implementation(AController* Player)
{
	
	if (UStealthGameInstance* GI = Cast<UStealthGameInstance>(GetGameInstance()))
	{
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(),PlayerStarts);
		for (auto PlayerStart : PlayerStarts)
		{
			if (APlayerStart* Start = Cast<APlayerStart>(PlayerStart))
			{
				if (GI->StartLocation == Start->PlayerStartTag)
				{
					return Start;
				}
			}
		}
	}
	return Super::ChoosePlayerStart_Implementation(Player);
}
