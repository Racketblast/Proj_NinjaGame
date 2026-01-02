// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MeleeAIController.h"
#include "BodyguardAIController.generated.h"


UCLASS()
class PROJ_NINJAGAME_API ABodyguardAIController : public AMeleeAIController
{
	GENERATED_BODY()

protected:
	virtual void OnPossess(APawn* InPawn) override;
	
	virtual void Tick(float DeltaSeconds) override;

	virtual void HandleFollowing(float DeltaSeconds) override;

	virtual void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result) override;
};
