// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MeleeAIController.h" // Byt ut till riktiga controller senare 
#include "GameFramework/Character.h"
#include "NiagaraComponent.h"
#include "Enemy.generated.h"

class UBoxComponent;

UCLASS()
class PROJ_NINJAGAME_API AEnemy : public ACharacter
{
	GENERATED_BODY()

public:
	AEnemy();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	bool bCanBeAssassinated = false;

	UFUNCTION(BlueprintCallable)
	virtual void StartAttack();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaTime) override;

	virtual void FaceRotation(FRotator NewRotation, float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	TArray<AActor*> PatrolPoints;

	UPROPERTY(EditAnywhere, Category = "AI|Vision")
	float VisionRange = 1500.f;

	UPROPERTY(VisibleAnywhere, Category = "AI|Vision")
	float OriginalVisionRange;

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

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Health")
	bool bIsDead = false;

	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	virtual void Die();

	UPROPERTY(BlueprintReadWrite)
	class AEnemyHandler* EnemyHandler;
	
	UPROPERTY()
	APawn* PlayerPawn;

	bool bCanSeePlayer = false;

	virtual void CheckPlayerVisibility();

	virtual void CheckChaseProximityDetection();

	virtual void CheckCloseDetection();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vision")
	bool bVisionDebug = true;

	// Andra syn sättet för fienden / andra konen
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vision")
	float SuspiciousVisionRange = 3000.f;
	
	UPROPERTY(VisibleAnywhere, Category = "AI|Vision")
	float OriginalSuspiciousVisionRange;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vision")
	float SuspiciousVisionAngle = 90.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vision")
	float TimeToSpotPlayer = 6.f; // sekunder för att upptäcka om spelaren stannar kvar

	float SuspiciousTimer = 0.f;
	bool bIsSuspicious = false;
	bool bPlayerInSuspiciousZone = false;
	bool bPlayerWithinChaseProximity = false;


	void OnSuspiciousLocationDetected();
	
	// Spread Agro
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agro")
	float AgroSpreadRadius = 800.f;   

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agro")
	bool bUseLineOfSightForAgroSpread = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Agro")
	float AgroSpreadCooldown = 0.5f;

	float LastAgroSpreadTime = -100.f; 

	UFUNCTION(BlueprintCallable, Category = "Agro")
	void SpreadAgroToNearbyEnemies();

	UFUNCTION()
	void OnAgroSpreadTriggered();
	
	// För Attack 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat")
	float AttackDamage = 1.f;

	float AttackRange = 0.f;

	UPROPERTY(EditDefaultsOnly, Category="Combat")
	UCapsuleComponent* AssassinationCapsule;

	UPROPERTY(EditDefaultsOnly, Category="Combat")
	UCapsuleComponent* HeadCapsule;

	UPROPERTY(EditDefaultsOnly)
	USkeletalMeshComponent* SkeletalMeshComp;
	
	UFUNCTION()
	void OnAssasinationOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
							 UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
							 bool bFromSweep, const FHitResult& SweepResult);
	
	UFUNCTION()
	void OnAssasinationOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
								UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// Time handle Funktioner:
	void ForgetHeardSound();

	// Audio:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Audio")
	UAudioComponent* FootstepsAudioComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Audio")
	UAudioComponent* StateAudioComponent;
	
	UPROPERTY(EditAnywhere, Category="Audio")
	USoundBase* SearchingSound;

	UPROPERTY(EditAnywhere, Category="Audio")
	USoundBase* AlertSound;

	UPROPERTY(EditAnywhere, Category="Audio")
	USoundBase* ChasingSound;

	void PlayStateSound(USoundBase* NewSound);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Audio")
	UAudioComponent* VoiceAudioComponent;

	UPROPERTY(EditAnywhere, Category="Audio")
	USoundBase* HurtSoundOne;

	UPROPERTY(EditAnywhere, Category="Audio")
	USoundBase* HurtSoundTwo;

	UFUNCTION()
	void PlayHurtSound();

public:
	FORCEINLINE const TArray<AActor*>& GetPatrolPoints() const { return PatrolPoints; }
	FORCEINLINE bool CanSeePlayer() const { return bCanSeePlayer; }
	FORCEINLINE float GetLoseSightTime() const { return LoseSightTime; }
	FORCEINLINE float GetWaitTimeAtPoint() const { return WaitTimeAtPoint; }
	FORCEINLINE float GetSearchTime() const { return SearchTime; }
	FORCEINLINE float GetWalkSpeed() const { return WalkSpeed; }
	FORCEINLINE float GetRunSpeed() const { return RunSpeed; }
	FORCEINLINE bool GetIsDead() const { return bIsDead; }
	FORCEINLINE UCapsuleComponent* GetHeadComponent() const { return HeadCapsule; }
	FORCEINLINE AEnemyHandler* GetEnemyHandler() const { return EnemyHandler; }
	FORCEINLINE void SetEnemyHandler(AEnemyHandler* NewEnemyHandler) { EnemyHandler = NewEnemyHandler; }
	FORCEINLINE virtual float GetAttackRange() const { return AttackRange; } 
	float GetHealth() const { return Health; }

	void UpdateLastSeenPlayerLocation();
	FVector GetLastSeenPlayerLocation() const { return LastSeenPlayerLocation; }

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	bool bIsChasing = false;

	UPROPERTY(BlueprintReadWrite)
	bool bPlayerInAlertCone = false;
	
	virtual void ApplyDamageTo(AActor* Target);

	void ReduceEnemyRange(bool bShouldReduce);
	void ReduceEnemyHearingRange(bool bShouldReduce);
	
	void SetLastSeenPlayerLocation(FVector NewLocation);
	
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSuspiciousLocationDelegate, FVector, Location);

	UPROPERTY(BlueprintAssignable, Category = "AI")
	FOnSuspiciousLocationDelegate OnSuspiciousLocation;


	UPROPERTY(BlueprintReadWrite, Category="Combat")
	bool bCanAttack = true;
	UPROPERTY(BlueprintReadWrite, Category="Combat")
	bool bIsAttacking = false;

	UPROPERTY()
	bool bRotationLocked = false;
	
	/* Hearing system */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI|Perception")
	float HearingRange = 800.f;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="AI|Perception")
	float OriginalHearingRange;
	
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

	EEnemyState PreviousState = EEnemyState::Patrolling;


	// Throw:
	virtual void EnemyThrow();
	virtual float GetThrowRange() const;
	virtual float GetThrowCooldown() const;

protected:
	FVector LastSeenPlayerLocation;

};
