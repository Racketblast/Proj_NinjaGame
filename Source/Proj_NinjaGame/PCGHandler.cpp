// Fill out your copyright notice in the Description page of Project Settings.


#include "PCGHandler.h"

#include "PCGRoom.h"
#include "Kismet/KismetMathLibrary.h"

APCGHandler::APCGHandler()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}


// Called when the game starts or when spawned
void APCGHandler::BeginPlay()
{
	Super::BeginPlay();
	if (AmountOfRooms == 0)
	{
		AmountOfRooms = UKismetMathLibrary::RandomIntegerInRange(0,PossibleRooms.Num()-1);
	}
	int AmountOfRoomsLeft = AmountOfRooms;
	
	for (auto RoomType : PossibleRooms)
	{
		APCGRoom* Room = RoomType->GetDefaultObject<APCGRoom>();
		if (Room && Room->Entrances <= AmountOfRooms)
		{
			AmountOfRoomsLeft--;
			GetWorld()->SpawnActor<APCGRoom>(RoomType, GetActorTransform());
		}
	}
}

// Called every frame
void APCGHandler::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}