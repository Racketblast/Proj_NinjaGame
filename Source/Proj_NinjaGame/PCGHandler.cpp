// Fill out your copyright notice in the Description page of Project Settings.


#include "PCGHandler.h"

#include "PCGRoom.h"
#include "Components/ArrowComponent.h"
#include "Components/BoxComponent.h"
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
	PCGRoomPlacement();
}

// Called every frame
void APCGHandler::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void APCGHandler::PCGRoomPlacement()
{
	if (AmountOfRooms == 0)
	{
		AmountOfRooms = UKismetMathLibrary::RandomIntegerInRange(1,PossibleRooms.Num()-1);
	}
	
	int AmountOfRoomsLeft = AmountOfRooms;

	for (int i = 0; i < AmountOfRooms; i++)
	{
		bool bRightRoomType = false;
		int Attempts = 0;
		const int MaxAttempts = 200; // tune as necessary

		do
		{
			++Attempts;
			if (Attempts > MaxAttempts)
			{
				UE_LOG(LogTemp, Error, TEXT("Exceeded MaxAttempts (%d) for room %d, giving up on this placement. CurrentOpenEntrances: %d, AmountOfRoomsLeft: %d"),
					MaxAttempts, i, CurrentOpenEntrances.Num(), AmountOfRoomsLeft);
				// force exit the loop for this room to avoid hang; we don't successfully place a room in this iteration
				bRightRoomType = true;
				break;
			}
			
			TSubclassOf<APCGRoom> RoomType = PossibleRooms[UKismetMathLibrary::RandomIntegerInRange(0,PossibleRooms.Num()-1)];
			UE_LOG(LogTemp, Warning, TEXT("New RoomType"));

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
							ExistingEntrances.Append(NewRoom->EntrancesArray);
							
							UE_LOG(LogTemp, Warning, TEXT("Room of type %i was chosen, name: %s"), NewRoom->AmountOfEntrances, *NewRoom->GetActorLabel());
							bRightRoomType = true;
							PlacedRooms.Add(NewRoom);
						}
					}
					else
					{
						AmountOfRoomsLeft--;
						CurrentOpenEntrances.Append(NewRoom->EntrancesArray);
							ExistingEntrances.Append(NewRoom->EntrancesArray);
						UE_LOG(LogTemp, Warning, TEXT("Room of type %i was chosen, name: %s"), NewRoom->AmountOfEntrances, *NewRoom->GetActorLabel());
						bRightRoomType = true;
						PlacedRooms.Add(NewRoom);
					}
				}
			}
			
			//So that it does not get stuck when there can't be anymore rooms
			else if (CurrentOpenEntrances.Num() == 0)
			{
				UE_LOG(LogTemp, Error, TEXT("CurrentOpenEntrances is empty! Cannot generate rooms."));
				bRightRoomType = true;
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
								TArray<UArrowComponent*> OtherRooms = CheckForOtherRooms(NewRoom);
								//OtherRooms.Num() > 1
							
								UE_LOG(LogTemp, Warning, TEXT("This Room %i This Many Entrances %i") , NewRoom->AmountOfEntrances, OtherRooms.Num());
								if (OtherRooms.Num() > 1)
								{
									UE_LOG(LogTemp, Warning, TEXT("Room %i Need to be %i") , NewRoom->AmountOfEntrances, OtherRooms.Num());
									if (NewRoom->AmountOfEntrances == OtherRooms.Num())
									{
										
										//Looks for overlap
										TArray<AActor*> OverlappingActors;
										TArray<APCGRoom*> OverlappingRooms;
										NewRoom->OverlapComponent->GetOverlappingActors(OverlappingActors);
										for (auto OverlappingActor : OverlappingActors)
										{
											if (APCGRoom* OverlappedRoom = Cast<APCGRoom>(OverlappingActor))
											{
												OverlappingRooms.Add(OverlappedRoom);
											}
										}
							
										UE_LOG(LogTemp, Warning, TEXT("This Room has this many overlaps%i"), OverlappingRooms.Num());
										
										if (OverlappingRooms.Num() <= 1)
										{
											//What happens when there are several rooms touching the same spot
											//Make it so that it fits the place
											if (RotateRoomForSeveral(NewRoom, OtherRooms))
											{
												AmountOfRoomsLeft--;
												bRightRoomType = true;
												UE_LOG(LogTemp, Warning, TEXT("Room of type %i was chosen, name: %s"), NewRoom->AmountOfEntrances, *NewRoom->GetActorLabel());
												PlacedRooms.Add(NewRoom);
								
												TArray<UArrowComponent*> ClosedRooms;
												for (auto Arrow : NewRoom->EntrancesArray)
												{
													for (auto OtherRoom : OtherRooms)
													{
														if (Arrow->GetComponentLocation().Equals(OtherRoom->GetComponentLocation(), 0.1f))
														{
															//UE_LOG(LogTemp, Warning, TEXT("New Arrow %s, Old Arrow %s"),*Arrow->GetComponentLocation().ToString(),*CurrentOpenEntrances[RoomPlacement]->GetComponentLocation().ToString());
															ClosedRooms.Add(Arrow);
														}
													}
												}
												for (auto Arrow : ClosedRooms)
												{
													OtherRooms.Remove(Arrow);
													CurrentOpenEntrances.Remove(Arrow);
												}

												CurrentOpenEntrances.Append(OtherRooms);
												ExistingEntrances.Append(OtherRooms);
											}
										}
										else
										{
											UE_LOG(LogTemp, Error, TEXT("Has Overlap"));

											for (APCGRoom* OverlappedRoom : OverlappingRooms)
											{
												if (!OverlappedRoom) continue;
												// Save the entrances to re-add
												CurrentOpenEntrances.Append(OverlappedRoom->EntrancesArray);

												PlacedRooms.Remove(OverlappedRoom);
												OverlappedRoom->Destroy();
											}
											OverlappingRooms.Empty();
										}
									}
								}
								else
								{
									AmountOfRoomsLeft--;
									bRightRoomType = true;
									UE_LOG(LogTemp, Warning, TEXT("Room of type %i was chosen, name: %s"), NewRoom->AmountOfEntrances, *NewRoom->GetActorLabel());
									PlacedRooms.Add(NewRoom);
							
									RotateRoom(NewRoom, RoomPlacement);
							
									for (auto Arrow : NewRoom->EntrancesArray)
									{
										if (!Arrow->GetComponentLocation().Equals(CurrentOpenEntrances[RoomPlacement]->GetComponentLocation(), 0.1f))
										{
											//UE_LOG(LogTemp, Warning, TEXT("New Arrow %s, Old Arrow %s"),*Arrow->GetComponentLocation().ToString(),*CurrentOpenEntrances[RoomPlacement]->GetComponentLocation().ToString());
											CurrentOpenEntrances.Add(Arrow);
											ExistingEntrances.Add(Arrow);
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
			
			UE_LOG(LogTemp, Warning, TEXT("Loop done"));
		}
		while (!bRightRoomType);
	}
	UE_LOG(LogTemp, Warning, TEXT("Open doors %i"), CurrentOpenEntrances.Num());
	UE_LOG(LogTemp, Warning, TEXT("AmountOfRoomsLeft %i"), CurrentOpenEntrances.Num());
}


//Kanske kan använda "FRotator RoomRot = Room->GetActorRotation();" Istället för att faktsikt rotera
void APCGHandler::RotateRoom(APCGRoom* Room, int RoomPlacement)
{
	int RandomDirection = UKismetMathLibrary::RandomIntegerInRange(0,1);
	
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
			//Rotate left
			if (RandomDirection == 1)
			{
				Room->SetActorRotation({Room->GetActorRotation().Pitch,Room->GetActorRotation().Yaw + 90, Room->GetActorRotation().Roll});
			}
			//Rotate Right
			else
			{
				Room->SetActorRotation({Room->GetActorRotation().Pitch,Room->GetActorRotation().Yaw - 90, Room->GetActorRotation().Roll});
			}
		}
	}
}

