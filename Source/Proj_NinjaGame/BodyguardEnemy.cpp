// Fill out your copyright notice in the Description page of Project Settings.


#include "BodyguardEnemy.h"


void ABodyguardEnemy::BeginPlay()
{
	Super::BeginPlay();

	if (ProtectedTarget)
	{
		ProtectedTarget->OnTargetEnemyDied.AddDynamic(this, &ABodyguardEnemy::OnProtectedTargetDied);
	}
}

void ABodyguardEnemy::OnProtectedTargetDied(FVector DeathLocation)
{
	bProtectedTargetIsDead = true;
	LastKnownTargetDeathLocation = DeathLocation;
}
 