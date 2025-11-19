// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
//Make Enum classes later
#include "StealthGameInstance.h"

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
	TArray<EMission> SavedMissionsCleared;

	//Options saved
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SavedMasterVolumeScale;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SavedSensitivityScale;
};
