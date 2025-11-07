// Fill out your copyright notice in the Description page of Project Settings.


#include "PCGHandler.h"

#include "PCGRoom.h"
#include "Algo/RandomShuffle.h"
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
	if (AmountOfRooms <= 0)
	{
		AmountOfRooms = 1;
	}
	
	UE_LOG(LogTemp, Warning, TEXT("Amount of doors that exist %i"), AmountOfRooms);
	int AmountOfRoomsLeft = AmountOfRooms;
	
	//How many rooms I want
	for (int i = 0; i < AmountOfRooms; i++)
	{
		bool bRightRoomType = false;

		
		TArray<TSubclassOf<APCGRoom>> ShuffledRooms = PossibleRooms;
		Algo::RandomShuffle(ShuffledRooms);
		
		int RoomPlacement = UKismetMathLibrary::RandomIntegerInRange(0,CurrentOpenEntrances.Num()-1);
		
		for (auto RoomType : ShuffledRooms)
		{
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
							for (auto Arrow : NewRoom->EntrancesArray)
							{
								FNodeInfo NodeInfo;
								NodeInfo.Arrow = Arrow;
								NodeInfo.Room = NewRoom;
								CurrentOpenEntrances.Add(NodeInfo);
							}
							
							//ExistingEntrances.Append(NewRoom->EntrancesArray);
							
							UE_LOG(LogTemp, Warning, TEXT("Room of type %i was chosen, name: %s"), NewRoom->AmountOfEntrances, *NewRoom->GetActorLabel());
							bRightRoomType = true;

							NewRoom->GridLocation.bOccupied = true;
							NewRoom->GridLocation.Coordinates = {0,0,0};
							OccupiedNodes.Add(NewRoom->GridLocation);
							PlacedRooms.Add(NewRoom);
						}
					}
					else
					{
						AmountOfRoomsLeft--;
						for (auto Arrow : NewRoom->EntrancesArray)
						{
							FNodeInfo NodeInfo;
							NodeInfo.Arrow = Arrow;
							NodeInfo.Room = NewRoom;
							CurrentOpenEntrances.Add(NodeInfo);
						}
						//ExistingEntrances.Append(NewRoom->EntrancesArray);
						UE_LOG(LogTemp, Warning, TEXT("Room of type %i was chosen, name: %s"), NewRoom->AmountOfEntrances, *NewRoom->GetActorLabel());
						bRightRoomType = true;
						
						NewRoom->GridLocation.bOccupied = true;
						NewRoom->GridLocation.Coordinates = {0,0,0};
						OccupiedNodes.Add(NewRoom->GridLocation);
						PlacedRooms.Add(NewRoom);
					}
				}
			}
			
			//Every other Room except the first one
			else
			{
				FTransform EntrancesTransform;
				EntrancesTransform.SetLocation(CurrentOpenEntrances[RoomPlacement].Arrow->GetComponentLocation() - CurrentOpenEntrances[RoomPlacement].Room->GetActorLocation() + CurrentOpenEntrances[RoomPlacement].Arrow->GetComponentLocation());
				NewRoom = GetWorld()->SpawnActor<APCGRoom>(RoomType, EntrancesTransform);
				bRightRoomType = true;
				/*//Puts the room in the right place after getting a random spot
				FTransform EntrancesTransform;

				APCGRoom* RoomThatItsCommingFrom = nullptr;
				for (auto RoomPlacedUpon : PlacedRooms)
				{
					if (RoomPlacedUpon->EntrancesArray.Contains(CurrentOpenEntrances[RoomPlacement]))
					{
						RoomThatItsCommingFrom = RoomPlacedUpon;
					}
				}
				FRotator AlignRot = CurrentOpenEntrances[RoomPlacement]->GetForwardVector().Rotation();
				FVector RotatedOffset = AlignRot.RotateVector(CurrentOpenEntrances[RoomPlacement]->GetRelativeLocation());

				//If the NewRoom touches several entrances
				TArray<UArrowComponent*> OtherRooms = CheckForOtherRooms(NewRoom);

				//Something with OtherRooms.Num()
				
				//Sees if room exists, if the room has fewer entrances then there are rooms that are supposed to spawn and if there will be fewer entrances with every open entrance that exists
				if (NewRoom && NewRoom->AmountOfEntrances <= AmountOfRoomsLeft && NewRoom->AmountOfEntrances - 1 + CurrentOpenEntrances.Num() <= AmountOfRoomsLeft)
				{
					// Looks for a 0 entrance room and throws it out
					if (NewRoom->AmountOfEntrances != 0)
					{
						//This is so the whole floor is not closed off and so that it gets closed off at the end
						if (NewRoom->AmountOfEntrances - CurrentOpenEntrances.Num() != 0 || AmountOfRoomsLeft - 1 == 0)
						{
								//OtherRooms.Num() > 1
							
								UE_LOG(LogTemp, Warning, TEXT("This Room %i This Many Entrances %i") , NewRoom->AmountOfEntrances, OtherRooms.Num());
								if (OtherRooms.Num() > 1)
								{
									UE_LOG(LogTemp, Warning, TEXT("Room %i Need to be %i") , NewRoom->AmountOfEntrances, OtherRooms.Num());
									if (NewRoom->AmountOfEntrances == OtherRooms.Num())
									{
										UE_LOG(LogTemp, Warning, TEXT("Room of type %i was chosen, name: %s"), NewRoom->AmountOfEntrances, *NewRoom->GetActorLabel());
										//What happens when there are several rooms touching the same spot
										//Make it so that it fits the place
										if (RotateRoomForSeveral(NewRoom, OtherRooms))
										{
											AmountOfRoomsLeft--;
											bRightRoomType = true;
											UE_LOG(LogTemp, Warning, TEXT("Room of type %i was chosen, name: %s"), NewRoom->AmountOfEntrances, *NewRoom->GetActorLabel());
											NewRoom->GridLocation.bOccupied = true;
											if (RoomThatItsCommingFrom)
											{
												NewRoom->GridLocation.Coordinates = {CurrentOpenEntrances[RoomPlacement]->GetForwardVector() + RoomThatItsCommingFrom->GridLocation.Coordinates};
											}
											OccupiedNodes.Add(NewRoom->GridLocation);
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
								}
								else
								{
									AmountOfRoomsLeft--;
									bRightRoomType = true;
									UE_LOG(LogTemp, Warning, TEXT("Room of type %i was chosen, name: %s"), NewRoom->AmountOfEntrances, *NewRoom->GetActorLabel());
									NewRoom->GridLocation.bOccupied = true;
									if (RoomThatItsCommingFrom)
									{
										NewRoom->GridLocation.Coordinates = {CurrentOpenEntrances[RoomPlacement]->GetForwardVector() + RoomThatItsCommingFrom->GridLocation.Coordinates};
									}
									OccupiedNodes.Add(NewRoom->GridLocation);
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
					}*/
				}
			
			if (!bRightRoomType && NewRoom)
			{
				NewRoom->Destroy();
				NewRoom = nullptr;
			}
			if (bRightRoomType)
			{
				UE_LOG(LogTemp, Warning, TEXT("Loop done"));
				break;
			}
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("Open doors %i"), CurrentOpenEntrances.Num());
	UE_LOG(LogTemp, Warning, TEXT("AmountOfRoomsLeft %i"), AmountOfRoomsLeft);
	
	/*if (CurrentOpenEntrances.Num() != 0 || AmountOfRoomsLeft != 0)
	{
		
		UE_LOG(LogTemp, Warning, TEXT("Restarts %i"), Restarts);
		if (Restarts >= MaxRestarts)
		{
			Restarts = 0;
			AmountOfRooms--;
			if (AmountOfRooms <= 0)
			{
				bCanRestart = false;
			}
		}
		
		if (bCanRestart)
		{
			Restarts++;
			EmptyVariables();
			PCGRoomPlacement();
		}
	}*/
}

void APCGHandler::EmptyVariables()
{
	for (APCGRoom* Room : PlacedRooms)
	{
		if (IsValid(Room))
		{
			Room->Destroy();
		}
	}

	for (UArrowComponent* Arrow : ExistingEntrances)
	{
		if (IsValid(Arrow) && Arrow->GetOwner() == this)
		{
			Arrow->DestroyComponent();
		}
	}
	OccupiedNodes.Empty();
	PlacedRooms.Empty();
	CurrentOpenEntrances.Empty();
	ExistingEntrances.Empty();

	NewRoom = nullptr;
}


//Kanske kan använda "FRotator RoomRot = Room->GetActorRotation();" Istället för att faktsikt rotera
void APCGHandler::RotateRoom(APCGRoom* Room, int RoomPlacement)
{
	/*int RandomDirection = UKismetMathLibrary::RandomIntegerInRange(0,1);
	
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
	}*/
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
