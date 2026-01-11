// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractableVFXObject.h"
#include "NiagaraComponent.h"
#include "SoundUtility.h"
#include "Components/AudioComponent.h"


AInteractableVFXObject::AInteractableVFXObject()
{
	PrimaryActorTick.bCanEverTick = false;

	// Audio
	AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
	AudioComponent->SetupAttachment(RootComponent);
	AudioComponent->bAutoActivate = false;

	// VFX
	VFXComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("VFXComponent"));
	VFXComponent->SetupAttachment(RootComponent);
	VFXComponent->SetAutoActivate(false);
}

void AInteractableVFXObject::BeginPlay()
{
	Super::BeginPlay();

	if (LoopingSound)
	{
		AudioComponent->SetSound(LoopingSound);
		USoundUtility::ReportNoise(GetWorld(), GetActorLocation(), NoiseLevel); 
	}

	if (LoopingVFX)
	{
		VFXComponent->SetAsset(LoopingVFX);
	}
}

void AInteractableVFXObject::Use_Implementation(AStealthCharacter* Player)
{
	Super::Use_Implementation(Player);

	if (bIsActive)
	{
		TurnOff();
	}
	else
	{
		TurnOn();
	}
}

void AInteractableVFXObject::TurnOn()
{
	bIsActive = true;

	if (AudioComponent && !AudioComponent->IsPlaying())
	{
		AudioComponent->Play();
	}

	if (VFXComponent)
	{
		VFXComponent->Activate();
	}
}

void AInteractableVFXObject::TurnOff()
{
	bIsActive = false;

	if (AudioComponent && AudioComponent->IsPlaying())
	{
		AudioComponent->Stop();
	}

	if (VFXComponent)
	{
		VFXComponent->Deactivate();
	}
}