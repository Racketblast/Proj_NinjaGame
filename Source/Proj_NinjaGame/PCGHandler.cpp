// Fill out your copyright notice in the Description page of Project Settings.


#include "PCGHandler.h"

#include "PCGRoom.h"
#include "Components/ArrowComponent.h"
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
		AmountOfRooms = UKismetMathLibrary::RandomIntegerInRange(1,PossibleRooms.Num()-1);
	}
	
	int AmountOfRoomsLeft = AmountOfRooms;
	TArray<UArrowComponent*> CurrentOpenEntrances;

	for (int i = 0; i < AmountOfRooms; i++)
	{
		bool bRightRoomType = false;
		do
		{
			
			TSubclassOf<APCGRoom> RoomType = PossibleRooms[UKismetMathLibrary::RandomIntegerInRange(0,PossibleRooms.Num()-1)];
			APCGRoom* Room = RoomType->GetDefaultObject<APCGRoom>();
			
			if (i == 0)
			{
				if (Room && Room->AmountOfEntrances < AmountOfRoomsLeft)
				{
					AmountOfRoomsLeft--;
					CurrentOpenEntrances.Append(Room->EntrancesArray);
					UE_LOG(LogTemp, Warning, TEXT("%i"), CurrentOpenEntrances.Num());
					bRightRoomType = true;
					GetWorld()->SpawnActor<APCGRoom>(RoomType, GetTransform());
				}
			}
			else
			{
				
				bRightRoomType = true;
				/*if (Room && Room->AmountOfEntrances <= AmountOfRoomsLeft)
				{
					int RoomPlacement = UKismetMathLibrary::RandomIntegerInRange(0,CurrentOpenEntrances.Num()-1);
					AmountOfRoomsLeft--;
					GetWorld()->SpawnActor<APCGRoom>(RoomType, CurrentOpenEntrances[RoomPlacement]->GetComponentTransform());
					bRightRoomType = true;
					
					for (auto Arrow : Room->EntrancesArray)
					{
						if (Arrow->GetComponentLocation() != CurrentOpenEntrances[RoomPlacement]->GetComponentLocation())
						{
							CurrentOpenEntrances.Add(Arrow);
						}	
					}
					
					CurrentOpenEntrances.RemoveAt(RoomPlacement);
				}*/
			}
			
		}
		while (bRightRoomType == false);
	}
}

// Called every frame
void APCGHandler::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}