bool APCGHandler::RotateRoomForSeveral(APCGRoom* Room, TArray<UArrowComponent*> OtherRooms)
{
	if (OtherRooms.Num() == 0)
	{
		return false;
	}
	
	int RandomDirection = UKismetMathLibrary::RandomIntegerInRange(0,1);

	
	//UE_LOG(LogTemp, Warning, TEXT("Connections = %i, entrances = %i"), OtherRooms.Num(), Room->AmountOfEntrances);
	for (int i = 0; i < 4; ++i)
	{
			int ConnectionsOccupied = 0;
			for (int u = 0; u < OtherRooms.Num()-1; ++u)
			{
				for (auto Arrow : Room->EntrancesArray)
				{
					if (Arrow->GetComponentLocation().Equals(OtherRooms[u]->GetComponentLocation(), 0.1f))
					{
						ConnectionsOccupied++;
					}
					if (ConnectionsOccupied == OtherRooms.Num())
					{
						//UE_LOG(LogTemp, Warning, TEXT("Connections have been Occupied %i"), ConnectionsOccupied);
						return true;
					}
				}	
			}

		if (i != 4)
		{
			//Rotate left
			if (RandomDirection == 1)
			{
				Room->SetActorRotation({Room->GetActorRotation().Pitch,Room->GetActorRotation().Yaw + 90, Room->GetActorRotation().Roll});
			}
			//Rotate Right
			else
			{
				Room->SetActorRotation({Room->GetActorRotation().Pitch,Room->GetActorRotation().Yaw - 90, Room->GetActorRotation().Roll});
			}
		}
	}
	
	UE_LOG(LogTemp, Warning, TEXT("Connections have not been Occupied"));
	return false;
}

//This Checks if there are more entrances then one that the room can take
TArray<UArrowComponent*> APCGHandler::CheckForOtherRooms(APCGRoom* Room)
{
	TArray<UArrowComponent*> OtherRoomsConnected;
	if (!Room || Room->EntrancesArray.Num() == 0) return OtherRoomsConnected;

	// Cache world transform once
	const FVector RoomLocation = Room->GetActorLocation();
	const FRotator OriginalRotation = Room->GetActorRotation();

	UArrowComponent* RoomEntranceComp = Room->EntrancesArray[0];
	if (!RoomEntranceComp) return OtherRoomsConnected;
	FVector LocalEntrance = RoomEntranceComp->GetRelativeLocation();
	for (int i = 0; i < 4; ++i)
	{
		FRotator TestRotation = OriginalRotation + FRotator(0.f, i * 90.f, 0.f);
		FVector RotatedEntrance = TestRotation.RotateVector(LocalEntrance) + RoomLocation;
		// Compare against all open entrances
		for (auto Entrance : ExistingEntrances)
		{
			if (RotatedEntrance.Equals(Entrance->GetComponentLocation(), 0.1f))
			{
				OtherRoomsConnected.Add(Entrance);
			}
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("CheckForOtherRooms %d"), OtherRoomsConnected.Num());
	return OtherRoomsConnected;
}
