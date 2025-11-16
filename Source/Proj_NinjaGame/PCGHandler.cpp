// Fill out your copyright notice in the Description page of Project Settings.


#include "PCGHandler.h"

#include "PCGRoom.h"
#include "Algo/RandomShuffle.h"
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
	PCGRoomPlacement();
}

// Called every frame
void APCGHandler::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

FVector APCGHandler::WorldToGrid(const FVector& EntranceWorldLocation,const FVector& GridLocation) const
{
	FVector RelativeLocation = EntranceWorldLocation - GridToWorld(GridLocation);
	int32 X = FMath::RoundToInt((EntranceWorldLocation.X + RelativeLocation.X) / CellSize);
	int32 Y = FMath::RoundToInt((EntranceWorldLocation.Y + RelativeLocation.Y) / CellSize);
	int32 Z = FMath::RoundToInt((EntranceWorldLocation.Z + RelativeLocation.Z) / CellSize);
	return FVector(X, Y, Z);
}

FVector APCGHandler::GridToWorld(const FVector& GridLocation) const
{
    return FVector(GridLocation.X * CellSize, GridLocation.Y * CellSize, GridLocation.Z * CellSize);
}


void APCGHandler::PCGRoomPlacement()
{
	if (AmountOfRooms <= 0)
	{
		AmountOfRooms = 1;
	}
	
	UE_LOG(LogTemp, Warning, TEXT("Amount of doors that exist %i"), AmountOfRooms);
	int AmountOfRoomsLeft = AmountOfRooms;
	
	CreateFirstRoom(AmountOfRoomsLeft);
	CreateNextRooms(AmountOfRoomsLeft);
}


