// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MeleeEnemy.h"
#include "GameFramework/Actor.h"
#include "EnemyHandler.generated.h"

UCLASS()
class PROJ_NINJAGAME_API AEnemyHandler : public AActor
{
	GENERATED_BODY()
	
public:	
	AEnemyHandler();
	
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	
	UFUNCTION(BlueprintPure)
	bool GetEnemySeesPlayer() const { return bEnemySeesPlayer; }

	TArray<AMeleeEnemy*> GetTwoClosestEnemies(FVector TargetLocation);

	AMeleeEnemy* GetClosestEnemyToLocation(FVector TargetLocation);
	
protected:
	UPROPERTY()
	TArray<AActor*> AllEnemies;
	
	bool bEnemySeesPlayer = false; 	// True om minst en fiende jagar spelaren
	
	void UpdateEnemyStates();
};
