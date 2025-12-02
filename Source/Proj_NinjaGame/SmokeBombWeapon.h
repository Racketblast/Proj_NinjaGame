// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ThrowableWeapon.h"
#include "SmokeBombWeapon.generated.h"

/**
 * 
 */
UCLASS()
class PROJ_NINJAGAME_API ASmokeBombWeapon : public AThrowableWeapon
{
	GENERATED_BODY()
public:
	// Sets default values for this actor's properties
	ASmokeBombWeapon();

	virtual void Throw(AStealthCharacter* Player) override;
	virtual void Drop(AStealthCharacter* Player) override;
};
