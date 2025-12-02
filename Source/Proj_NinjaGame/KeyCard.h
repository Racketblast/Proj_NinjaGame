// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractableObject.h"
#include "KeyCard.generated.h"

/**
 * 
 */
UENUM(BlueprintType)
enum class SpecificKeyCard : uint8
{
	None    UMETA(DisplayName = "None"),
	Blue     UMETA(DisplayName = "Blue"),
	Green  UMETA(DisplayName = "Green"),
	Yellow  UMETA(DisplayName = "Yellow"),
	Wrench  UMETA(DisplayName = "Wrench"),
};

UCLASS()
class PROJ_NINJAGAME_API AKeyCard : public AInteractableObject
{
	GENERATED_BODY()
public:	
	// Sets default values for this actor's properties
	AKeyCard();

	virtual void Use_Implementation(class AStealthCharacter* Player) override;

	bool ContainsDoor(class ADoor* Door);

	SpecificKeyCard SpecificKeyCardType = SpecificKeyCard::None;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interact")
	TArray<class ADoor*> DoorsToUnlock;
};
