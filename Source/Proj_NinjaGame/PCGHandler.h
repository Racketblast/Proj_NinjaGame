// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PCGHandler.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FNodeInfo
{
	GENERATED_BODY()
	UPROPERTY()
	class UArrowComponent* Arrow = nullptr;
	UPROPERTY()
	class APCGRoom* Room = nullptr;
};

UCLASS()
class PROJ_NINJAGAME_API APCGHandler : public AActor
{
	GENERATED_BODY()
	
public:
	APCGHandler();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCGInfo")
	TArray<TSubclassOf<APCGRoom>> PossibleRooms;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCGInfo")
	int AmountOfRooms = 0;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "PCGInfo")
	TArray<UArrowComponent*> ExistingEntrances;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "PCGInfo")
	TArray<FNodeInfo> CurrentOpenEntrances;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "PCGInfo")
	TArray< APCGRoom*> PlacedRooms;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "PCGInfo")
	TArray<struct FPCGNodes> OccupiedNodes;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
private:
	void RotateRoom(APCGRoom* Room, int RoomPlacement);
	bool RotateRoomForSeveral(APCGRoom* Room, TArray<UArrowComponent*> OtherRooms);
	TArray<UArrowComponent*> CheckForOtherRooms(APCGRoom* Room);
	UPROPERTY()
	APCGRoom* NewRoom;
	void PCGRoomPlacement();
	
	void EmptyVariables();
	bool bCanRestart = true;
	int Restarts = 0;
	int MaxRestarts = 5;
};

