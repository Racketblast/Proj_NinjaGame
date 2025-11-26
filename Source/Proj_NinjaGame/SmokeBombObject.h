// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ThrowableObject.h"
#include "SmokeBombObject.generated.h"

/**
 * 
 */
UCLASS()
class PROJ_NINJAGAME_API ASmokeBombObject : public AThrowableObject
{
	GENERATED_BODY()
	ASmokeBombObject();
protected:
	virtual void ThrowableOnComponentHitFunction(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);


	virtual void HandlePickup(class AStealthCharacter* Player) override;
	UPROPERTY(EditDefaultsOnly)
	class USphereComponent* SphereComp;
	
	UPROPERTY(EditDefaultsOnly)
	class UNiagaraComponent* SmokeComponent;
	UPROPERTY(EditDefaultsOnly)
	class UNiagaraSystem* SmokeEffect;
};
