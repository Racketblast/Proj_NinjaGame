// Fill out your copyright notice in the Description page of Project Settings.


#include "BodyguardAIController.h"
#include "BodyguardEnemy.h"



void ABodyguardAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	ControlledEnemy = Cast<ABodyguardEnemy>(InPawn);

	UE_LOG(LogTemp, Warning, TEXT("ABodyguardAIController OnPossess!!!!!!!!!!!!!!!!!!!!!!!!!"));

	if (ControlledEnemy)
	{
		CurrentState = EEnemyState::Following;
	}
}


void ABodyguardAIController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (CurrentState == EEnemyState::Following)
	{
		HandleFollowing(DeltaSeconds);
	}
}


void ABodyguardAIController::HandleFollowing(float DeltaSeconds)
{
	ABodyguardEnemy* Guard = Cast<ABodyguardEnemy>(ControlledEnemy);
	if (!Guard || !Guard->ProtectedTarget) return;

	// Om fienden ser spelaren igen så börjar den jaga direkt
	if (ControlledEnemy->CanSeePlayer())
	{
		GetWorldTimerManager().ClearTimer(LookAroundTimerHandle);
		GetWorldTimerManager().ClearTimer(EndSearchTimerHandle);
		bIsLookingAround = false;
		StartChasing();
		return;  
	}
	
	if (ControlledEnemy->bPlayerInAlertCone) 
	{
		bHasLookAroundTarget = false;
		bIsDoingLookAroundMove = false;

		// Stoppa rotation 
		bIsRotating = false;
		
		StopMovement();
		
		StartAlert();
		//UE_LOG(LogTemp, Error, TEXT("StartAlert"));
		return;
	}

	const FVector TargetLoc = Guard->ProtectedTarget->GetActorLocation();
	const float Dist = FVector::Dist(Guard->GetActorLocation(), TargetLoc);

	if (Dist > Guard->FollowDistance)
	{
		MoveToActor(
			Guard->ProtectedTarget,
			Guard->FollowDistance * 0.8f,   
			true,
			true,
			false
		);
	}
	else
	{
		StopMovement();
	}
}

void ABodyguardAIController::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
	return;
}