// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnumSavedProperties.h"
#include "AchievementId.h"
#include "GameFramework/SaveGame.h"
#include "StealthSaveGame.generated.h"

/**
 * 
 */
UCLASS()
class PROJ_NINJAGAME_API UStealthSaveGame : public USaveGame
{
	GENERATED_BODY()
public:
	//Game saved
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	int SavedCurrentGameFlag;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	bool bSavedFinishedTheGame;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TArray<EMission> SavedMissionsCleared;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSubclassOf<class AThrowableWeapon> SavedOwnThrowWeapon;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	EPlayerOwnThrowWeapon SavedOwnThrowWeaponEnum;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TMap<EMission, int> SavedScoreMap;

	// Achievements
	UPROPERTY(SaveGame)
	TMap<EAchievementId, bool> SavedAchievements;

	UPROPERTY(SaveGame)
	int32 SavedTotalEnemiesKilled = 0;

	UPROPERTY(SaveGame)
	int32 SavedTotalHelmetsRemoved = 0;

	UPROPERTY(SaveGame)
	int32 SavedTotalHeadShots = 0;

	UPROPERTY(SaveGame)
	int32 SavedTotalTotalBackStabs = 0;
	
	//Options saved
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SavedMasterVolumeScale;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SavedSFXVolumeScale;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SavedMusicVolumeScale;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SavedSpeechVolumeScale;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SavedSensitivityScale;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SavedFOVScale;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	int SavedCurrentScalabilitySetting;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FIntPoint SavedCurrentResolutionSetting;
};
