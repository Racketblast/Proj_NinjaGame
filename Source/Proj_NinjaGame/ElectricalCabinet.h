// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractableObject.h"
#include "ElectricalCabinet.generated.h"

/**
 * 
 */
UCLASS()
class PROJ_NINJAGAME_API AElectricalCabinet : public AInteractableObject
{
	GENERATED_BODY()
public:	
	// Sets default values for this actor's properties
	AElectricalCabinet();

	virtual void Use_Implementation(class AStealthCharacter* Player) override;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	bool bPowerOff = false;

	void TurnPowerOnOff();
};
