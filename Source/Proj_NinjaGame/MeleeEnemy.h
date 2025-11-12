// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "MeleeAIController.h"
#include "MeleeEnemy.generated.h"

class UBoxComponent;
class UAnimMontage;

UCLASS()
class PROJ_NINJAGAME_API AMeleeEnemy : public ACharacter
{
	GENERATED_BODY()

public:
	AMeleeEnemy();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	TArray<AActor*> PatrolPoints;

	UPROPERTY(EditAnywhere, Category = "AI|Vision")
	float VisionRange = 1500.f;

	UPROPERTY(EditAnywhere, Category = "AI|Vision")
	float VisionAngle = 45.f;

	UPROPERTY(EditAnywhere, Category = "AI|Vision")
	float LoseSightTime = 2.f;
	
	UPROPERTY(EditAnywhere, Category = "AI|Patrol")
	float WaitTimeAtPoint = 2.0f;

	UPROPERTY(EditAnywhere, Category = "AI|Patrol")
	float SearchTime = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float WalkSpeed = 350.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float RunSpeed = 500.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	float Health = 5.f;
	
	float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser);

	void Die();

	UPROPERTY()
	APawn* PlayerPawn;

	bool bCanSeePlayer = false;

	void CheckPlayerVisibility();

	
	// För Attack 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat")
	float AttackRange = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat")
	float AttackDamage = 1.f;

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

public:
	FORCEINLINE const TArray<AActor*>& GetPatrolPoints() const { return PatrolPoints; }
	FORCEINLINE bool CanSeePlayer() const { return bCanSeePlayer; }
	FORCEINLINE float GetLoseSightTime() const { return LoseSightTime; }
	FORCEINLINE float GetWaitTimeAtPoint() const { return WaitTimeAtPoint; }
	FORCEINLINE float GetSearchTime() const { return SearchTime; }
	FORCEINLINE float GetWalkSpeed() const { return WalkSpeed; }
	FORCEINLINE float GetRunSpeed() const { return RunSpeed; }

	void UpdateLastSeenPlayerLocation();
	FVector GetLastSeenPlayerLocation() const { return LastSeenPlayerLocation; }

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	bool bIsChasing = false;

	
	UFUNCTION(BlueprintCallable)
	void StartAttack();
	
	FORCEINLINE float GetAttackRange() const { return AttackRange; }
	
	void EnableHitbox(float WindowSeconds = 0.15f);
	void DisableHitbox();
	
	virtual void ApplyDamageTo(AActor* Target);


	/* Hearing system */
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI|Perception")
	float HearingRange = 800.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI|Perception")
	float HearingMemoryTime = 5.f;
	
	FVector LastHeardSoundLocation;
	
	bool bHeardSoundRecently = false;

	// Anropas när ett ljud hörs
	void HearSoundAtLocation(FVector SoundLocation);


	// VFX
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="VFX")
	UNiagaraComponent* StateVFXComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="VFX")
	UNiagaraSystem* ChaseVFX;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="VFX")
	UNiagaraSystem* SearchVFX;

	UFUNCTION(BlueprintCallable, Category="VFX")
	void UpdateStateVFX(EEnemyState NewState);
	

private:
	FVector LastSeenPlayerLocation;

};
