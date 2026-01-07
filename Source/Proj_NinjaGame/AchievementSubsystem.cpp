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

	if (GI->AchivementsPopupWidgetClass)
	{
		PopupWidgetClass = GI->AchivementsPopupWidgetClass;
	}
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
		if (PopupWidget->IsInViewport())
		{
			PopupWidget->AchievementPopup(Id);
		}
		else
		{
			PopupWidget->AddToViewport(10);
			
			PopupWidget->AchievementPopup(Id);
		}
	}
	else
	{
		if (PopupWidgetClass)
		{
			if (UPopupWidget* Widget = CreateWidget<UPopupWidget>(GetWorld(), PopupWidgetClass))
			{
				PopupWidget = Widget;
				PopupWidget->AddToViewport(10);
			
				PopupWidget->AchievementPopup(Id);
			}
		}
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
	TotalHeadShots = Save->SavedTotalHeadShots;
	TotalBackStabs = Save->SavedTotalTotalBackStabs;
}

void UAchievementSubsystem::SaveToSave(UStealthSaveGame* Save)
{
	if (!Save)
		return;

	Save->SavedAchievements = AchievementStates;
	Save->SavedTotalEnemiesKilled = TotalEnemiesKilled;
	Save->SavedTotalHelmetsRemoved = TotalHelmetsRemoved;
	Save->SavedTotalHeadShots = TotalHeadShots;
	Save->SavedTotalTotalBackStabs = TotalBackStabs;
}

void UAchievementSubsystem::RestartAchievements()
{
	AchievementStates.Empty();
	TotalEnemiesKilled = 0;
	TotalHelmetsRemoved = 0;
	TotalHeadShots = 0;
	TotalBackStabs = 0;
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


void UAchievementSubsystem::OnHeadShot()
{
	TotalHeadShots++;
	UE_LOG(LogTemp, Log, TEXT("HeadShot. Total HeadShots: %d"), TotalHeadShots);

	if (TotalHeadShots >= 50)
	{
		UnlockAchievement(EAchievementId::Headshot_Fifty_Enemies);
	}
}

void UAchievementSubsystem::OnBackStab()
{
	TotalBackStabs++;
	UE_LOG(LogTemp, Log, TEXT("BackStab. Total BackStabs: %d"), TotalBackStabs);

	if (TotalBackStabs >= 50)
	{
		UnlockAchievement(EAchievementId::Backstab_Fifty_Enemies);
	}
}


// För widgets
int32 UAchievementSubsystem::GetTotalAchievementCount() const
{
	if (!AchievementTable)
	{
		return 0;
	}

	return AchievementTable->GetRowMap().Num();
}

int32 UAchievementSubsystem::GetUnlockedAchievementCount() const
{
	int32 Count = 0;

	for (const auto& Pair : AchievementStates)
	{
		if (Pair.Value)
		{
			Count++;
		}
	}

	return Count;
}

FText UAchievementSubsystem::GetAchievementProgressText() const
{
	const int32 Unlocked = GetUnlockedAchievementCount();
	const int32 Total = GetTotalAchievementCount();

	return FText::Format(
		NSLOCTEXT("Achievements", "ProgressText", "Unlocked {0}/{1}"),
		Unlocked,
		Total
	);
}


float UAchievementSubsystem::GetAchievementProgress() const
{
	const int32 Total = GetTotalAchievementCount();
	if (Total == 0)
	{
		return 0.f;
	}

	return (float)GetUnlockedAchievementCount() / (float)Total;
}
