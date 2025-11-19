// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "MeleeAIController.generated.h"

UENUM(BlueprintType)
enum class EEnemyState : uint8
{
	Patrolling,
	Alert, 
	Chasing,
	Searching
};

UCLASS()
class PROJ_NINJAGAME_API AMeleeAIController : public AAIController
{
	GENERATED_BODY()

public:
	AMeleeAIController();

	// Kallas när fienden hör ett ljud
	void OnHeardSound(FVector SoundLocation);

	void RefreshChaseTarget();

	EEnemyState GetCurrentState() const { return CurrentState; }
	
protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void Tick(float DeltaSeconds) override;

	virtual void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result) override;

	UPROPERTY()
	class AMeleeEnemy* ControlledEnemy;

	int32 CurrentPatrolIndex = 0;
	EEnemyState CurrentState = EEnemyState::Patrolling;

	FTimerHandle LoseSightTimerHandle;
	
	void MoveToNextPatrolPoint();
	
	void StartChasing();
	void StopChasing();
	void StartAlert();
	FTimerHandle AlertTimerHandle;

	virtual void OnUnPossess() override;

	UFUNCTION()
	void HandleSuspiciousLocation(FVector Location);

	// Rotation
	FRotator DesiredLookRotation;
	bool bIsRotatingTowardPatrolPoint = false;
	float RotationProgress = 0.f;


	// Failsafe
	FVector LastSearchLocation;
	float TimeWithoutMovement = 0.f;

	UPROPERTY(EditAnywhere, Category="AI")
	float SearchFailSpeedThreshold = 5.f; // Ifall fiendens hastighet är under detta räknas den som stilla

	UPROPERTY(EditAnywhere, Category="AI")
	float SearchFailTime = 5.f; // Hur länge fienden kan vara stilla innan failsafe triggas
	
	// Time handle Funktioner:
	void ResetSoundFlag();
	void OnAlertTimerExpired();
	void RetryMoveToNextPatrolPoint();

private:
	FTimerHandle StartPatrolTimerHandle;
	FTimerHandle LookAroundTimerHandle;
	FTimerHandle EndSearchTimerHandle;
	FTimerHandle ResetSoundFlagHandle;
	FVector LastKnownPlayerLocation;
	bool bIsLookingAround = false;
	bool bIsInvestigatingSound = false;
	bool bIsMovingToSound = false;

	void BeginSearch();
	void LookAround();
	void EndSearch();
	void OnTargetLost();
};
