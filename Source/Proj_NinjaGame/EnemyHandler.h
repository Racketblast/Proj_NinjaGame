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
	UFUNCTION(BlueprintPure)
	TArray<AActor*> GetAllEnemies() const { return AllEnemies; }
	UFUNCTION(BlueprintPure)
	TArray<AActor*> GetAllCameras() const { return AllSecurityCamera; }

	TArray<AMeleeEnemy*> GetTwoClosestEnemies(FVector TargetLocation);
	
	AMeleeEnemy* GetClosestEnemyToLocation(FVector TargetLocation);
	
protected:
	bool bEnemySeesPlayer = false; 	// True om minst en fiende jagar spelaren
	
	UPROPERTY()
	TArray<AActor*> AllEnemies;
	UPROPERTY()
	TArray<AActor*> AllSecurityCamera;
	
	void UpdateEnemyStates();
};
