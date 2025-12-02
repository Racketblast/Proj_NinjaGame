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
	
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaTime) override;

	bool bCanSeePlayer = false;

	virtual void CheckPlayerVisibility() override;
	virtual void CheckChaseProximityDetection() override;
	virtual void CheckCloseDetection() override;

	float SuspiciousTimer = 0.f;
	bool bIsSuspicious = false;
	bool bPlayerInSuspiciousZone = false;
	bool bPlayerWithinChaseProximity = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat")
	float AttackCooldown = 1.2f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat")
	UAnimMontage* AttackMontage;
	
	UPROPERTY(VisibleAnywhere, Category="Combat")
	UBoxComponent* MeleeHitBox;
	
	bool bCanAttack = true;
	bool bHitRegisteredThisSwing = false;

	FTimerHandle AttackCooldownHandle;
	FTimerHandle HitboxWindowHandle;
	
	UFUNCTION()
	void OnMeleeOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
							 UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
							 bool bFromSweep, const FHitResult& SweepResult);
	

	// Time handle Funktioner:
	void ResetAttackCooldown();

private:
	FVector LastSeenPlayerLocation;

};
