// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnemyHandler.h"
#include "GameFramework/Actor.h"
#include "MissionHandler.generated.h"

UCLASS()
class PROJ_NINJAGAME_API AMissionHandler : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMissionHandler();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	float CalculateScore(float TimeTaken);
	float FinalScore;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Scoring")
	int32 CurrentScore = 0;
	
	UPROPERTY()
	AEnemyHandler* EnemyHandler;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoring|Time")
	float TimeThresholdSeconds = 300.f; // 5 min

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoring|Time")
	int32 BaseTimeBonus = 10000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoring|Time")
	int32 PenaltyPerMinute = 1000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoring|Time")
	int32 MinimumTimeBonus = 1000;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Scoring|Stealth")
	int32 StealthKillScoreValue = 500;

	int32 CalculateTimeBonus(float TimeTaken) const;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Objective")
	TArray<AActor*> AllTargets;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Objective")
	TArray<AActor*> AllMissionObjects;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Objective")
	int TotalTargetsToKill;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Objective")
	int TargetsToKill = -1;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Objective")
	int TargetsKilled = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Objective")
	int TotalObjectsToSteal;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Objective")
	int ObjectsToSteal = -1;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Objective")
	int ObjectsStolen = 0;

	void SetupMissionObjectives();
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	float GetScore() const { return FinalScore; }

	UFUNCTION(BlueprintCallable, Category="Scoring")
	void AddStealthKillScore();

	UFUNCTION(BlueprintCallable, Category="Objective")
	void RemoveObjectiveFromTotal(AActor* ThisObject);
};


