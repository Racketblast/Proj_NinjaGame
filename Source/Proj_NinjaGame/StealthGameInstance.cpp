// Fill out your copyright notice in the Description page of Project Settings.


#include "StealthGameInstance.h"

#include "DialogueInfo.h"
#include "StealthCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "ThrowableWeapon.h"
#include "GameFramework/GameUserSettings.h"
#include "StealthSaveGame.h"
#include "Components/AudioComponent.h"

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
	Save->SavedMissionsCleared = MissionsCleared;
	Save->SavedOwnThrowWeapon = CurrentOwnThrowWeapon;
	Save->SavedOwnThrowWeaponEnum = CurrentOwnThrowWeaponEnum;
	
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
			MissionsCleared = Save->SavedMissionsCleared;
			CurrentOwnThrowWeapon = Save->SavedOwnThrowWeapon;
			CurrentOwnThrowWeaponEnum = Save->SavedOwnThrowWeaponEnum;
			
			SensitivityScale = Save->SavedSensitivityScale;
			MasterVolumeScale = Save->SavedMasterVolumeScale;
		}
	}
	/*else
	{
		//For Graphics Hardware Check
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
	MissionsCleared = {};
	CurrentOwnThrowWeapon = nullptr;
	CurrentOwnThrowWeaponEnum = EPlayerOwnThrowWeapon::None;
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
			if (MissionsCleared != Save->SavedMissionsCleared)
				return true;
			if (CurrentOwnThrowWeapon != Save->SavedOwnThrowWeapon)
				return true;
			if (CurrentOwnThrowWeaponEnum != Save->SavedOwnThrowWeaponEnum)
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

void UStealthGameInstance::StartDialogue()
{if (!EventDialogueInfo)
	return;

	if (!bCanPlayDialogue)
		return;

	
	if (StartDialogueRowName == "")
		return;
	
	CurrentDialogueRowName = StartDialogueRowName;

	if (FDialogueInfo* Row = EventDialogueInfo->FindRow<FDialogueInfo>(StartDialogueRowName, TEXT("")))
	{
		if (CurrentGameFlag < Row->DialogueFlag || Row->DialogueFlag == 0)
		{
			NextDialogueRowName = Row->NextDialogue;

			//Plays the dialogue for the amount of time the sound plays
			float TimeUntilNextDialogue = 0.0f;
			if ( APawn* Player = Cast<APawn>(UGameplayStatics::GetPlayerPawn(this, 0)))
			{
				if (Row->DialogueSound)
				{
					bDialogueIsPlaying = true;
					DialogueComponent = UGameplayStatics::SpawnSoundAtLocation(
						GetWorld(),
						Row->DialogueSound,
						Player->GetActorLocation()
					);
					TimeUntilNextDialogue = Row->DialogueSound->GetDuration();
				}
			}
		
			//Goes to next dialogue
			//This is the reason why the dialogue is broken into two functions, because it needs a delay between each dialog
			GetWorld()->GetTimerManager().SetTimer(DialogueTimerHandle, this, &UStealthGameInstance::PlayNextDialogue, TimeUntilNextDialogue, false);
		}
	}
}

void UStealthGameInstance::PlayNextDialogue()
{
	if (!EventDialogueInfo)
	return;
	if (NextDialogueRowName != "")
	{
		CurrentDialogueRowName = NextDialogueRowName;
		if (FDialogueInfo* Row = EventDialogueInfo->FindRow<FDialogueInfo>(NextDialogueRowName, TEXT("")))
		{
			NextDialogueRowName = Row->NextDialogue;
			//Plays the dialogue for the amount of time the sound plays
			float TimeUntilNextDialogue = 0.0f;
			if ( APawn* Player = Cast<APawn>(UGameplayStatics::GetPlayerPawn(this, 0)))
			{
				if (Row->DialogueSound)
				{
					DialogueComponent = UGameplayStatics::SpawnSoundAtLocation(
						GetWorld(),
						Row->DialogueSound,
						Player->GetActorLocation()
					);
					TimeUntilNextDialogue = Row->DialogueSound->GetDuration();
				}
			}
			
			//Goes to next dialogue
			GetWorld()->GetTimerManager().SetTimer(DialogueTimerHandle, this, &UStealthGameInstance::PlayNextDialogue, TimeUntilNextDialogue, false);
		}
	}
	//If dialogue is over
	else
	{
		bDialogueIsPlaying = false;
	}
}

void UStealthGameInstance::StopDialogue()
{
	GetWorld()->GetTimerManager().ClearTimer(DialogueTimerHandle);

	if (DialogueComponent && DialogueComponent->IsPlaying())
	{
		DialogueComponent->Stop();
	}

	bDialogueIsPlaying = false;
	StartDialogueRowName = "";
	CurrentDialogueRowName = "";
	NextDialogueRowName = "";
}

float UStealthGameInstance::GetDialogueDuration()
{
	//Plays the dialogue for the amount of time the sound plays
	float TimeUntilNextDialogue = 0.0f;
	
	if (CurrentDialogueRowName != "")
	{
		if (FDialogueInfo* Row = EventDialogueInfo->FindRow<FDialogueInfo>(CurrentDialogueRowName, TEXT("")))
		{
			if (Row->DialogueSound)
			{
				TimeUntilNextDialogue = Row->DialogueSound->GetDuration();
			}
		}
	}
	return TimeUntilNextDialogue;
}

void UStealthGameInstance::SwitchOwnWeapon(EPlayerOwnThrowWeapon WeaponToSwitchTo)
{
	if (AStealthCharacter* Player = Cast<AStealthCharacter>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0)))
	{
		switch (WeaponToSwitchTo)
		{
		case EPlayerOwnThrowWeapon::None:
			CurrentOwnThrowWeaponEnum = EPlayerOwnThrowWeapon::None;
			break;
		case EPlayerOwnThrowWeapon::Kunai:
			CurrentOwnThrowWeaponEnum = EPlayerOwnThrowWeapon::Kunai;
			CurrentOwnThrowWeapon = Player->KunaiWeapon;
			break;
		case EPlayerOwnThrowWeapon::SmokeBomb:
			CurrentOwnThrowWeaponEnum = EPlayerOwnThrowWeapon::SmokeBomb;
			CurrentOwnThrowWeapon = Player->SmokeBombWeapon;
			break;
		}

		Player->AmountOfOwnWeapon = Player->MaxAmountOfOwnWeapon;
		Player->CurrentOwnThrowWeapon = CurrentOwnThrowWeapon;
		Player->EquipThrowWeapon(Player->CurrentOwnThrowWeapon);
	}
}
