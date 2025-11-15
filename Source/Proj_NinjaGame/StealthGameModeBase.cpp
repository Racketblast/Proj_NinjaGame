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
	
	SetPlayerStart();
}

void AStealthGameModeBase::SetPlayerStart()
{
	TArray<AActor*> PlayerStarts;
	UGameplayStatics::GetAllActorsOfClass(this, TSubclassOf<APlayerStart>(),PlayerStarts);
	for (auto PlayerStart : PlayerStarts)
	{
		if (APlayerStart* Start = Cast<APlayerStart>(PlayerStart))
		{
			if (UStealthGameInstance* GI = Cast<UStealthGameInstance>(GetGameInstance()))
			{
				UE_LOG(LogTemp, Error, TEXT("Something"));
				if (GI->StartLocation == Start->PlayerStartTag)
				{
					UE_LOG(LogTemp, Error, TEXT("Found it"));
					UGameplayStatics::GetPlayerCharacter(this,0)->SetActorLocation(Start->GetActorLocation());
					return;
				}
			}
		}
	}
}
