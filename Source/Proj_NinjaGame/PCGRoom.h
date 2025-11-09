// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PCGRoom.generated.h"

/**
 * 
 */
UENUM(BlueprintType)
enum class ERoomType : uint8
{
	None            UMETA(DisplayName = "None"),
	Start          UMETA(DisplayName = "Start"),
	End            UMETA(DisplayName = "End"),
};

USTRUCT(BlueprintType)
struct FPCGNodes
{
	GENERATED_BODY()
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "NodeInfo")
	FVector Coordinates = FVector(0, 0, 0);
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "NodeInfo")
	bool bOccupied = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "NodeInfo")
	TArray<FVector> OpenDoors;
};

UCLASS()
class PROJ_NINJAGAME_API APCGRoom : public AActor
{
	GENERATED_BODY()
	
public:
	APCGRoom();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "RoomInfo")
	ERoomType RoomType = ERoomType::None;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "RoomInfo")
	USceneComponent* RoomRootComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RoomInfo")
	int AmountOfEntrances;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "RoomInfo")
	TArray<class UArrowComponent*> EntrancesArray;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "RoomInfo")
	FPCGNodes GridLocation;

	int RoomWidthInBlocks = 1;
	int RoomHeightInBlocks = 1;
private:
	UPROPERTY(EditDefaultsOnly, Category = "RoomInfo")
	TArray<AActor*> DoorPoints;
	
};
