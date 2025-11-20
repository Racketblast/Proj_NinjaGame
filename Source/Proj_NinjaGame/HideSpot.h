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
	
};
