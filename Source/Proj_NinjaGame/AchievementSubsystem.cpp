// Fill out your copyright notice in the Description page of Project Settings.


#include "AchievementSubsystem.h"
#include "StealthSaveGame.h"
#include "Kismet/GameplayStatics.h"


void UAchievementSubsystem::UnlockAchievement(EAchievementId Id)
{
	bool& bUnlocked = AchievementStates.FindOrAdd(Id);

	if (bUnlocked)
	{
		UE_LOG(LogTemp, Warning, TEXT("Achievement already unlocked: %s"), *UEnum::GetValueAsString(Id));
		return;
	}

	bUnlocked = true;

	UE_LOG(LogTemp, Warning, TEXT("Achievement unlocked: %s"), *UEnum::GetValueAsString(Id));
}

bool UAchievementSubsystem::IsAchievementUnlocked(EAchievementId Id) const
{
	if (const bool* bUnlocked = AchievementStates.Find(Id))
	{
		return *bUnlocked;
	}
	return false;
}

const TMap<EAchievementId, bool>&
UAchievementSubsystem::GetAllAchievements() const
{
	return AchievementStates;
}

void UAchievementSubsystem::GetAllAchievementData(
	TArray<FAchievementRow>& OutRows) const
{
	if (!AchievementTable)
		return;

	static const FString Context(TEXT("AchievementContext"));
	TArray<FAchievementRow*> Rows;
	AchievementTable->GetAllRows(Context, Rows);

	for (FAchievementRow* Row : Rows)
	{
		if (Row)
		{
			OutRows.Add(*Row);
		}
	}
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
