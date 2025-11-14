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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	bool bCanBeAssassinated = false;
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
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

	void CheckImmediateProximityDetection();

	void CheckChaseProximityDetection();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vision")
	bool bVisionDebug = true;

	// Andra syn sättet för fienden / andra konen
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vision")
	float SuspiciousVisionRange = 3000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vision")
	float SuspiciousVisionAngle = 90.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vision")
	float TimeToSpotPlayer = 6.f; // sekunder för att upptäcka om spelaren stannar kvar

	float SuspiciousTimer = 0.f;
	bool bIsSuspicious = false;
	bool bPlayerInSuspiciousZone = false;


	void OnSuspiciousLocationDetected();

	
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

	UPROPERTY(VisibleAnywhere, Category="Combat")
	UCapsuleComponent* AssassinationCapsule;
	
	UFUNCTION()
	void OnAssasinationOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
							 UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
							 bool bFromSweep, const FHitResult& SweepResult);
	
	UFUNCTION()
	void OnAssasinationOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
								UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
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
	void ForgetHeardSound();

	// Audio:
	UPROPERTY(EditAnywhere, Category="Audio")
	USoundBase* SearchingSound;

	UPROPERTY(EditAnywhere, Category="Audio")
	USoundBase* AlertSound;

	UPROPERTY(EditAnywhere, Category="Audio")
	USoundBase* ChasingSound;

public:
	FORCEINLINE const TArray<AActor*>& GetPatrolPoints() const { return PatrolPoints; }
	FORCEINLINE bool CanSeePlayer() const { return bCanSeePlayer; }
	FORCEINLINE float GetLoseSightTime() const { return LoseSightTime; }
	FORCEINLINE float GetWaitTimeAtPoint() const { return WaitTimeAtPoint; }
	FORCEINLINE float GetSearchTime() const { return SearchTime; }
	FORCEINLINE float GetWalkSpeed() const { return WalkSpeed; }
	FORCEINLINE float GetRunSpeed() const { return RunSpeed; }
	float GetHealth() const { return Health; }

	void UpdateLastSeenPlayerLocation();
	FVector GetLastSeenPlayerLocation() const { return LastSeenPlayerLocation; }

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	bool bIsChasing = false;

	UPROPERTY(BlueprintReadWrite)
	bool bPlayerInAlertCone = false;
	
	UFUNCTION(BlueprintCallable)
	void StartAttack();
	
	FORCEINLINE float GetAttackRange() const { return AttackRange; }
	
	void EnableHitbox(float WindowSeconds = 0.15f);
	void DisableHitbox();
	
	virtual void ApplyDamageTo(AActor* Target);


	
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSuspiciousLocationDelegate, FVector, Location);

	UPROPERTY(BlueprintAssignable, Category = "AI")
	FOnSuspiciousLocationDelegate OnSuspiciousLocation;


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
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="VFX")
	UNiagaraSystem* AlertVFX;

	UFUNCTION(BlueprintCallable, Category="VFX")
	void UpdateStateVFX(EEnemyState NewState);
	

private:
	FVector LastSeenPlayerLocation;

};
