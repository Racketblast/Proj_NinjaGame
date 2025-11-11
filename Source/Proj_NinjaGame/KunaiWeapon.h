// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ThrowableWeapon.h"
#include "KunaiWeapon.generated.h"

/**
 * 
 */
UCLASS()
class PROJ_NINJAGAME_API AKunaiWeapon : public AThrowableWeapon
{
	GENERATED_BODY()
public:
	// Sets default values for this actor's properties
	AKunaiWeapon();

	virtual void Throw(AStealthCharacter* Player) override;
};
