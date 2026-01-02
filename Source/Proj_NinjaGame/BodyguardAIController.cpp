// Fill out your copyright notice in the Description page of Project Settings.


#include "BodyguardAIController.h"
#include "BodyguardEnemy.h"
#include "GameFramework/CharacterMovementComponent.h"


void ABodyguardAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	ControlledEnemy = Cast<ABodyguardEnemy>(InPawn);

	//UE_LOG(LogTemp, Warning, TEXT("ABodyguardAIController OnPossess"));

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

	if (ControlledEnemy->bPlayerInAlertCone) 
	{
		CurrentState = EEnemyState::Alert;
		StartAlert();
		//UE_LOG(LogTemp, Warning, TEXT("StartAlert"));
	}
	else if (ControlledEnemy->CanSeePlayer())
	{
		StartChasing();
	}
	else if (ControlledEnemy->bHeardSoundRecently && !bIsInvestigatingSound)
	{
		UE_LOG(LogTemp, Warning, TEXT("HandleFollowing: Heard sound at %s"), *ControlledEnemy->LastHeardSoundLocation.ToString());
		OnHeardSound(ControlledEnemy->LastHeardSoundLocation);
		bIsInvestigatingSound = true;
				
		GetWorldTimerManager().SetTimer(ResetSoundFlagHandle, this, &ABodyguardAIController::ResetSoundFlag, 0.5f, false);
	}
	
	ControlledEnemy->UpdateStateVFX(CurrentState); // För VFX

	const FVector TargetLoc = Guard->ProtectedTarget->GetActorLocation();
	const FRotator TargetRot = Guard->ProtectedTarget->GetActorRotation();
	const FVector WorldOffset = TargetRot.RotateVector(Guard->FollowOffset);
	const FVector DesiredFollowPos = TargetLoc + WorldOffset;

	const float Dist = FVector::Dist(
		Guard->GetActorLocation(),
		DesiredFollowPos
	);

	// Catch up logik
	const bool bShouldSprint = Dist > Guard->SprintCatchUpDistance;
	if (bShouldSprint)
	{
		ControlledEnemy->GetCharacterMovement()->MaxWalkSpeed = ControlledEnemy->GetRunSpeed(); 
	}
	else
	{
		ControlledEnemy->GetCharacterMovement()->MaxWalkSpeed = ControlledEnemy->GetWalkSpeed();
	}

	if (Dist > Guard->FollowDistance)
	{
		MoveToLocation(DesiredFollowPos,Guard->FollowDistance * 0.4f,true,true,false);
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

void ABodyguardAIController::HandlePatrolling(float DeltaSeconds)
{
	CurrentState = EEnemyState::Following;
} 