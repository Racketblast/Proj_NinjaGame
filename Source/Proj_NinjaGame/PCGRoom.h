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
	int Entrances = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "RoomInfo")
	int EntranceWidth = 100;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RoomInfo")
	int Enemies = 0;

private:
	UPROPERTY(EditDefaultsOnly, Category = "RoomInfo")
	TArray<AActor*> DoorPoints;
};
