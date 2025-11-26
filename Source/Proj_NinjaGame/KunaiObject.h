// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ThrowableObject.h"
#include "KunaiObject.generated.h"

/**
 * 
 */
UCLASS()
class PROJ_NINJAGAME_API AKunaiObject : public AThrowableObject
{
	GENERATED_BODY()
public:	
	// Sets default values for this actor's properties
	AKunaiObject();

	virtual void HandlePickup(class AStealthCharacter* Player) override;
	
};
