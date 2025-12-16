// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AchievementSubsystem.generated.h"


UCLASS()
class PROJ_NINJAGAME_API UAchievementSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	void UnlockAchievement(FName AchievementId);
	bool IsAchievementUnlocked(FName AchievementId) const;

	const TMap<FName, bool>& GetAllAchievements() const;

	void LoadFromSave(class UStealthSaveGame* Save);
	void SaveToSave(class UStealthSaveGame* Save);

private:
	UPROPERTY()
	TMap<FName, bool> AchievementStates;
	
};
