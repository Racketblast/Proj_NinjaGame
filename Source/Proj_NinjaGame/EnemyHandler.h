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
	UFUNCTION(BlueprintCallable)
	void RemoveEnemy(AActor* EnemyRemoved);
	UFUNCTION(BlueprintCallable)
	void RemoveCamera(AActor* CameraRemoved);
	UFUNCTION(BlueprintPure)
	TArray<AActor*> GetAllCameras() const { return AllSecurityCameras; }

	TArray<AEnemy*> GetTwoClosestEnemies(FVector TargetLocation);
	
	AEnemy* GetClosestEnemyToLocation(FVector TargetLocation);

	bool GetAEnemyHasSeenPlayer() const { return bAEnemyHasSeenPlayer; }

	bool GetAreAllEnemiesDead() const { return bAreAllEnemiesDead; }

	bool GetAreAllEnemiesAlive() const { return bAreAllEnemiesDead; }
	
protected:
	bool bEnemySeesPlayer = false; 	// True om minst en fiende jagar spelaren

	bool bAEnemyHasSeenPlayer = false;

	bool bAreAllEnemiesDead = false;

	bool bAreAllEnemiesAlive = false;
	
	UPROPERTY()
	TArray<AActor*> AllEnemies;
	UPROPERTY()
	TArray<AActor*> AllSecurityCameras;
	
	void UpdateEnemyStates();
};

