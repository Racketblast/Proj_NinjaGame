// Fill out your copyright notice in the Description page of Project Settings.


#include "StealthGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "StealthSaveGame.h"

void UStealthGameInstance::Init()
{
	Super::Init();
	
	/*UGameUserSettings* Settings = GEngine->GetGameUserSettings();
	if (Settings)
	{
		Settings->SetOverallScalabilityLevel(0);
		Settings->ApplySettings(false);
	}*/
	
	//Loads the saved game
	LoadGame();
}

void UStealthGameInstance::SaveGame()
{
	if (UGameplayStatics::DoesSaveGameExist("Save1", 0))
	{
		FillSaveGame();
		UGameplayStatics::SaveGameToSlot(Save,"Save1", 0);
	}
	else if (SaveGameObject)
	{
		Save = Cast<UStealthSaveGame>(UGameplayStatics::CreateSaveGameObject(SaveGameObject));
		if (Save)
		{
			FillSaveGame();
			UGameplayStatics::SaveGameToSlot(Save,"Save1", 0);
		}
	}
}


void UStealthGameInstance::SaveOptions()
{
	if (UGameplayStatics::DoesSaveGameExist("Save1", 0))
	{
		FillSaveOptions();
		UGameplayStatics::SaveGameToSlot(Save,"Save1", 0);
	}
}

void UStealthGameInstance::FillSaveGame()
{
	//Saves Gameplay data
	Save->SavedCurrentGameFlag = CurrentGameFlag;
	Save->SavedLevelsUnlocked = LevelsUnlocked;
	
	//Saves options data
	FillSaveOptions();
}

void UStealthGameInstance::FillSaveOptions()
{
	Save->SavedMasterVolumeScale = MasterVolumeScale;
	Save->SavedSensitivityScale = SensitivityScale;
}

void UStealthGameInstance::LoadGame()
{
	//Loads the saved game
	if (UGameplayStatics::DoesSaveGameExist("Save1",0))
	{
		Save = Cast<UStealthSaveGame>(UGameplayStatics::LoadGameFromSlot("Save1",0));
		if (Save)
		{
			CurrentGameFlag = Save->SavedCurrentGameFlag;
			LevelsUnlocked = Save->SavedLevelsUnlocked;
			
			SensitivityScale = Save->SavedSensitivityScale;
			MasterVolumeScale = Save->SavedMasterVolumeScale;
		}
	}
	
	//For Graphics if needed
	/*else
	{
		UGameUserSettings::GetGameUserSettings()->RunHardwareBenchmark();
		UGameUserSettings::GetGameUserSettings()->ApplySettings(true);
	}*/
}

void UStealthGameInstance::LoadOptions()
{
	if (UGameplayStatics::DoesSaveGameExist("Save1",0))
	{
		Save = Cast<UStealthSaveGame>(UGameplayStatics::LoadGameFromSlot("Save1",0));
		if (Save)
		{
			SensitivityScale = Save->SavedSensitivityScale;
			MasterVolumeScale = Save->SavedMasterVolumeScale;
		}
	}
}

void UStealthGameInstance::RestartGame()
{
	CurrentGameFlag = 0;
	LevelsUnlocked = {};
}

bool UStealthGameInstance::HasGameChanged()
{
	if (UGameplayStatics::DoesSaveGameExist("Save1",0))
	{
		Save = Cast<UStealthSaveGame>(UGameplayStatics::LoadGameFromSlot("Save1",0));
		if (Save)
		{
			if (CurrentGameFlag != Save->SavedCurrentGameFlag)
				return true;
			if (LevelsUnlocked != Save->SavedLevelsUnlocked)
				return true;
			
			if (SensitivityScale != Save->SavedSensitivityScale)
				return true;
			if (MasterVolumeScale != Save->SavedMasterVolumeScale)
				return true;
		}
		return false;
	}
	
	return true;
}