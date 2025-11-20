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

	UPROPERTY(VisibleAnywhere, Category="Collision")
	class USphereComponent* HitCollision;

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
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera")
	float MaxHealth = 1.f;

	float CurrentHealth = MaxHealth;

	bool bIsCameraDisabled = false;

	void CheckPlayerVisibility(float DeltaTime);
	void OnPlayerSpotted();

	virtual float TakeDamage(
		float DamageAmount,
		struct FDamageEvent const& DamageEvent,
		AController* EventInstigator,
		AActor* DamageCauser
	) override;

	void Die();


public:	
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	void DisableCamera();

};
