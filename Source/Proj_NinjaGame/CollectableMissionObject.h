// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractableObject.h"
#include "CollectableMissionObject.generated.h"

/**
 * 
 */
UCLASS()
class PROJ_NINJAGAME_API ACollectableMissionObject : public AInteractableObject
{
	GENERATED_BODY()
public:	
	// Sets default values for this actor's properties
	ACollectableMissionObject();

	virtual void Use_Implementation(class AStealthCharacter* Player) override;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	UPROPERTY(EditAnywhere, Category = "Movement")
	float DistFloat = 10;
	
	FVector StartVector;
	
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	FVector MoveVelocity = {0,0,5};
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	FRotator RotationVelocity = {0,90,0};
	
	void MoveObject(float DeltaTime);
	void RotateObject(float DeltaTime);
	
	bool ShouldObjectReturn() const;
	float GetDistanceMoved() const;
public:
	virtual void Tick(float DeltaTime) override;
	
	UFUNCTION(BlueprintCallable)
	FORCEINLINE class AMissionHandler* GetMissionHandler() const { return MissionHandler; }
	UFUNCTION(BlueprintCallable)
	FORCEINLINE void SetMissionHandler(AMissionHandler* NewMissionHandler) { MissionHandler = NewMissionHandler; }
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	AMissionHandler* MissionHandler;
};
