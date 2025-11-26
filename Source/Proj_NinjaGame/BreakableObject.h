// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GeometryCollection/GeometryCollectionActor.h"
#include "BreakableObject.generated.h"

/**
 * 
 */
UCLASS()
class PROJ_NINJAGAME_API ABreakableObject : public AActor
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
	UFUNCTION()
	void OnCompHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
	
	bool bBroken = false;

	UPROPERTY(EditDefaultsOnly)
	UAudioComponent* AudioComp;
	UPROPERTY(EditDefaultsOnly)
	float ShatterNoiceLevel = 4;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	USceneComponent* RootSceneComp;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMeshComponent* StaticMeshComponent;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UGeometryCollection* ImpactDebris;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UMaterial* BreakMaterial;
public:	
	UFUNCTION(BlueprintCallable)
	void BreakObject();
	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
