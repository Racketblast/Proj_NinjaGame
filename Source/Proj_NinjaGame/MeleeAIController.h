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

UENUM(BlueprintType)
enum class EEnemyMission : uint8
{
	Patrol,
	Camera, 
	Electrical,
	Sprinkler
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

	EEnemyMission GetCurrentMission() const { return CurrentMission; }
	void SetCurrentMission(EEnemyMission NewMission) { CurrentMission = NewMission; }

	void StartChasingFromExternalOrder(FVector LastSpottedPlayerLocation);

	//För missions
	void AssignMission(EEnemyMission NewMission, FVector MissionLocation);

	bool GetHasMission() const { return bHasMission; }

	// För stun
	void StunEnemy(float Duration, TOptional<EEnemyState> WantedState = TOptional<EEnemyState>()); 
	
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

	//Stun
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Stun")
	bool bIsStunned = false;
	
	EEnemyState StateAfterStun = EEnemyState::Patrolling; 

	EEnemyState StateBeforeStun; 
	
	FTimerHandle StunTimerHandle;

	void EndStun();

	// Nya mission system
	UPROPERTY(BlueprintReadOnly)
	EEnemyMission CurrentMission = EEnemyMission::Patrol;

	UPROPERTY(BlueprintReadOnly)
	FVector CurrentMissionLocation;

	UPROPERTY(BlueprintReadOnly)
	bool bHasMission = false;

	bool bIsDoingMissionMoveTo = false;
	
	void CompleteMission();

	UFUNCTION()
	void StartMissionMoveTo(FVector Location);
	
	// Mission failsafe
	float MissionFailSafeTime = 5.f; 
	float MissionFailSafeSpeedThreshold = 5.f; 
	float MissionTimeWithoutMovement = 0.f;
	FVector MissionLastLocation;

	// Rotation
	void StartSmoothRotationTowards(const FVector& TargetLocation, float RotationSpeed);
	bool bIsRotating = false;
	FRotator DesiredRotation;
	float CurrentRotationSpeed = 1.f;
	FVector InvestigateTarget;
	bool bIsInvestigatingTarget = false;
	FVector LookAroundTarget;
	bool bHasLookAroundTarget = false;
	bool bIsDoingLookAroundMove = false;

	
	FRotator DesiredLookRotation;
	bool bIsRotatingTowardPatrolPoint = false;
	float RotationProgress = 0.f;


	// Search Failsafe 
	FVector LastSearchLocation;
	float TimeWithoutMovement = 0.f;

	UPROPERTY(EditAnywhere, Category="AI")
	float SearchFailSpeedThreshold = 5.f; // Ifall fiendens hastighet är under detta räknas den som stilla

	UPROPERTY(EditAnywhere, Category="AI")
	float SearchFailTime = 5.f; // Hur länge fienden kan vara stilla innan failsafe triggas

	
	// Chase Failsafe
	float ChaseFailTime = 5.f;              
	float ChaseFailSpeedThreshold = 5.f;    
	float TimeWithoutMovement_Chase = 0.f;
	FVector LastChaseLocation;
	void RunChaseFailsafe(float DeltaSeconds);
	
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
	bool bChasingFromExternalOrder = false;

	
	int32 LookAroundCount = 0;
	int32 LookAroundMax = 3;

	void BeginSearch();
	void LookAround();
	void EndSearch();
	void OnTargetLost();
};
