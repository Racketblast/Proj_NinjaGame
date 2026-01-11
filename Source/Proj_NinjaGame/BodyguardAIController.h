// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MeleeAIController.h"
#include "TargetEnemyExit.h"
#include "BodyguardAIController.generated.h"

enum class EBodyguardState
{
	Following,
	GoingToDeadTarget,
	SearchingDeadTarget,
	GuardingExit
};

UCLASS()
class PROJ_NINJAGAME_API ABodyguardAIController : public AMeleeAIController
{
	GENERATED_BODY()

protected:
	virtual void OnPossess(APawn* InPawn) override;
	
	virtual void Tick(float DeltaSeconds) override;

	virtual void HandleFollowing(float DeltaSeconds) override;

	virtual void HandlePatrolling(float DeltaSeconds) override; 

	virtual void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result) override;

	void HandleDeadTargetBehaviour(float DeltaSeconds);

	EBodyguardState BodyguardState = EBodyguardState::Following;
	
	ATargetEnemyExit* GetClosestExit() const;

	virtual void EndSearch() override;
};
