// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NiagaraSystem.h"
#include "Components/SpotLightComponent.h"
#include "GameFramework/Actor.h"
#include "SecurityCamera.generated.h"

UENUM(BlueprintType)
enum class ECameraVFXState : uint8
{
	None,
	Alert,     
	Detected   
};

UCLASS()
class PROJ_NINJAGAME_API ASecurityCamera : public AActor
{
	GENERATED_BODY()
	
public:	
	ASecurityCamera();

protected:
	virtual void BeginPlay() override;

	ECameraVFXState CurrentVFXState = ECameraVFXState::None;

	void SetVFXState(ECameraVFXState NewState);
	
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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bIsCameraDead = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bPlayerInCone = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bHasSpottedPlayer = false;

	float SpotTimer = 0.f;
	
	bool bIsAnimationPlaying = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector LastSpottedPlayerLocation;
	
	UPROPERTY(EditAnywhere, Category="Vision")
	bool bVisionDebug = true;
	
	UPROPERTY()
	APawn* PlayerPawn;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera")
	float MaxHealth = 1.f;

	float CurrentHealth = MaxHealth;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
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

	UPROPERTY(BlueprintReadWrite)
	class AEnemyHandler* EnemyHandler;

	// VFX
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="VFX")
	UNiagaraComponent* StateVFXComponent;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="VFX")
	UNiagaraSystem* AlertVFX;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="VFX")
	UNiagaraSystem* DetectedVFX;

	// Audio
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Audio")
	UAudioComponent* StateAudioComponent;

	UPROPERTY(EditAnywhere, Category="Audio")
	USoundBase* AlertSound;

	UPROPERTY(EditAnywhere, Category="Audio")
	USoundBase* DetectedSound;

	//Spotlight
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Vision", meta=(AllowPrivateAccess="true"))
	USpotLightComponent* VisionSpotlight;

	FLinearColor HexToLinearColor(const FString& Hex);

	void SetSpotlightColorFromHex(const FString& HexColor);


public:	
	virtual void Tick(float DeltaTime) override;
	
	FORCEINLINE AEnemyHandler* GetEnemyHandler() const { return EnemyHandler; }
	FORCEINLINE void SetEnemyHandler(AEnemyHandler* NewEnemyHandler) { EnemyHandler = NewEnemyHandler; }
	FORCEINLINE bool GetIsDead() const { return bIsCameraDead; }

	UFUNCTION(BlueprintCallable)
	void DisableCamera();

	UFUNCTION(BlueprintCallable)
	void ActivateCamera();

};
