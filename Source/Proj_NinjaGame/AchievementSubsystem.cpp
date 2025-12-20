// Fill out your copyright notice in the Description page of Project Settings.


#include "AchievementSubsystem.h"

#include "PopupWidget.h"
#include "StealthGameInstance.h"
#include "StealthSaveGame.h"
#include "Kismet/GameplayStatics.h"


void UAchievementSubsystem::Initialize(FSubsystemCollectionBase& Collection) 
{
	Super::Initialize(Collection);
	UE_LOG(LogTemp, Warning, TEXT("AchievementSubsystem Initialized"));

	/*
	const TCHAR* TablePath = TEXT("/Game/DataTables/AchievementInfo.AchievementInfo");

	AchievementTable = LoadObject<UDataTable>(nullptr, TablePath);
	*/

	UStealthGameInstance* GI = Cast<UStealthGameInstance>(GetGameInstance());

	if (!GI || !GI->AchievementTable)
	{
		UE_LOG(LogTemp, Error, TEXT("AchievementTable missing in StealthGameInstance"));
		return;
	}

	AchievementTable = GI->AchievementTable;

	UE_LOG(LogTemp, Warning, TEXT("AchievementSubsystem received AchievementTable from GI"));
}

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

	if (PopupWidget)
	{
		PopupWidget->AchievementPopup(Id);
	}
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
	{
		UE_LOG(LogTemp, Error, TEXT("AchievementTable is NULL"));
		return;
	}

	static const FString Context(TEXT("AchievementContext"));
	TArray<FAchievementRow*> Rows;
	AchievementTable->GetAllRows(Context, Rows);

	UE_LOG(LogTemp, Warning, TEXT("AchievementTable rows: %d"), Rows.Num());

	for (FAchievementRow* Row : Rows)
	{
		if (Row)
		{
			OutRows.Add(*Row);
		}
	}
}

void UAchievementSubsystem::GetAchievementData(EAchievementId OutRows) const
{
	
}

void UAchievementSubsystem::LoadFromSave(UStealthSaveGame* Save)
{
	AchievementStates.Empty();

	if (!Save)
		return;

	AchievementStates = Save->SavedAchievements;
	TotalEnemiesKilled = Save->SavedTotalEnemiesKilled;
	TotalHelmetsRemoved = Save->SavedTotalHelmetsRemoved;
}

void UAchievementSubsystem::SaveToSave(UStealthSaveGame* Save)
{
	if (!Save)
		return;

	Save->SavedAchievements = AchievementStates;
	Save->SavedTotalEnemiesKilled = TotalEnemiesKilled;
	Save->SavedTotalHelmetsRemoved = TotalHelmetsRemoved;
}

void UAchievementSubsystem::RestartAchievements()
{
	AchievementStates.Empty();
	TotalEnemiesKilled = 0;
	TotalHelmetsRemoved = 0;
}


void UAchievementSubsystem::OnEnemyKilled()
{
	TotalEnemiesKilled++;

	UE_LOG(LogTemp, Log, TEXT("Enemy killed. Total kills: %d"), TotalEnemiesKilled);

	if (TotalEnemiesKilled >= 100)
	{
		UnlockAchievement(EAchievementId::Killed_OneHundred_Enemies);
	}
}

void UAchievementSubsystem::OnHelmetRemoved()
{
	TotalHelmetsRemoved++;

	UE_LOG(LogTemp, Log, TEXT("Helmet removed. Total helmets: %d"), TotalHelmetsRemoved);

	if (TotalHelmetsRemoved >= 20)
	{
		UnlockAchievement(EAchievementId::Remove_Twenty_Helmets_From_Enemies);
	}
}
