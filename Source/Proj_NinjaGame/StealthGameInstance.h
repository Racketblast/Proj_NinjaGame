// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "StealthGameInstance.generated.h"

/**
 * 
 */

UENUM(BlueprintType)
enum class EMission : uint8
{
	None				UMETA(DisplayName = "None"),
	FirstMission		UMETA(DisplayName = "FirstMission"),
	SecondMission		UMETA(DisplayName = "SecondMission"),
	ThirdMission		UMETA(DisplayName = "ThirdMission"),
	FourthMission		UMETA(DisplayName = "FourthMission"),
};

UENUM(BlueprintType)
enum class EPlayerOwnThrowWeapon : uint8
{
	None    UMETA(DisplayName = "None"),
	Kunai     UMETA(DisplayName = "Kunai"),
	SmokeBomb  UMETA(DisplayName = "SmokeBomb"),
};

UCLASS()
class PROJ_NINJAGAME_API UStealthGameInstance : public UGameInstance
{
	GENERATED_BODY()
public:
	virtual void Init() override;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bUsingGamepad = false;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	int CurrentGameFlag;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSubclassOf<class AThrowableWeapon> CurrentOwnThrowWeapon;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	EPlayerOwnThrowWeapon CurrentOwnThrowWeaponEnum = EPlayerOwnThrowWeapon::None;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TArray<EMission> MissionsCleared;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FString CurrentInteractText;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	bool InteractTextOverride;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FName StartLocation;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	EMission CurrentMission;
	
	//Sound
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Sound")
	USoundMix* SoundMix;
	
	//Sound Options
	UPROPERTY(BlueprintReadWrite, Category="Options")
	float MasterVolumeScale = 1.0f;
	
	//Player Options
	UPROPERTY(BlueprintReadWrite, Category="Options")
	float SensitivityScale = 1.0f;
	
	//SaveGame
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class UStealthSaveGame> SaveGameObject;
	UPROPERTY(BlueprintReadWrite)
	UStealthSaveGame* Save;
	
	UFUNCTION(BlueprintCallable)
	void SaveGame();
	UFUNCTION(BlueprintCallable)
	void SaveOptions();
	
	void FillSaveGame();
	void FillSaveOptions();
	
	UFUNCTION(BlueprintCallable)
	void LoadGame();
	UFUNCTION(BlueprintCallable)
	void LoadOptions();
	UFUNCTION(BlueprintCallable)
	void RestartGame();
	UFUNCTION(BlueprintCallable)
	bool HasGameChanged();
	
	//Dialogue
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly, Category="Dialogue")
	UDataTable* EventDialogueInfo;
	UPROPERTY(BlueprintReadWrite, Category="Dialogue")
	bool bCanPlayDialogue = false;
	UPROPERTY(BlueprintReadOnly, Category="Dialogue")
	bool bDialogueIsPlaying = false;
	UPROPERTY(BlueprintReadOnly, Category="Dialogue")
	FName CurrentDialogueRowName;
	UPROPERTY(BlueprintReadWrite, Category="Dialogue")
	FName StartDialogueRowName;
	FName NextDialogueRowName;
	UPROPERTY(BlueprintReadWrite, Category="Dialogue")
	UAudioComponent* DialogueComponent;
	
	UFUNCTION(BlueprintCallable, Category="Dialogue")
	void StartDialogue();
	UFUNCTION()
	void PlayNextDialogue();
	UFUNCTION(BlueprintCallable, Category="Dialogue")
	void StopDialogue();
	UFUNCTION(BlueprintPure, Category="Dialogue")
	float GetDialogueDuration();
	
	//WeaponSwitch
	UFUNCTION(BlueprintCallable)
	void SwitchOwnWeapon(EPlayerOwnThrowWeapon WeaponToSwitchTo);
	
private:
	FTimerHandle DialogueTimerHandle;
};


