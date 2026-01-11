// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractableObject.h"
#include "EnumSpecificKeycard.h"
#include "KeyCard.generated.h"

/**
 * 
 */

UCLASS()
class PROJ_NINJAGAME_API AKeyCard : public AInteractableObject
{
	GENERATED_BODY()
public:	
	// Sets default values for this actor's properties
	AKeyCard();

	virtual void Use_Implementation(class AStealthCharacter* Player) override;

	bool ContainsDoor(class ADoor* Door);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interact")
	SpecificKeycard SpecificKeyCardType = SpecificKeycard::None;
	
protected:
	UPROPERTY(EditAnywhere, Category = "Movement")
	float DistFloat = 10;
	
	FVector StartVector;
	
	UPROPERTY(EditAnywhere, Category = "Movement")
	FVector MoveVelocity = {0,0,5};
	UPROPERTY(EditAnywhere, Category = "Movement")
	FRotator RotationVelocity = {0,90,0};
	
	void MoveObject(float DeltaTime);
	void RotateObject(float DeltaTime);
	
	bool ShouldObjectReturn() const;
	float GetDistanceMoved() const;
	virtual void Tick(float DeltaSeconds) override;
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interact")
	TArray<class ADoor*> DoorsToUnlock;
};
