// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractableObject.h"
#include "PickupWeaponObject.generated.h"

class AThrowableWeapon;
/**
 * 
 */
UCLASS()
class PROJ_NINJAGAME_API APickupWeaponObject : public AInteractableObject
{
	GENERATED_BODY()
public:	
	// Sets default values for this actor's properties
	APickupWeaponObject();

	virtual void Use_Implementation(class AStealthCharacter* Player) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AThrowableWeapon> ThrowableWeapon;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
};
