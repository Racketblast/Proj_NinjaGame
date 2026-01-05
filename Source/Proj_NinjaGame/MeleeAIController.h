// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "EnemyAIController.h"
#include "MeleeAIController.generated.h"

UCLASS()
class PROJ_NINJAGAME_API AMeleeAIController : public AEnemyAIController
{
	GENERATED_BODY()

public:
	AMeleeAIController();

	virtual void RefreshChaseTarget() override;

	//virtual void StartChasingFromExternalOrder(FVector LastSpottedPlayerLocation) override;

	virtual void StartBackOff(FVector BackLocation) override;

	void StopBackOff();
	
protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void Tick(float DeltaSeconds) override;

	virtual void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result) override;

	int32 CurrentPatrolIndex = 0;
	//EEnemyState CurrentState = EEnemyState::Patrolling;
	
	virtual void StartChasing() override;
	virtual void StopChasing() override;
	virtual void OnUnPossess() override;
	virtual void HandleChasing(float DeltaSeconds) override;
	
	bool CannotReachPlayer(APawn* Player);

	void MoveCloserToPlayer(APawn* Player);

	bool bBackingOff = false;
	float BackOffDuration = 5.0f;
	FVector BackOffLocation;

private:
	FTimerHandle StartPatrolTimerHandle;
	FTimerHandle BackOffTimerHandle;
	FVector LastKnownPlayerLocation;
	//bool bChasingFromExternalOrder = false;
	float RangedThrowCooldown = 0;
};
