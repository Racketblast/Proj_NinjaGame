// Fill out your copyright notice in the Description page of Project Settings.


#include "MissionHandler.h"

#include "CollectableMissionObject.h"
#include "EnemyHandler.h"
#include "StealthCharacter.h"
#include "TargetEnemy.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AMissionHandler::AMissionHandler()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AMissionHandler::BeginPlay()
{
	Super::BeginPlay();

	EnemyHandler = Cast<AEnemyHandler>(UGameplayStatics::GetActorOfClass(
	GetWorld(), AEnemyHandler::StaticClass()
	));
	
	SetupMissionObjectives();
}

// Called every frame
void AMissionHandler::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AMissionHandler::SetupMissionObjectives()
{
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATargetEnemy::StaticClass(), AllTargets);
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACollectableMissionObject::StaticClass(), AllMissionObjects);

	for (auto Target : AllTargets)
	{
		if (ATargetEnemy* TargetEnemy = Cast<ATargetEnemy>(Target))
		{
			TargetEnemy->SetMissionHandler(this);
			TotalTargetsToKill++;
		}
	}
	
	for (auto Objects : AllMissionObjects)
	{
		if (ACollectableMissionObject* CollectableObject = Cast<ACollectableMissionObject>(Objects))
		{
			CollectableObject->SetMissionHandler(this);
			TotalObjectsToSteal++;
		}
	}

	if (TargetsToKill < 0)
	{
		TargetsToKill = TotalTargetsToKill;
	}
	if (ObjectsToSteal < 0)
	{
		ObjectsToSteal = TotalObjectsToSteal;
	}
}

// Kalla funktionen när spelaren har klarat missionet 
float AMissionHandler::CalculateScore(float TimeTaken)
{
	//Lite start score, kanske för att man klara av missionet
	float Score = 1000;
	
	float ScoreMultiplier = 1.0f;

	Score += CurrentScore;

	// Lägg till tidsbonus
	Score += CalculateTimeBonus(TimeTaken);

	if (!EnemyHandler)
	{
		UE_LOG(LogTemp, Warning, TEXT("MissionHandler: EnemyHandler is null"));
		return FinalScore = Score *= ScoreMultiplier;
	}

	//  ifall man inte blev sedd
	if (!EnemyHandler->GetAEnemyHasSeenPlayer())
	{
		Score += 5000;
	}

	// ifall man dödade alla fiender
	if (EnemyHandler->GetAreAllEnemiesDead())
	{
		ScoreMultiplier += 1;
	}

	// ifall inga fiender har dött
	if (EnemyHandler->GetAreAllEnemiesAlive())
	{
		ScoreMultiplier += 1;
	}

	FinalScore = Score *= ScoreMultiplier;
	
	return FinalScore;
}


int32 AMissionHandler::CalculateTimeBonus(float TimeTaken) const
{
	// full bonus
	if (TimeTaken <= TimeThresholdSeconds)
	{
		return BaseTimeBonus;
	}

	// Hur många hela minuter över threshold
	float TimeOverSeconds = TimeTaken - TimeThresholdSeconds;
	int32 MinutesOver = TimeOverSeconds / 60.f; // FMath::FloorToInt(TimeOverSeconds / 60.f);

	int32 Penalty = MinutesOver * PenaltyPerMinute;

	int32 FinalBonus = BaseTimeBonus - Penalty;

	// Se till att bonus aldrig går under minvärdet
	FinalBonus = FMath::Max(FinalBonus, MinimumTimeBonus);

	return FinalBonus;
}


void AMissionHandler::AddStealthKillScore()
{
	if (!EnemyHandler)
	{
		UE_LOG(LogTemp, Warning, TEXT("MissionHandler: EnemyHandler is null"));
		return;
	}

	CurrentScore += StealthKillScoreValue;
	UE_LOG(LogTemp, Log, TEXT("Stealth kill! +%d score. Current total: %d"), StealthKillScoreValue, CurrentScore);
}

void AMissionHandler::RemoveObjectiveFromTotal(AActor* ThisObject)
{
	if (Cast<ATargetEnemy>(ThisObject))
	{
		TargetsKilled++;
		AllTargets.Remove(ThisObject);
	}
	else if (Cast<ACollectableMissionObject>(ThisObject))
	{
		ObjectsStolen++;
		AllMissionObjects.Remove(ThisObject);
	}

	//Player did everything
	if (TargetsKilled >= TargetsToKill && ObjectsStolen >= ObjectsToSteal)
	{
		if (AStealthCharacter* Player = Cast<AStealthCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0)))
		{
			Player->bHasCompletedTheMission = true;
		}
	}
}
