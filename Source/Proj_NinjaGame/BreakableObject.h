// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GeometryCollection/GeometryCollectionActor.h"
#include "BreakableObject.generated.h"

/**
 * 
 */
UCLASS()
class PROJ_NINJAGAME_API ABreakableObject : public AGeometryCollectionActor
{
	GENERATED_BODY()
	// Sets default values for this actor's properties
	ABreakableObject();
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Break event callback
	UFUNCTION()
	void OnChaosBreak(const struct FChaosBreakEvent& BreakEvent);

	bool bBroken = false;

	UPROPERTY(EditDefaultsOnly)
	UAudioComponent* AudioComp;
	UPROPERTY(EditDefaultsOnly)
	float ShatterNoiceLevel = 4;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
