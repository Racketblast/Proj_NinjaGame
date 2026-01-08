// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AchievementId.h"
#include "AchievementRow.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AchievementSubsystem.generated.h"

class UStealthGameInstance;

UCLASS()
class PROJ_NINJAGAME_API UAchievementSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void UnlockAchievement(EAchievementId Id);

	UFUNCTION(BlueprintCallable)
	bool IsAchievementUnlocked(EAchievementId Id) const;

	const TMap<EAchievementId, bool>& GetAllAchievements() const;

	UFUNCTION(BlueprintCallable)
	void GetAllAchievementData(TArray<FAchievementRow>& OutRows) const;
	
	UFUNCTION(BlueprintCallable)
	void GetAchievementData(EAchievementId OutRows) const;

	void LoadFromSave(class UStealthSaveGame* Save);
	void SaveToSave(class UStealthSaveGame* Save);
	void RestartAchievements();

	void OnEnemyKilled();
	void OnHelmetRemoved();
	void OnHeadShot();
	void OnBackStab();

	UPROPERTY()
	UDataTable* AchievementTable;
	
	UPROPERTY()
	TSubclassOf<class UPopupWidget> PopupWidgetClass;

	// För widgets 
	UFUNCTION(BlueprintCallable)
	int32 GetTotalAchievementCount() const;

	UFUNCTION(BlueprintCallable)
	int32 GetUnlockedAchievementCount() const;

	UFUNCTION(BlueprintCallable)
	FText GetAchievementProgressText() const;

	UFUNCTION(BlueprintCallable)
	float GetAchievementProgress() const;
	
protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override; 
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = true))
	UPopupWidget* PopupWidget;
private:
	UPROPERTY()
	TMap<EAchievementId, bool> AchievementStates;

	UPROPERTY()
	int32 TotalEnemiesKilled = 0;

	UPROPERTY()
	int32 TotalHelmetsRemoved = 0;

	UPROPERTY()
	int32 TotalHeadShots = 0;

	UPROPERTY()
	int32 TotalBackStabs = 0;
};


