// Fill out your copyright notice in the Description page of Project Settings.

#include "MeleeAIController.h"
#include "MeleeEnemy.h"
#include "TimerManager.h"
#include "AIController.h"
#include "Engine/World.h"
#include "Navigation/PathFollowingComponent.h"

AMeleeAIController::AMeleeAIController()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AMeleeAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
}

void AMeleeAIController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void AMeleeAIController::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
	if (!ControlledEnemy || !IsValid(ControlledEnemy))
		return;
	if (GetWorld()->bIsTearingDown)
		return;
	
	Super::OnMoveCompleted(RequestID, Result);
}


void AMeleeAIController::StartChasing()
{
	Super::StartChasing();
}

void AMeleeAIController::StopChasing()
{
	Super::StopChasing();
}

void AMeleeAIController::RefreshChaseTarget()
{
	Super::RefreshChaseTarget();
}


// Kallas just nu från kameran
void AMeleeAIController::StartChasingFromExternalOrder(FVector LastSpottedPlayerLocation)
{
	Super::StartChasingFromExternalOrder(LastKnownPlayerLocation);
}


void AMeleeAIController::OnUnPossess()
{
	Super::OnUnPossess();
}
