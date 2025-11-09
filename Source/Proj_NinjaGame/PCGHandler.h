// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PCGHandler.generated.h"

/**
 * 
 */

UCLASS()
class PROJ_NINJAGAME_API APCGHandler : public AActor
{
	GENERATED_BODY()
	
public:
	APCGHandler();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCGInfo")
	TArray<TSubclassOf<class APCGRoom>> PossibleRooms;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCGInfo")
	int AmountOfRooms = 0;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "PCGInfo")
	TArray< APCGRoom*> PlacedRooms;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "PCGInfo")
	TArray<struct FPCGNodes> OccupiedNodeLocations;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "PCGInfo")
	TArray<FVector> OpenNodeLocations;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
private:
	FVector WorldToGrid(const FVector& EntranceWorldLocation,const FVector& GridLocation) const;
	FVector GridToWorld(const FVector& GridLocation) const;

	void CreateFirstRoom(int &AmountOfRoomsLeft);
	void CreateNextRooms(int &AmountOfRoomsLeft);
	UPROPERTY()
	APCGRoom* NewRoom = nullptr;
	void PCGRoomPlacement();
	
	float CellSize = 1000;
	float CellHeight = 1000;
	
	void EmptyVariables();
	bool bCanRestart = true;
	int Restarts = 0;
	int MaxRestarts = 5;
};

