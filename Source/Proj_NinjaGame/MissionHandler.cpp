// Fill out your copyright notice in the Description page of Project Settings.


#include "MissionHandler.h"

#include "EnemyHandler.h"
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
	
}

// Called every frame
void AMissionHandler::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

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
