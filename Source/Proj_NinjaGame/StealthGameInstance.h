// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "StealthGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class PROJ_NINJAGAME_API UStealthGameInstance : public UGameInstance
{
	GENERATED_BODY()
public:
	virtual void Init() override;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bUsingGamepad = false;
	
	//Sound
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Sound")
	USoundMix* SoundMix;
	UPROPERTY(BlueprintReadWrite, Category="Options")
	float MasterVolumeScale = 1.0f;
	UPROPERTY(BlueprintReadWrite, Category="Options")
	float SensitivityScale = 1.0f;
	
	//SaveGame
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class UStealthSaveGame> SaveGameObject;
	UPROPERTY(BlueprintReadWrite)
	UStealthSaveGame* Save;
	
	UFUNCTION(BlueprintCallable)
	void SaveGame();
	void FillSaveGame();
	UFUNCTION(BlueprintCallable)
	void LoadGame();
	UFUNCTION(BlueprintCallable)
	void RestartGame();
	UFUNCTION(BlueprintCallable)
	bool HasGameChanged();
};


