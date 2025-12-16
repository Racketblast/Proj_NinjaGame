// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractableObject.h"
#include "NiagaraSystem.h"
#include "InteractableVFXObject.generated.h"


UCLASS()
class PROJ_NINJAGAME_API AInteractableVFXObject : public AInteractableObject
{
	GENERATED_BODY()

public:
	AInteractableVFXObject();

	virtual void Use_Implementation(class AStealthCharacter* Player) override;

protected:
	virtual void BeginPlay() override;

	// State 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="VFX")
	bool bIsActive = false;

	// Audio 
	UPROPERTY(EditDefaultsOnly, Category="Audio")
	USoundBase* LoopingSound;

	UPROPERTY(EditDefaultsOnly)
	UAudioComponent* AudioComponent;

	// VFX 
	UPROPERTY(EditDefaultsOnly, Category="VFX")
	UNiagaraSystem* LoopingVFX;

	UPROPERTY(EditDefaultsOnly)
	UNiagaraComponent* VFXComponent;

	void TurnOn();
	void TurnOff();
};
