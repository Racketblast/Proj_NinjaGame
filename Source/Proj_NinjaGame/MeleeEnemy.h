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
	FORCEINLINE float GetCanAttack() const { return bCanAttack; }

	/*bool bCanAttack = true;

	bool bIsAttacking = false;*/
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

	void EnemyThrow();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Throwable")
	TSubclassOf<class AAIThrowableObject> ThrowableObject;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Throwable")
	float ThrowVelocity = 1000.f;
	float ThrowCooldown = 0.f;
	
};
