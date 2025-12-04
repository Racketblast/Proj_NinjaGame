// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enemy.h"
#include "MeleeEnemy.generated.h"

class UBoxComponent;
class UAnimMontage;

UCLASS()
class PROJ_NINJAGAME_API AMeleeEnemy : public AEnemy
{
	GENERATED_BODY()

public:
	AMeleeEnemy();

	virtual void StartAttack() override;
	
	void EnableHitbox(float WindowSeconds = 0.15f);
	void DisableHitbox();

	FORCEINLINE virtual float GetAttackRange() const override { return AttackRange; }
	FORCEINLINE virtual float GetThrowRange() const override { return ThrowRange; }
	FORCEINLINE virtual float GetThrowCooldown() const override { return ThrowCooldown; }
	FORCEINLINE float GetCanAttack() const { return bCanAttack; }

	/*bool bCanAttack = true;

	bool bIsAttacking = false;*/

	// Fiende throw
	virtual void EnemyThrow() override;
	
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaTime) override;

	virtual void CheckPlayerVisibility() override;
	virtual void CheckChaseProximityDetection() override;
	virtual void CheckCloseDetection() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat")
	float AttackCooldown = 1.2f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat")
	float AttackRange = 100.f; 
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat")
	UAnimMontage* AttackMontage;
	
	UPROPERTY(VisibleAnywhere, Category="Combat")
	UBoxComponent* MeleeHitBox;
	
	bool bHitRegisteredThisSwing = false;

	FTimerHandle AttackCooldownHandle;
	FTimerHandle HitboxWindowHandle;
	
	UFUNCTION()
	void OnMeleeOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
							 UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
							 bool bFromSweep, const FHitResult& SweepResult);
	

	// Time handle Funktioner:
	void ResetAttackCooldown();

	// Fiende throw
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Throwable")
	TSubclassOf<class AAIThrowableObject> ThrowableObject;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Throwable")
	float ThrowVelocity = 1000.f;
	float ThrowCooldown = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat")
	USceneComponent* ProjectileSpawnPoint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI")
	float ThrowRange = 2200.f;

	bool IsThrowPathBlocked(const FVector& SpawnLocation, const FVector& TargetLocation, float TestArcHeight);

	FVector PredictPlayerLocation(float ProjectileSpeed) const;
	
};
