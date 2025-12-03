// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ThrowableObject.h"
#include "AIThrowableObject.generated.h"

/**
 * 
 */
UCLASS()
class PROJ_NINJAGAME_API AAIThrowableObject : public AThrowableObject
{
	GENERATED_BODY()
	virtual void ThrowableOnComponentHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;

};
