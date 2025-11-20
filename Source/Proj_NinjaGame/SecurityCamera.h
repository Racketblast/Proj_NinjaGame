// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SecurityCamera.generated.h"

UCLASS()
class PROJ_NINJAGAME_API ASecurityCamera : public AActor
{
	GENERATED_BODY()
	
public:	
	ASecurityCamera();

protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(VisibleAnywhere)
	USkeletalMeshComponent* CameraMesh;

	UPROPERTY(EditAnywhere, Category="Camera")
	UAnimationAsset* IdlePanAnimation;

	UPROPERTY(EditAnywhere, Category="Vision")
	float VisionRange = 1200.f;

	UPROPERTY(EditAnywhere, Category="Vision")
	float VisionAngle = 60.f;

	UPROPERTY(EditAnywhere, Category="Vision")
	float TimeToSpotPlayer = 1.5f;

	bool bPlayerInCone = false;
	bool bHasSpottedPlayer = false;

	float SpotTimer = 0.f;
	
	bool bIsAnimationPlaying = true;

	UPROPERTY(VisibleAnywhere)
	FVector LastSpottedPlayerLocation;
	
	UPROPERTY(EditAnywhere, Category="Vision")
	bool bVisionDebug = true;
	
	APawn* PlayerPawn;

	void CheckPlayerVisibility(float DeltaTime);
	void OnPlayerSpotted();

public:	
	virtual void Tick(float DeltaTime) override;

};
