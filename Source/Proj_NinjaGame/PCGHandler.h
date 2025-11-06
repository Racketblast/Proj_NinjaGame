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
	TArray<class UArrowComponent*> CurrentOpenEntrances;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "PCGInfo")
	TArray< APCGRoom*> PlacedRooms;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
private:
	void RotateRoom(APCGRoom* Room, int RoomPlacement);
	void RotateRoomForSeveral(APCGRoom* Room, TArray<UArrowComponent*> OtherRooms);
	TArray<UArrowComponent*> CheckForOtherRooms(APCGRoom* Room);
	UPROPERTY()
	APCGRoom* NewRoom;
};
