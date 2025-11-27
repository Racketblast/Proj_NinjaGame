// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlayerUseInterface.h"
#include "GameFramework/Actor.h"
#include "HideSpot.generated.h"

UCLASS()
class PROJ_NINJAGAME_API AHideSpot : public AActor, public IPlayerUseInterface
{
	GENERATED_BODY()
	
public:	
	AHideSpot();

protected:
	virtual void BeginPlay() override;

	void EnterHideSpot(AStealthCharacter* Player);
	void ExitHideSpot();

	UPROPERTY(EditAnywhere, Category = "HideSpot|Camera")
	float MinPitch = -10.0f;

	UPROPERTY(EditAnywhere, Category = "HideSpot|Camera")
	float MaxPitch = 20.0f;

	UPROPERTY(EditAnywhere, Category = "HideSpot|Camera")
	float MinYaw = -30.0f;

	UPROPERTY(EditAnywhere, Category = "HideSpot|Camera")
	float MaxYaw = 30.0f;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USceneComponent* Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* HideMesh;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USceneComponent* ExitPoint;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="HideSpot")
	USceneComponent* CameraPosition;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bOccupied;
	
	UPROPERTY()
	AStealthCharacter* PlayerPawn;
	
	virtual void Use_Implementation(AStealthCharacter* Player) override;
	
	virtual void ShowInteractable_Implementation(bool bShow) override;
	
	UPROPERTY(EditDefaultsOnly, Category="HideSpot")
	FString InteractText;
	UPROPERTY(EditDefaultsOnly, Category="HideSpot")
	FString EnterText = "Hide";
	UPROPERTY(EditDefaultsOnly, Category="HideSpot")
	FString ExitText = "Exit";
};
