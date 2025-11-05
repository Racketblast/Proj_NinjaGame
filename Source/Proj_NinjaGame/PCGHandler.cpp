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
	APCGRoom* Room;

	for (int i = 0; i < AmountOfRooms; i++)
	{
		bool bRightRoomType = false;
		do
		{
			TSubclassOf<APCGRoom> RoomType = PossibleRooms[UKismetMathLibrary::RandomIntegerInRange(0,PossibleRooms.Num()-1)];

			//First Room made
			if (i == 0)
			{
				Room = GetWorld()->SpawnActor<APCGRoom>(RoomType, GetTransform());
				if (Room && Room->AmountOfEntrances < AmountOfRoomsLeft)
				{
					//So if Room only is one
					if (Room->AmountOfEntrances == 0)
					{
						if ( AmountOfRooms == 1)
						{
							AmountOfRoomsLeft--;
							CurrentOpenEntrances.Append(Room->EntrancesArray);
							
							UE_LOG(LogTemp, Warning, TEXT("Room of type %i was chosen, name: %s"), Room->AmountOfEntrances, *Room->GetActorLabel());
							bRightRoomType = true;
						}
					}
					else
					{
						AmountOfRoomsLeft--;
						CurrentOpenEntrances.Append(Room->EntrancesArray);
						UE_LOG(LogTemp, Warning, TEXT("Room of type %i was chosen, name: %s"), Room->AmountOfEntrances, *Room->GetActorLabel());
						bRightRoomType = true;
					}
				}
			}
			
			//Every other Room except the first one
			else
			{
				int RoomPlacement = UKismetMathLibrary::RandomIntegerInRange(0,CurrentOpenEntrances.Num()-1);
				FTransform EntrancesTransform;

				FRotator AlignRot = CurrentOpenEntrances[RoomPlacement]->GetForwardVector().Rotation();
				FVector RotatedOffset = AlignRot.RotateVector(CurrentOpenEntrances[RoomPlacement]->GetRelativeLocation());
				
				EntrancesTransform.SetLocation(CurrentOpenEntrances[RoomPlacement]->GetComponentLocation() + RotatedOffset);
				Room = GetWorld()->SpawnActor<APCGRoom>(RoomType, EntrancesTransform);
				
				if (Room && Room->AmountOfEntrances <= AmountOfRoomsLeft && Room->AmountOfEntrances - 1 + CurrentOpenEntrances.Num() <= AmountOfRoomsLeft)
				{
					if (Room->AmountOfEntrances != 0)
					{
						if (Room->AmountOfEntrances - CurrentOpenEntrances.Num() != 0)
						{
							AmountOfRoomsLeft--;
							bRightRoomType = true;
							UE_LOG(LogTemp, Warning, TEXT("Room of type %i was chosen, name: %s"), Room->AmountOfEntrances, *Room->GetActorLabel());
							
							RotateRoom(Room, RoomPlacement);
							
							for (auto Arrow : Room->EntrancesArray)
							{
								if (!Arrow->GetComponentLocation().Equals(CurrentOpenEntrances[RoomPlacement]->GetComponentLocation(), 0.1f))
								{
									CurrentOpenEntrances.Add(Arrow);
								}	
							}
					
							CurrentOpenEntrances.RemoveAt(RoomPlacement);
						}
						else if (AmountOfRoomsLeft - 1 == 0)
						{
							AmountOfRoomsLeft--;
							bRightRoomType = true;
							UE_LOG(LogTemp, Warning, TEXT("Room of type %i was chosen, name: %s"), Room->AmountOfEntrances, *Room->GetActorLabel());

							RotateRoom(Room, RoomPlacement);
							
							for (auto Arrow : Room->EntrancesArray)
							{
								if (!Arrow->GetComponentLocation().Equals(CurrentOpenEntrances[RoomPlacement]->GetComponentLocation()))
								{
									//UE_LOG(LogTemp, Warning, TEXT("New Arrow %s, Old Arrow %s"),*Arrow->GetComponentLocation().ToString(),*CurrentOpenEntrances[RoomPlacement]->GetComponentLocation().ToString());
									CurrentOpenEntrances.Add(Arrow);
								}	
							}
							
							CurrentOpenEntrances.RemoveAt(RoomPlacement);
						}
					}
				}
			}
			
			if (!bRightRoomType && Room)
			{
				Room->Destroy();
			}
		}
		while (!bRightRoomType);
	}
	UE_LOG(LogTemp, Warning, TEXT("Open doors %i"), CurrentOpenEntrances.Num());
}

// Called every frame
void APCGHandler::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}



void APCGHandler::RotateRoom(APCGRoom* Room, int RoomPlacement)
{
	int RandomDirection = UKismetMathLibrary::RandomIntegerInRange(0,1);
	//Rotate left
	if (RandomDirection == 1)
	{
		for (int u = 0; u < 4; ++u)
		{
			for (auto Arrow : Room->EntrancesArray)
			{
				if (Arrow->GetComponentLocation().Equals(CurrentOpenEntrances[RoomPlacement]->GetComponentLocation(), 0.1f))
				{
					u = 4;
											
					break;
				}	
			}

			if (u != 4)
			{
				Room->SetActorRotation({Room->GetActorRotation().Pitch,Room->GetActorRotation().Yaw + 90, Room->GetActorRotation().Roll});
			}
		}
								
	}
	//Rotate Right
	else
	{
		for (int u = 0; u < 4; ++u)
		{
			for (auto Arrow : Room->EntrancesArray)
			{
				if (Arrow->GetComponentLocation().Equals(CurrentOpenEntrances[RoomPlacement]->GetComponentLocation(), 0.1f))
				{
					u = 4;
											
					break;
				}	
			}

			if (u != 4)
			{
				Room->SetActorRotation({Room->GetActorRotation().Pitch,Room->GetActorRotation().Yaw - 90, Room->GetActorRotation().Roll});
			}
		}
	}
}