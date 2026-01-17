// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "EnumSavedProperties.h"
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

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	bool bUsingGamepad = true;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	int CurrentGameFlag;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	bool bFinishedTheGame = false;
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

	EMission GetCurrentMission() const {return CurrentMission;}

	//Achivements
	UPROPERTY(EditDefaultsOnly, Category="Achievements")
	UDataTable* AchievementTable;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = true), Category="Achievements")
	TSubclassOf<class UPopupWidget> AchivementsPopupWidgetClass;
	
	//Sound
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Options | Sound | Scales")
	USoundMix* SoundMix;
	
	//Sound Options
	UPROPERTY(BlueprintReadWrite, Category="Options | Sound | Scales")
	float MasterVolumeScale = 1.0f;
	UPROPERTY(BlueprintReadWrite, Category="Options | Sound | Scales")
	float SFXVolumeScale = 1.0f;
	UPROPERTY(BlueprintReadWrite, Category="Options | Sound | Scales")
	float MusicVolumeScale = 1.0f;
	UPROPERTY(BlueprintReadWrite, Category="Options | Sound | Scales")
	float SpeechVolumeScale = 1.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Options | Sound | SoundClasses")
	USoundClass* MasterSoundClass;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Options | Sound | SoundClasses")
	USoundClass* SFXSoundClass;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Options | Sound | SoundClasses")
	USoundClass* MusicSoundClass;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Options | Sound | SoundClasses")
	USoundClass* SpeechSoundClass;
	UFUNCTION(BlueprintCallable)
	void SetAllSoundClassOverride();
	
	//Player Options
	UPROPERTY(BlueprintReadWrite, Category="Options")
	float SensitivityScale = 1.0f;
	UPROPERTY(BlueprintReadWrite, Category="Options")
	float FOVScale = 0.0f;
	UPROPERTY(BlueprintReadWrite, Category="Options")
	int CurrentScalabilitySetting;
	UPROPERTY(BlueprintReadWrite, Category="Options")
	bool bShadowsOnOff = true;
	UPROPERTY(BlueprintReadWrite, Category="Options")
	FIntPoint CurrentResolutionSetting;

	void SetCurrentToClosestResolution();
	
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
	void FillLoadGame();
	void FillLoadOptions();
	
	UFUNCTION(BlueprintCallable)
	void LoadGame();
	UFUNCTION(BlueprintCallable)
	void LoadOptions();
	UFUNCTION(BlueprintCallable)
	void SetOptions();
	UFUNCTION(BlueprintCallable)
	void RestartGame();
	UFUNCTION(BlueprintCallable)
	bool HasGameChanged();
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	TMap<EMission, int> ScoreMap;
	UFUNCTION(BlueprintCallable)
	bool ScoreMapEquals(const TMap<EMission, int> ScoreMap1, const TMap<EMission, int> ScoreMap2);
	UFUNCTION(BlueprintCallable)
	bool TrySetMissionScore(EMission Mission, int32 NewScore);

	UFUNCTION(BlueprintCallable, Category="Score")
	bool HasMissionScore(EMission Mission) const;

	UFUNCTION(BlueprintCallable, Category="Score")
	bool GetMissionScore(EMission Mission, int32& OutScore) const; 

	
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


