// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Door.h"
#include "DoorFacility.generated.h"

UCLASS()
class PROJ_NINJAGAME_API ADoorFacility : public ADoor
{
	GENERATED_BODY()
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Interact")
	FVector ClosedDoorLocation;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interact")
	FVector OpenDoorLocation{0, 0,250};

	FVector DoorTargetLocation;

	virtual void OpenCloseDoor() override;

	void MoveDoor(float DeltaSeconds) override;

	void DoorBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
};
