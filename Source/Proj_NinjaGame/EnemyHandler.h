// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MeleeEnemy.h"
#include "GameFramework/Actor.h"
#include "EnemyHandler.generated.h"

class AMissionHandler;

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
	bool GetEnemyAnyAlert() const { return bAnyAlert; }
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

	bool GetAreAllEnemiesDead() const { return bAreAllEnemiesDead; }

	bool GetAreAllEnemiesAlive() const { return bAreAllEnemiesAlive; }

	int32 GetAmountOfTimesSpottet() const { return AmountOfTimesSpottet; }
	
protected:
	bool bEnemySeesPlayer = false; 	// True om minst en fiende jagar spelaren

	bool bAnyAlert = false;

	bool bAreAllEnemiesDead = false;

	bool bAreAllEnemiesAlive = true;

	int32 AmountOfTimesSpottet = 0; 
	
	UPROPERTY()
	TArray<AActor*> AllEnemies;
	UPROPERTY()
	TArray<AActor*> AllSecurityCameras;

	UPROPERTY()
	AMissionHandler* MissionHandler = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Enemy Helmet Settings")
	float HelmetChancePercent = 30.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sound")
	TArray<UDataTable*> EnemyVoiceTables;
	
	UDataTable* GetRandomEnemyVoice(TArray<UDataTable*> SelectableVoices);
	
	TArray<UDataTable*> GetAllSelectableVoices();

	// PreviousVoiceIndex = -1, because it is always 0 if empty which means that the first random voice is never chosen
	UPROPERTY()
	int PreviousVoiceIndex = -1;
	
	void UpdateEnemyStates();
};

