// Fill out your copyright notice in the Description page of Project Settings.


#include "PCGRoom.h"

#include "Components/ArrowComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"

APCGRoom::APCGRoom()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	RoomRootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RoomRootComponent"));
	RootComponent = RoomRootComponent;
	OverlapComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("OverlapComponent"));
	OverlapComponent->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void APCGRoom::BeginPlay()
{
	Super::BeginPlay();
	
	TArray<UArrowComponent*> Arrows;
	GetComponents<UArrowComponent>(Arrows);

	EntrancesArray.Append(Arrows);
	AmountOfEntrances = EntrancesArray.Num();
}

// Called every frame
void APCGRoom::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}