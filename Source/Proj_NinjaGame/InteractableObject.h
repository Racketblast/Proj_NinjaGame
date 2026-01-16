// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlayerUseInterface.h"
#include "GameFramework/Actor.h"
#include "InteractableObject.generated.h"

#define TRACE_CHANNEL_INTERACT ECC_GameTraceChannel3
#define TRACE_CHANNEL_CLIMB ECC_GameTraceChannel4

UCLASS()
class PROJ_NINJAGAME_API AInteractableObject : public AActor, public IPlayerUseInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AInteractableObject();

	virtual void Use_Implementation(class AStealthCharacter* Player) override;
	virtual void ShowInteractable_Implementation(bool bShow) override;
	virtual void UpdateShowInteractable_Implementation() override;
	virtual void TurnOnVFX(bool bCond);

	FORCEINLINE void SetShowVFX(bool bShow){bShouldShowVFX = bShow;}
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	UStaticMeshComponent* StaticMeshComponent;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	UPROPERTY(EditDefaultsOnly, Category="Interact")
	USoundBase* InteractSound;
	
	UPROPERTY(EditDefaultsOnly, Category="Interact")
	USoundBase* HoverSound;
	
	UPROPERTY(EditAnywhere, Category="Interact")
	FString InteractText;
	
	UPROPERTY(EditDefaultsOnly, Category="Interact")
	bool bOverrideInteractText = false;
	
	UPROPERTY(VisibleAnywhere, Category="Interact")
	bool bPlayerLookingAtThis = false;

	UPROPERTY(EditDefaultsOnly)
	class UNiagaraComponent* SparkleComponent;
	
	bool bShouldShowVFX = true;

	virtual void ChangeSparkleBasedOnSize();
};
