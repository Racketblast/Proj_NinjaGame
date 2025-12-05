// Fill out your copyright notice in the Description page of Project Settings.


#include "TargetEnemy.h"

#include "MissionHandler.h"
#include "StealthCharacter.h"


ATargetEnemy::ATargetEnemy()
{
}

void ATargetEnemy::Die()
{
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
