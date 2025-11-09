// Fill out your copyright notice in the Description page of Project Settings.


#include "StealthGameInstance.h"

void UStealthGameInstance::Init()
{
	Super::Init();
	
	UGameUserSettings* Settings = GEngine->GetGameUserSettings();
	if (Settings)
	{
		Settings->SetOverallScalabilityLevel(0);
		Settings->ApplySettings(false);
	}
	
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
		Save = Cast<UGhostCatchersSaveGame>(UGameplayStatics::CreateSaveGameObject(SaveGameObject));
		if (Save)
		{
			FillSaveGame();
			UGameplayStatics::SaveGameToSlot(Save,"Save1", 0);
		}
	}
}

void UStealthGameInstance::FillSaveGame()
{
	Save->SavedCurrentGameFlag = CurrentGameFlag;
	Save->SavedCurrentFloor = CurrentFloor;
	Save->SavedFloorsUnlocked = FloorsUnlocked;
	
	Save->SavedMasterVolumeScale = MasterVolumeScale;
	Save->SavedFootStepsVolumeScale = FootStepsVolumeScale;
	Save->SavedSpotVolumeScale = SpotVolumeScale;
	Save->SavedGhostVolumeScale = GhostVolumeScale;
	Save->SavedRhythmVolumeScale = RhythmVolumeScale;
	Save->SavedDialogueVolumeScale = DialogueVolumeScale;
	Save->SavedTextReaderVolumeScale = TextReaderVolumeScale;
}

void UStealthGameInstance::LoadGame()
{
	//Loads the saved game
	if (UGameplayStatics::DoesSaveGameExist("Save1",0))
	{
		Save = Cast<UGhostCatchersSaveGame>(UGameplayStatics::LoadGameFromSlot("Save1",0));
		if (Save)
		{
			CurrentGameFlag = Save->SavedCurrentGameFlag;
			CurrentFloor = Save->SavedCurrentFloor;
			FloorsUnlocked = Save->SavedFloorsUnlocked;
	
			MasterVolumeScale = Save->SavedMasterVolumeScale;
			FootStepsVolumeScale = Save->SavedFootStepsVolumeScale;
			SpotVolumeScale = Save->SavedSpotVolumeScale;
			GhostVolumeScale = Save->SavedGhostVolumeScale;
			RhythmVolumeScale = Save->SavedRhythmVolumeScale;
			DialogueVolumeScale = Save->SavedDialogueVolumeScale;
			TextReaderVolumeScale = Save->SavedTextReaderVolumeScale;
		}
	}
	//For Graphics if needed
	/*else
	{
		UGameUserSettings::GetGameUserSettings()->RunHardwareBenchmark();
		UGameUserSettings::GetGameUserSettings()->ApplySettings(true);
	}*/
}

void UStealthGameInstance::RestartGame()
{
	CurrentGameFlag = 0;
	CurrentFloor = 1;  // Ändrar från 0 till 1
	FloorsUnlocked = {1}; // Lägger till 1, så vi startar på level 1
}

bool UStealthGameInstance::HasGameChanged()
{
	if (UGameplayStatics::DoesSaveGameExist("Save1",0))
	{
		Save = Cast<UGhostCatchersSaveGame>(UGameplayStatics::LoadGameFromSlot("Save1",0));
		if (Save)
		{
			if (CurrentGameFlag != Save->SavedCurrentGameFlag)
				return true;
			if (CurrentFloor != Save->SavedCurrentFloor)
				return true;
			if (FloorsUnlocked != Save->SavedFloorsUnlocked)
				return true;

			if (MasterVolumeScale != Save->SavedMasterVolumeScale)
				return true;
			if (FootStepsVolumeScale != Save->SavedFootStepsVolumeScale)
				return true;
			if (SpotVolumeScale != Save->SavedSpotVolumeScale)
				return true;
			if (GhostVolumeScale != Save->SavedGhostVolumeScale)
				return true;
			if (RhythmVolumeScale != Save->SavedRhythmVolumeScale)
				return true;
			if (DialogueVolumeScale != Save->SavedDialogueVolumeScale)
				return true;
			if (TextReaderVolumeScale != Save->SavedTextReaderVolumeScale)
				return true;
		}
		return false;
	}
	
	return true;
}