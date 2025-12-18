// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractableAudioObject.h"

#include "AchievementSubsystem.h"
#include "SoundUtility.h"
#include "Components/AudioComponent.h"


AInteractableAudioObject::AInteractableAudioObject()
{
	PrimaryActorTick.bCanEverTick = false;

	AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
	AudioComponent->SetupAttachment(RootComponent);
	AudioComponent->bAutoActivate = false;
}

void AInteractableAudioObject::BeginPlay()
{
	Super::BeginPlay();

	if (OneShotSound)
	{
		AudioComponent->SetSound(OneShotSound);
		USoundUtility::ReportNoise(GetWorld(), GetActorLocation(), NoiseLevel); 
	}

	AudioComponent->OnAudioFinished.AddDynamic(this, &AInteractableAudioObject::OnAudioFinished);
}

void AInteractableAudioObject::Use_Implementation(AStealthCharacter* Player)
{
	if (bIsPlaying)
	{
		return;
	}

	//Super::Use_Implementation(Player);

	if (!AudioComponent || !OneShotSound)
	{
		return;
	}

	bIsPlaying = true;
	AudioComponent->Play();

	if (bToilet)
	{
		if (UGameInstance* GI = GetGameInstance())
		{
			if (UAchievementSubsystem* Achievements = GI->GetSubsystem<UAchievementSubsystem>())
			{
				Achievements->UnlockAchievement(EAchievementId::Use_The_Toilet);
			}
		}
	}
}

void AInteractableAudioObject::OnAudioFinished()
{
	bIsPlaying = false;
}

void AInteractableAudioObject::ShowInteractable_Implementation(bool bShow)
{
	/*if (bIsPlaying)
	{
		return;
	}*/
	
	Super::ShowInteractable_Implementation(bShow);
}