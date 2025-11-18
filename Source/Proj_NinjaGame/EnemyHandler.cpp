// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyHandler.h"
#include "MeleeEnemy.h"
#include "Kismet/GameplayStatics.h"
#include "MeleeAIController.h"


AEnemyHandler::AEnemyHandler()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AEnemyHandler::BeginPlay()
{
	Super::BeginPlay();

	// Hitta alla AMeleeEnemy i leveln
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMeleeEnemy::StaticClass(), AllEnemies);

	UE_LOG(LogTemp, Warning, TEXT("EnemyHandler found %d enemies"), AllEnemies.Num());
}

void AEnemyHandler::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdateEnemyStates();
}

void AEnemyHandler::UpdateEnemyStates()
{
	bool bAnyChasing = false;

	// Går igenom alla fiender för att se om någon ut av dem är i Chasing State
	for (AActor* EnemyActor : AllEnemies)
	{
		AMeleeEnemy* Enemy = Cast<AMeleeEnemy>(EnemyActor);
		if (!Enemy) continue;

		AMeleeAIController* AICon = Cast<AMeleeAIController>(Enemy->GetController());
		if (!AICon) continue;

		if (AICon->GetCurrentState() == EEnemyState::Chasing)
		{
			bAnyChasing = true;
			break;
		}
	}

	//Debug
	if (bEnemySeesPlayer != bAnyChasing)
	{
		bEnemySeesPlayer = bAnyChasing;

		if (bEnemySeesPlayer)
		{
			UE_LOG(LogTemp, Warning, TEXT("EnemyHandler: A enemy sees player: TRUE"));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("EnemyHandler: A enemy sees player: FALSE"));
		}
	}

	bEnemySeesPlayer = bAnyChasing;
}

