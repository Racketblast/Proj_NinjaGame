// Fill out your copyright notice in the Description page of Project Settings.


#include "MissionHandler.h"

#include "CollectableMissionObject.h"
#include "StealthCharacter.h"
#include "TargetEnemy.h"
#include "Kismet/GameplayStatics.h"


AMissionHandler::AMissionHandler()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AMissionHandler::BeginPlay()
{
	Super::BeginPlay();

	EnemyHandler = Cast<AEnemyHandler>(UGameplayStatics::GetActorOfClass( GetWorld(), AEnemyHandler::StaticClass() ));
	
	SetupMissionObjectives();

	bMissionTimerActive = true;
	MissionTimeElapsed = 0.0f;
}

void AMissionHandler::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bMissionTimerActive)
	{
		MissionTimeElapsed += DeltaTime;
	}

	// Debug
	/*if (GEngine)
	{
		// Tid debug, så att den är på skärmen
		GEngine->AddOnScreenDebugMessage(
			2,						
			0.0f,					
			FColor::Cyan,			
			FString::Printf(TEXT("Mission Time: %.2f"), MissionTimeElapsed)
		);
	}*/
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

	//Failsafe if there is no objective
	if (TargetsToKill <= 0 && ObjectsToSteal <= 0)
	{
		if (AStealthCharacter* Player = Cast<AStealthCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0)))
		{
			Player->bHasCompletedTheMission = true;
		}
	}
}

// Kalla funktionen när spelaren har klarat missionet 
float AMissionHandler::CalculateScore(float TimeTaken)
{
	float Score = 0.0f;
	
	if (AStealthCharacter* Player = Cast<AStealthCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0)))
	{
		if (!Player->bHasCompletedTheMission)
		{
			UE_LOG(LogTemp, Warning, TEXT("CalculateScore: bHasCompletedTheMission is false"));
			return Score;
		}
	}

	//SetMissionTimerActive(false); 

	//Lite start score för att man klara av missionet
	Score += 5000;
	
	float ScoreMultiplier = 1.0f;

	Score += StealthKillScore;

	// Lägg till tidsbonus
	Score += CalculateTimeBonus(TimeTaken);

	TimeScoreBonus = CalculateTimeBonus(TimeTaken);

	UE_LOG(LogTemp, Warning, TEXT("MISSION SCORE DEBUG START"));
	UE_LOG(LogTemp, Warning, TEXT("Mission Complete Score: %f"), Score - StealthKillScore - CalculateTimeBonus(TimeTaken));
	UE_LOG(LogTemp, Warning, TEXT("StealthKillScoreValue Added: %i"), StealthKillScore);
	UE_LOG(LogTemp, Warning, TEXT("Time Bonus: %i (TimeTaken: %f sec)"), CalculateTimeBonus(TimeTaken), TimeTaken);

	if (!EnemyHandler)
	{
		UE_LOG(LogTemp, Warning, TEXT("MissionHandler: EnemyHandler is null"));
		return FinalScore = Score *= ScoreMultiplier;
	}

	TimesSpotted = EnemyHandler->GetAmountOfTimesSpottet();
	// ifall man inte blev sedd
	if (TimesSpotted == 0)
	{
		Score += 5000;
		TimesSpottedScore = 5000;
		UE_LOG(LogTemp, Warning, TEXT("Player was never spotted: +5000"));
	}
	else
	{
		TimesSpottedScore -= TimesSpotted * 100;
		Score -= TimesSpottedScore;
		UE_LOG(LogTemp, Warning, TEXT("Player spotted %d times: -%f"), TimesSpotted, TimesSpottedScore);
	}

	// ifall man dödade alla fiender
	if (EnemyHandler->GetAreAllEnemiesDead())
	{
		ScoreMultiplier += 1;
		bKilledAllOrNoEnemies = true;
		UE_LOG(LogTemp, Warning, TEXT("All enemies dead: ScoreMultiplier +1"));
	}

	// ifall inga fiender har dött
	if (EnemyHandler->GetAreAllEnemiesAlive())
	{
		ScoreMultiplier += 1;
		bKilledAllOrNoEnemies = true;
		UE_LOG(LogTemp, Warning, TEXT("All enemies alive: ScoreMultiplier +1"));
	}

	UE_LOG(LogTemp, Warning, TEXT("Final Score before multiplier: %f"), Score);
	
	FinalScore = Score *= ScoreMultiplier;
	
	UE_LOG(LogTemp, Warning, TEXT("Score Multiplier: %f"), ScoreMultiplier);
	UE_LOG(LogTemp, Warning, TEXT("Final Score: %f"), FinalScore);
	UE_LOG(LogTemp, Warning, TEXT("MISSION SCORE DEBUG END"));
	
	return FinalScore;
}

int32 AMissionHandler::CalculateTimeBonus(float TimeTaken) const
{
	// Full bonus om man ligger under eller exakt på threshold
	if (TimeTaken <= TimeThresholdSeconds)
	{
		return BaseTimeBonus;
	}

	float SecondsOver = TimeTaken - TimeThresholdSeconds;

	// penalty per sekund 
	int32 Penalty = SecondsOver * PenaltyPerSecond;

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

	StealthKills++;
	StealthKillScore += StealthKillScoreValue;
	UE_LOG(LogTemp, Log, TEXT("Stealth kill number %d! +%d score. Current total: %d"), StealthKills, StealthKillScoreValue, StealthKillScore);
}

void AMissionHandler::SetMissionTimerActive(bool bActive)
{
	if (bActive)
	{
		bMissionTimerActive = true;
	}
	else
	{
		bMissionTimerActive = false;
	}
}

FString AMissionHandler::FormatTime(float TimeTaken) const
{
	// Runda ned till hela sekunder
	int32 TotalSeconds = FMath::FloorToInt(TimeTaken);

	// Plocka ut timmar, minuter, sekunder
	int32 Hours = TotalSeconds / 3600;
	int32 Minutes = (TotalSeconds % 3600) / 60;
	int32 Seconds = TotalSeconds % 60;

	// Bygg strängen
	if (Hours > 0)
	{
		return FString::Printf(TEXT("%02d:%02d:%02d"), Hours, Minutes, Seconds);
	}
	else
	{
		return FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
	}
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
