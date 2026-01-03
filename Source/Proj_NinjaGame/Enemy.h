// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MeleeAIController.h" // Byt ut till riktiga controller senare 
#include "GameFramework/Character.h"
#include "NiagaraComponent.h"
#include "GeometryCollection/GeometryCollectionObject.h"
#include "Enemy.generated.h"

class UBoxComponent;

UCLASS()
class PROJ_NINJAGAME_API AEnemy : public ACharacter
{
	GENERATED_BODY()

public:
	AEnemy();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	bool bAllowedToAttack = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Health")
	bool bStunned = false;

	UFUNCTION(BlueprintCallable)
	virtual void StartAttack();

	virtual bool IsLocationStillSeeingPlayer(const FVector& TestLoc) const;

	bool DoesHaveHelmet() const { return bHasHelmet; }

	void SetHaveHelmet(bool bHelmet); 

	UFUNCTION()
	void RemoveHelmet();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaTime) override;

	virtual void FaceRotation(FRotator NewRotation, float DeltaTime) override;

	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Equipment")
	bool bHasHelmet = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Enemy|Equipment")
	bool bCanHaveHelmet = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy|Equipment")
	UStaticMeshComponent* HelmetMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Equipment")
	UGeometryCollection* HelmetBreakGeoCollection;
	

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
	
	bool bHasDirectVisualOnPlayer = false; 	// Endast true när fienden faktiskt ser spelaren

	virtual void CheckPlayerVisibility();

	virtual void CheckChaseProximityDetection();

	virtual void CheckCloseDetection();

	virtual bool HasLineOfSightToPlayer();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vision")
	bool bVisionDebug = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float InitialLookAroundDelay = 2.0f;

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
	UCapsuleComponent* HeadCapsule;

	UPROPERTY(EditDefaultsOnly)
	USkeletalMeshComponent* SkeletalMeshComp;

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

public:
	UPROPERTY(EditAnywhere, Category="Audio")
	USoundBase* StunnedSound;
	
	void PlayStateSound(USoundBase* NewSound);
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Audio")
	UAudioComponent* VoiceAudioComponent;
	
	UPROPERTY(EditAnywhere,BlueprintReadOnly, Category="Dialogue")
	UDataTable* EnemyVoiceInfo;
	UPROPERTY(BlueprintReadOnly, Category="Dialogue")
	FName CurrentDialogueRowName;
	UPROPERTY(BlueprintReadWrite, Category="Dialogue")
	FName StartDialogueRowName;
	FName NextDialogueRowName;
	UPROPERTY(BlueprintReadWrite, Category="Dialogue")
	bool bCanPlayDialogue = true;
	FTimerHandle DialogueTimerHandle;
	
public:
	void SetVoiceDataTable(UDataTable* DataTable){EnemyVoiceInfo = DataTable;}
	UDataTable* GetVoiceDataTable() const {return EnemyVoiceInfo;}
	void SetStartDialogueRowName(FName RowName){StartDialogueRowName = RowName;}
	UFUNCTION(BlueprintCallable, Category="Dialogue")
	void StartDialogue();
	UFUNCTION(BlueprintCallable, Category="Dialogue")
	void StopDialogue();

protected:
	UFUNCTION()
	void NextDialogue();
	UFUNCTION(BlueprintPure, Category="Dialogue")
	float GetDialogueDuration();
	
	UPROPERTY(EditAnywhere, Category="Audio")
	USoundBase* HurtSoundOne;

	UPROPERTY(EditAnywhere, Category="Audio")
	USoundBase* HurtSoundTwo;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Audio")
	UAudioComponent* ActionAudioComponent;

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
	FORCEINLINE bool GetVisionDebug() const { return bVisionDebug; }
	FORCEINLINE UCapsuleComponent* GetHeadComponent() const { return HeadCapsule; }
	FORCEINLINE AEnemyHandler* GetEnemyHandler() const { return EnemyHandler; }
	FORCEINLINE void SetEnemyHandler(AEnemyHandler* NewEnemyHandler) { EnemyHandler = NewEnemyHandler; }
	FORCEINLINE virtual float GetAttackRange() const { return AttackRange; } 
	float GetHealth() const { return Health; }
	virtual float GetBackOffMaxDistance() const {return 0;}
	bool CanHaveHelmet() const { return bCanHaveHelmet; }

	void UpdateLastSeenPlayerLocation();
	FVector GetLastSeenPlayerLocation() const { return LastSeenPlayerLocation; }

	float GetInitialLookAroundDelay() const { return InitialLookAroundDelay; }

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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="VFX")
	UNiagaraSystem* StunnedVFX;

	UFUNCTION(BlueprintCallable, Category="VFX")
	void UpdateStateVFX(EEnemyState NewState);

	EEnemyState PreviousState = EEnemyState::Patrolling;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="EEnemyState")
	EEnemyState State = EEnemyState::Patrolling; 	// Används just nu bara för debug
	

	// Throw:
	UFUNCTION(BlueprintCallable)
	virtual void EnemyThrow();
	virtual float GetThrowRange() const;
	virtual float GetThrowCooldown() const;

	UPROPERTY(BlueprintReadWrite, Category="Combat")
	bool bIsThrowing = false;

protected:
	FVector LastSeenPlayerLocation;

};
