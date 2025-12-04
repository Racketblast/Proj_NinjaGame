// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractableObject.h"
#include "PlayerUseInterface.h"
#include "GameFramework/Actor.h"
#include "HideSpot.generated.h"

#define TRACE_CHANNEL_INTERACT ECC_GameTraceChannel3


UCLASS()
class PROJ_NINJAGAME_API AHideSpot : public AInteractableObject
{
	GENERATED_BODY()
	
public:	
	AHideSpot();

protected:
	virtual void BeginPlay() override;

	void EnterHideSpot(AStealthCharacter* Player);
	void ExitHideSpot();

	UPROPERTY(EditAnywhere, Category = "HideSpot|Camera")
	float MinPitch = -20.0f;

	UPROPERTY(EditAnywhere, Category = "HideSpot|Camera")
	float MaxPitch = 40.0f;

	UPROPERTY(EditAnywhere, Category = "HideSpot|Camera")
	float MinYaw = -60.0f;

	UPROPERTY(EditAnywhere, Category = "HideSpot|Camera")
	float MaxYaw = 60.0f;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USceneComponent* HideRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* HideMesh;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USceneComponent* ExitPoint;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="HideSpot")
	USceneComponent* CameraPosition;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="HideSpot")
	class UBoxComponent* ExitBox;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bOccupied;
	
	UPROPERTY()
	AStealthCharacter* PlayerPawn;
	
	virtual void Use_Implementation(AStealthCharacter* Player) override;
	
	UPROPERTY(EditDefaultsOnly, Category="HideSpot")
	FString EnterText = "Hide";
	UPROPERTY(EditDefaultsOnly, Category="HideSpot")
	FString ExitText = "Exit";
};
