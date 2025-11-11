// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractableObject.h"

#include "StealthGameInstance.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AInteractableObject::AInteractableObject()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	RootComponent = StaticMeshComponent;

}

void AInteractableObject::Use_Implementation(class AStealthCharacter* Player)
{
	IPlayerUseInterface::Use_Implementation(Player);
	
	if (!Player) return;
	
	if (InteractSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), InteractSound, GetActorLocation());
	}
}

void AInteractableObject::ShowInteractable_Implementation(bool bShow)
{
	IPlayerUseInterface::ShowInteractable_Implementation(bShow);
	
	StaticMeshComponent->SetRenderCustomDepth(bShow);
	if (UStealthGameInstance* GI = Cast<UStealthGameInstance>(GetGameInstance()))
	{
		if (bShow)
		{
			if (HoverSound)
			{
				UGameplayStatics::PlaySoundAtLocation(GetWorld(), HoverSound, GetActorLocation());
			}
	
			GI->CurrentInteractText = InteractText;
		}
		else
		{
			GI->CurrentInteractText = "";
		}
	}
}

// Called when the game starts or when spawned
void AInteractableObject::BeginPlay()
{
	Super::BeginPlay();
	
}

