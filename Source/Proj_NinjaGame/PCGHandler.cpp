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

	for (int i = 0; i < AmountOfRooms; i++)
	{
		bool bRightRoomType = false;
		do
		{
			TSubclassOf<APCGRoom> RoomType = PossibleRooms[UKismetMathLibrary::RandomIntegerInRange(0,PossibleRooms.Num()-1)];

			//First Room made
			if (i == 0)
			{
				NewRoom = GetWorld()->SpawnActor<APCGRoom>(RoomType, GetTransform());
				if (NewRoom && NewRoom->AmountOfEntrances < AmountOfRoomsLeft)
				{
					//So if Room only is one
					if (NewRoom->AmountOfEntrances == 0)
					{
						if ( AmountOfRooms == 1)
						{
							AmountOfRoomsLeft--;
							CurrentOpenEntrances.Append(NewRoom->EntrancesArray);
							
							UE_LOG(LogTemp, Warning, TEXT("Room of type %i was chosen, name: %s"), NewRoom->AmountOfEntrances, *NewRoom->GetActorLabel());
							bRightRoomType = true;
						}
					}
					else
					{
						AmountOfRoomsLeft--;
						CurrentOpenEntrances.Append(NewRoom->EntrancesArray);
						UE_LOG(LogTemp, Warning, TEXT("Room of type %i was chosen, name: %s"), NewRoom->AmountOfEntrances, *NewRoom->GetActorLabel());
						bRightRoomType = true;
					}
				}
			}
			
			//Every other Room except the first one
			else
			{
				//Puts the room in the right place after getting a random spot
				int RoomPlacement = UKismetMathLibrary::RandomIntegerInRange(0,CurrentOpenEntrances.Num()-1);
				FTransform EntrancesTransform;

				FRotator AlignRot = CurrentOpenEntrances[RoomPlacement]->GetForwardVector().Rotation();
				FVector RotatedOffset = AlignRot.RotateVector(CurrentOpenEntrances[RoomPlacement]->GetRelativeLocation());
				
				EntrancesTransform.SetLocation(CurrentOpenEntrances[RoomPlacement]->GetComponentLocation() + RotatedOffset);
				NewRoom = GetWorld()->SpawnActor<APCGRoom>(RoomType, EntrancesTransform);

				//Sees if room exists, if the room has fewer entrances then there are rooms that are supposed to spawn and if there will be fewer entrances with every open entrance that exists
				if (NewRoom && NewRoom->AmountOfEntrances <= AmountOfRoomsLeft && NewRoom->AmountOfEntrances - 1 + CurrentOpenEntrances.Num() <= AmountOfRoomsLeft)
				{
					// Looks for a 0 entrance room and throws it out
					if (NewRoom->AmountOfEntrances != 0)
					{
						//This is so the whole floor is not closed off and so that it gets closed off at the end
						if (NewRoom->AmountOfEntrances - CurrentOpenEntrances.Num() != 0 || AmountOfRoomsLeft - 1 == 0)
						{
							//If the NewRoom touches several entrances
							if (CheckForOtherRooms(NewRoom))
							{
								//What happens when there are several rooms touching the same spot
							}
							else
							{
								AmountOfRoomsLeft--;
								bRightRoomType = true;
								UE_LOG(LogTemp, Warning, TEXT("Room of type %i was chosen, name: %s"), NewRoom->AmountOfEntrances, *NewRoom->GetActorLabel());
							
								RotateRoom(NewRoom, RoomPlacement);
							
								for (auto Arrow : NewRoom->EntrancesArray)
								{
									if (!Arrow->GetComponentLocation().Equals(CurrentOpenEntrances[RoomPlacement]->GetComponentLocation(), 0.1f))
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
			}
			
			if (!bRightRoomType && NewRoom)
			{
				NewRoom->Destroy();
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


//Kanske kan använda "FRotator RoomRot = Room->GetActorRotation();" Istället för att faktsikt rotera
void APCGHandler::RotateRoom(APCGRoom* Room, int RoomPlacement)
{
	int RandomDirection = UKismetMathLibrary::RandomIntegerInRange(0,1);
	//Rotate left
	if (RandomDirection == 1)
	{
		for (int i = 0; i < 4; ++i)
		{
			for (auto Arrow : Room->EntrancesArray)
			{
				if (Arrow->GetComponentLocation().Equals(CurrentOpenEntrances[RoomPlacement]->GetComponentLocation(), 0.1f))
				{
					i = 4;
											
					break;
				}	
			}

			if (i != 4)
			{
				Room->SetActorRotation({Room->GetActorRotation().Pitch,Room->GetActorRotation().Yaw + 90, Room->GetActorRotation().Roll});
			}
		}
								
	}
	//Rotate Right
	else
	{
		for (int i = 0; i < 4; ++i)
		{
			for (auto Arrow : Room->EntrancesArray)
			{
				if (Arrow->GetComponentLocation().Equals(CurrentOpenEntrances[RoomPlacement]->GetComponentLocation(), 0.1f))
				{
					i = 4;
											
					break;
				}	
			}

			if (i != 4)
			{
				Room->SetActorRotation({Room->GetActorRotation().Pitch,Room->GetActorRotation().Yaw - 90, Room->GetActorRotation().Roll});
			}
		}
	}
}

//This Checks if there are more entrances then one that the room can take
bool APCGHandler::CheckForOtherRooms(APCGRoom* Room)
{
	if (!Room || Room->EntrancesArray.Num() == 0) return false;

	FVector RoomLocation = Room->GetActorLocation();
	FRotator OriginalRotation = Room->GetActorRotation();

	UArrowComponent* RoomEntranceComp = Room->EntrancesArray[0];
	if (!RoomEntranceComp) return false;

	TArray<UArrowComponent*> OtherRoomsConnected;

	FVector LocalEntrance = RoomEntranceComp->GetRelativeLocation();

	for (int i = 0; i < 4; ++i)
	{
		FRotator TestRotation = OriginalRotation + FRotator(0.f, i * 90.f, 0.f);
		FVector RotatedEntrance = TestRotation.RotateVector(LocalEntrance) + RoomLocation;

		// Compare against all open entrances
		for (auto Entrance : CurrentOpenEntrances)
		{
			if (RotatedEntrance.Equals(Entrance->GetComponentLocation(), 0.1f))
			{
				OtherRoomsConnected.Add(Entrance);
			}
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("CheckForOtherRooms %i") , OtherRoomsConnected.Num());
	return OtherRoomsConnected.Num() > 1;
}
