// Fill out your copyright notice in the Description page of Project Settings.


#include "MissionSelectObject.h"

#include "StealthGameInstance.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AMissionSelectObject::AMissionSelectObject()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
}

void AMissionSelectObject::Use_Implementation(AStealthCharacter* Player)
{
	Super::Use_Implementation(Player);
	
	if (!Player) return;

	if (MissionWidgetClass)
	{
		if (UUserWidget* MissionWidget = CreateWidget<UUserWidget>(GetWorld(), MissionWidgetClass))
		{
			MissionWidget->AddToViewport();
		}
	}
}

// Called when the game starts or when spawned
void AMissionSelectObject::BeginPlay()
{
	Super::BeginPlay();
	
}

