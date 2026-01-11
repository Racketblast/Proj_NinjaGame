// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractableObject.h"
#include "InteractableAudioObject.generated.h"


UCLASS()
class PROJ_NINJAGAME_API AInteractableAudioObject : public AInteractableObject
{
	GENERATED_BODY()

public:
	AInteractableAudioObject();

	virtual void Use_Implementation(class AStealthCharacter* Player) override;

	virtual void ShowInteractable_Implementation(bool bShow) override;

protected:
	virtual void BeginPlay() override;

	// Audio
	UPROPERTY(EditDefaultsOnly, Category="Audio")
	USoundBase* OneShotSound;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Audio")
	UAudioComponent* AudioComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	float NoiseLevel = 2.0f; 

	// State 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Audio")
	bool bIsPlaying = false;

	UFUNCTION()
	void OnAudioFinished();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Achievement")
	bool bToilet = false;
};
