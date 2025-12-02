// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlayerUseInterface.h"
#include "GameFramework/Actor.h"
#include "InteractableObject.generated.h"

#define TRACE_CHANNEL_INTERACT ECC_GameTraceChannel3

UCLASS()
class PROJ_NINJAGAME_API AInteractableObject : public AActor, public IPlayerUseInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AInteractableObject();

	virtual void Use_Implementation(class AStealthCharacter* Player) override;
	virtual void ShowInteractable_Implementation(bool bShow) override;

	UPROPERTY(EditDefaultsOnly)
	UStaticMeshComponent* StaticMeshComponent;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	UPROPERTY(EditDefaultsOnly, Category="Interact")
	USoundBase* InteractSound;
	
	UPROPERTY(EditDefaultsOnly, Category="Interact")
	USoundBase* HoverSound;
	
	UPROPERTY(EditDefaultsOnly, Category="Interact")
	FString InteractText;
	
	UPROPERTY(EditDefaultsOnly, Category="Interact")
	bool bOverrideInteractText = false;
};