void APCGHandler::CreateFirstRoom(int &AmountOfRoomsLeft)
{
	UE_LOG(LogTemp, Warning, TEXT("AmountOfRoomsLeft: %i"), AmountOfRoomsLeft);
	//First Room made
	bool bRightRoomType = false;
	TArray<TSubclassOf<APCGRoom>> ShuffledRooms = PossibleRooms;
	Algo::RandomShuffle(ShuffledRooms);
	
	for (auto RoomType : ShuffledRooms)
	{
		NewRoom = GetWorld()->SpawnActor<APCGRoom>(RoomType, GetTransform());
		if (NewRoom && NewRoom->AmountOfEntrances < AmountOfRoomsLeft)
		{
			if (NewRoom->AmountOfEntrances == 0)
			{
				if (AmountOfRoomsLeft == 1)
				{
					NewRoom->GridLocation.bOccupied = true;
					NewRoom->GridLocation.Coordinates = {0,0,0};

					//New open rooms
					for (auto OpenRoom : NewRoom->EntrancesArray)
					{
						NewRoom->GridLocation.OpenDoors.Add(WorldToGrid(OpenRoom->GetComponentLocation(), NewRoom->GridLocation.Coordinates));
					}
			
					AmountOfRoomsLeft--;

					//UE_LOG(LogTemp, Warning, TEXT("Room of type %i was chosen, name: %s"), NewRoom->AmountOfEntrances, *NewRoom->GetActorLabel());
					bRightRoomType = true;
					OccupiedNodeLocations.Add(NewRoom->GridLocation);
					PlacedRooms.Add(NewRoom);
				}
			}
			else
			{
				NewRoom->GridLocation.bOccupied = true;
				NewRoom->GridLocation.Coordinates = {0,0,0};

				//New open rooms
				for (auto OpenRoom : NewRoom->EntrancesArray)
				{
					NewRoom->GridLocation.OpenDoors.Add(WorldToGrid(OpenRoom->GetComponentLocation(), NewRoom->GridLocation.Coordinates));
					OpenNodeLocations.Add(WorldToGrid(OpenRoom->GetComponentLocation(), NewRoom->GridLocation.Coordinates));
				}
			
				AmountOfRoomsLeft--;

				//UE_LOG(LogTemp, Warning, TEXT("Room of type %i was chosen, name: %s"), NewRoom->AmountOfEntrances, *NewRoom->GetActorLabel());
				bRightRoomType = true;
				OccupiedNodeLocations.Add(NewRoom->GridLocation);
				PlacedRooms.Add(NewRoom);
			}
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

void APCGHandler::CreateNextRooms(int &AmountOfRoomsLeft)
{
	int AmountOfRoomsToPlace = AmountOfRoomsLeft;
	
	if (AmountOfRoomsToPlace == 0 || OpenNodeLocations.Num() == 0)
	{
		return;
	}
	
	for (int i = 0; i < AmountOfRoomsToPlace; ++i)
	{
		bool bRightRoomType = false;
		TArray<TSubclassOf<APCGRoom>> ShuffledRooms = PossibleRooms;
		Algo::RandomShuffle(ShuffledRooms);
		
		int RoomPlacement = UKismetMathLibrary::RandomIntegerInRange(0,OpenNodeLocations.Num()-1);

		TArray<FVector> NeedsToConnectToNodes;
		TArray<FVector> NeedsToNotConnectNodes;
		for (auto OccupiedNodeLocation : OccupiedNodeLocations)
		{
			bool bHasConnection = false;
			for(auto AdjacentRoom : OccupiedNodeLocation.OpenDoors)
			{
				if (AdjacentRoom.Equals(OpenNodeLocations[RoomPlacement], 0.01))
				{
					bHasConnection = true;
				}
			}
			if (bHasConnection)
			{
				NeedsToConnectToNodes.Add(OccupiedNodeLocation.Coordinates);
			}
			else
			{
				NeedsToNotConnectNodes.Add(OccupiedNodeLocation.Coordinates);
			}
		}
		
		FTransform NewRoomTransform;
		NewRoomTransform.SetLocation(GridToWorld(OpenNodeLocations[RoomPlacement]));
		for (auto RoomType : ShuffledRooms)
		{
			NewRoom = GetWorld()->SpawnActor<APCGRoom>(RoomType, NewRoomTransform);
			if (NewRoom && NewRoom->AmountOfEntrances >= NeedsToConnectToNodes.Num())
			{
				int TotalRoomsAdded = NewRoom->AmountOfEntrances - NeedsToConnectToNodes.Num();
				
				if (NewRoom && TotalRoomsAdded <= AmountOfRoomsLeft-1)
				{
					int AmountOfOpeningsLeft = OpenNodeLocations.Num() - NeedsToConnectToNodes.Num() + TotalRoomsAdded;
					if (AmountOfOpeningsLeft <= AmountOfRoomsLeft-1)
					{
						if (AmountOfOpeningsLeft == 0)
						{
							if (AmountOfRoomsLeft == 1)
							{
								
								int RoomRandomRoomFlip = UKismetMathLibrary::RandomIntegerInRange(0,1);
								for (int u = 0; u < 4; ++u)
								{
									int HitRightConnection = 0;
									bool HitWrongConnection = false;
									//New open rooms
									for (auto Entrance : NewRoom->EntrancesArray)
									{
										FVector GridVector = WorldToGrid(Entrance->GetComponentLocation(), OpenNodeLocations[RoomPlacement]);
										for (auto NeedsToConnectToNode : NeedsToConnectToNodes)
										{
											if (NeedsToConnectToNode.Equals(GridVector, 0.001))
											{
												HitRightConnection++;
											}
										}
								
										for (auto NeedsToNotConnectNode : NeedsToNotConnectNodes)
										{
											if (NeedsToNotConnectNode.Equals(GridVector, 0.001))
											{
												HitWrongConnection = true;
											}	
										}
									}

									if (!HitWrongConnection && HitRightConnection == NeedsToConnectToNodes.Num())
									{
										bRightRoomType = true;
										break;
									}

									//Rotate left
									if (RoomRandomRoomFlip == 1)
									{
										NewRoom->SetActorRotation({NewRoom->GetActorRotation().Pitch,NewRoom->GetActorRotation().Yaw + 90, NewRoom->GetActorRotation().Roll});
									}
									//Rotate Right
									else
									{
										NewRoom->SetActorRotation({NewRoom->GetActorRotation().Pitch,NewRoom->GetActorRotation().Yaw - 90, NewRoom->GetActorRotation().Roll});
									}
								}
							}
						}
						else
						{
							
							int RoomRandomRoomFlip = UKismetMathLibrary::RandomIntegerInRange(0,1);
							for (int u = 0; u < 4; ++u)
							{
								int HitRightConnection = 0;
								bool HitWrongConnection = false;
								//New open rooms
								for (auto Entrance : NewRoom->EntrancesArray)
								{
									FVector GridVector = WorldToGrid(Entrance->GetComponentLocation(), OpenNodeLocations[RoomPlacement]);
									for (auto NeedsToConnectToNode : NeedsToConnectToNodes)
									{
										if (NeedsToConnectToNode.Equals(GridVector, 0.001))
										{
											HitRightConnection++;
										}
									}
								
									for (auto NeedsToNotConnectNode : NeedsToNotConnectNodes)
									{
										if (NeedsToNotConnectNode.Equals(GridVector, 0.001))
										{
											HitWrongConnection = true;
										}	
									}
								}

								if (!HitWrongConnection && HitRightConnection == NeedsToConnectToNodes.Num())
								{
									bRightRoomType = true;
									break;
								}

								//Rotate left
								if (RoomRandomRoomFlip == 1)
								{
									NewRoom->SetActorRotation({NewRoom->GetActorRotation().Pitch,NewRoom->GetActorRotation().Yaw + 90, NewRoom->GetActorRotation().Roll});
								}
								//Rotate Right
								else
								{
									NewRoom->SetActorRotation({NewRoom->GetActorRotation().Pitch,NewRoom->GetActorRotation().Yaw - 90, NewRoom->GetActorRotation().Roll});
								}
							}
						}
					}
				}
			}
				
			if (!bRightRoomType && NewRoom)
			{
				NewRoom->Destroy();
				NewRoom = nullptr;
			}
			
			if (bRightRoomType)
			{
				NewRoom->GridLocation.bOccupied = true;
				NewRoom->GridLocation.Coordinates = {OpenNodeLocations[RoomPlacement]};
				
				//New open rooms
				for (auto OpenRoom : NewRoom->EntrancesArray)
				{
					FVector AdjacentRoomCoordinate = WorldToGrid(OpenRoom->GetComponentLocation(), NewRoom->GridLocation.Coordinates);
					NewRoom->GridLocation.OpenDoors.Add(AdjacentRoomCoordinate);
					bool bFound = false;
				
					for (auto OccupiedNode : OccupiedNodeLocations)
					{
						if (OccupiedNode.Coordinates.Equals(AdjacentRoomCoordinate))
						{
							bFound = true;
						}
					}
					
					if (!bFound)
					{
						UE_LOG(LogTemp, Warning, TEXT("GridVector: %s"), *AdjacentRoomCoordinate.ToString());
						OpenNodeLocations.Add(AdjacentRoomCoordinate);
					}
				}
				
				OpenNodeLocations.Remove(NewRoom->GridLocation.Coordinates);
				
				AmountOfRoomsLeft--;

				//UE_LOG(LogTemp, Warning, TEXT("Room of type %i was chosen, name: %s"), NewRoom->AmountOfEntrances, *NewRoom->GetActorLabel());
				bRightRoomType = true;
				OccupiedNodeLocations.Add(NewRoom->GridLocation);
				PlacedRooms.Add(NewRoom);
				
				UE_LOG(LogTemp, Warning, TEXT("Loop done"));
				break;
			}
		}
	}
	
	if (OpenNodeLocations.Num() != 0 || AmountOfRoomsLeft != 0)
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
	}
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
	
	OccupiedNodeLocations.Empty();
	OpenNodeLocations.Empty();
	PlacedRooms.Empty();

	NewRoom = nullptr;
}
