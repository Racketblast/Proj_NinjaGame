// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AchievementId.h"
#include "AchievementRow.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AchievementSubsystem.generated.h"


UCLASS()
class PROJ_NINJAGAME_API UAchievementSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	void UnlockAchievement(EAchievementId Id);

	UFUNCTION(BlueprintCallable)
	bool IsAchievementUnlocked(EAchievementId Id) const;

	const TMap<EAchievementId, bool>& GetAllAchievements() const;

	UFUNCTION(BlueprintCallable)
	void GetAllAchievementData(TArray<FAchievementRow>& OutRows) const;

	void LoadFromSave(class UStealthSaveGame* Save);
	void SaveToSave(class UStealthSaveGame* Save);

protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override; 
	
	UPROPERTY(Transient)
	UDataTable* AchievementTable;

private:
	UPROPERTY()
	TMap<EAchievementId, bool> AchievementStates;
	
};
