// Fill out your copyright notice in the Description page of Project Settings.


#include "AchievementSubsystem.h"
#include "StealthSaveGame.h"
#include "Kismet/GameplayStatics.h"

void UAchievementSubsystem::UnlockAchievement(FName AchievementId)
{
	if (AchievementId.IsNone())
		return;

	// Kållar om Achievementet redan blivit upplåst
	if (AchievementStates.Contains(AchievementId) && AchievementStates[AchievementId])
		return;

	AchievementStates.Add(AchievementId, true);

	UE_LOG(LogTemp, Warning, TEXT("Achievement unlocked: %s"), *AchievementId.ToString());
}

bool UAchievementSubsystem::IsAchievementUnlocked(FName AchievementId) const
{
	if (const bool* bUnlocked = AchievementStates.Find(AchievementId))
	{
		return *bUnlocked;
	}
	return false;
}

const TMap<FName, bool>& UAchievementSubsystem::GetAllAchievements() const
{
	return AchievementStates;
}

void UAchievementSubsystem::LoadFromSave(UStealthSaveGame* Save)
{
	AchievementStates.Empty();
	
	if (!Save)
		return;

	AchievementStates = Save->SavedAchievements;
}

void UAchievementSubsystem::SaveToSave(UStealthSaveGame* Save)
{
	if (!Save)
		return;

	Save->SavedAchievements = AchievementStates;
}
