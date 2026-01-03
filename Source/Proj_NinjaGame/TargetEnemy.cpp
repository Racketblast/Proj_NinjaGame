// Fill out your copyright notice in the Description page of Project Settings.


#include "TargetEnemy.h"

#include "MissionHandler.h"
#include "StealthCharacter.h"


ATargetEnemy::ATargetEnemy()
{
}

void ATargetEnemy::Die()
{
	// För bodyguard
	const FVector DeathLocation = GetActorLocation();
	OnTargetEnemyDied.Broadcast(DeathLocation); 
	
	Super::Die();
	

	if (MissionHandler)
	{
		MissionHandler->RemoveObjectiveFromTotal(this);
	}
	else
	{
		if (AStealthCharacter* Player = Cast<AStealthCharacter>(PlayerPawn))
		{
			Player->bHasCompletedTheMission = true;
		}
	}
}
