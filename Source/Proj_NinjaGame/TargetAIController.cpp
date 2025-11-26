// Fill out your copyright notice in the Description page of Project Settings.


#include "TargetAIController.h"

ATargetAIController::ATargetAIController()
{
}

void ATargetAIController::OnHeardSound(FVector SoundLocation)
{
}

void ATargetAIController::RefreshChaseTarget()
{
}

void ATargetAIController::StartChasingFromExternalOrder(FVector LastSpottedPlayerLocation)
{
}

void ATargetAIController::AssignMission(ETargetMission NewMission, FVector MissionLocation)
{
}

void ATargetAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
}

void ATargetAIController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void ATargetAIController::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
	Super::OnMoveCompleted(RequestID, Result);
}

void ATargetAIController::MoveToNextPatrolPoint()
{
}

void ATargetAIController::StartChasing()
{
}

void ATargetAIController::StopChasing()
{
}

void ATargetAIController::StartAlert()
{
}

void ATargetAIController::OnUnPossess()
{
	Super::OnUnPossess();
}

void ATargetAIController::HandleSuspiciousLocation(FVector Location)
{
}

void ATargetAIController::CompleteMission()
{
}

void ATargetAIController::StartMissionMoveTo(FVector Location)
{
}

void ATargetAIController::StartSmoothRotationTowards(const FVector& TargetLocation, float RotationSpeed)
{
}

void ATargetAIController::RunChaseFailsafe(float DeltaSeconds)
{
}

void ATargetAIController::ResetSoundFlag()
{
}

void ATargetAIController::OnAlertTimerExpired()
{
}

void ATargetAIController::RetryMoveToNextPatrolPoint()
{
}

void ATargetAIController::BeginSearch()
{
}

void ATargetAIController::LookAround()
{
}

void ATargetAIController::EndSearch()
{
}

void ATargetAIController::OnTargetLost()
{
}
