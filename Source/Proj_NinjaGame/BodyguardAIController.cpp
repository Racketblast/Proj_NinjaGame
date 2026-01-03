// Fill out your copyright notice in the Description page of Project Settings.


#include "BodyguardAIController.h"
#include "BodyguardEnemy.h"
#include "NavigationPath.h"
#include "NavigationSystem.h"
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

	// Target är död
	if (Guard->bProtectedTargetIsDead)
	{
		HandleDeadTargetBehaviour(DeltaSeconds);
		return;
	}

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

	const FVector TargetVelocity = Guard->ProtectedTarget->GetVelocity();
	const bool bTargetIsMoving = TargetVelocity.Size() > 5.f;

	if (!bTargetIsMoving)
	{
		// Guards står still om Target står still
		StopMovement();

		return;
	}

	const FVector TargetLoc = Guard->ProtectedTarget->GetActorLocation();
	const FRotator TargetRot = Guard->ProtectedTarget->GetActorRotation();
	const FVector WorldOffset = TargetRot.RotateVector(Guard->FollowOffset);
	const FVector DesiredFollowPos = TargetLoc + WorldOffset;

	const float Dist = FVector::Dist(Guard->GetActorLocation(), DesiredFollowPos);

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


void ABodyguardAIController::HandleDeadTargetBehaviour(float DeltaSeconds)
{
	ABodyguardEnemy* Guard = Cast<ABodyguardEnemy>(ControlledEnemy);
	if (!Guard) return;

	switch (BodyguardState)
	{
	case EBodyguardState::Following:
		{
			BodyguardState = EBodyguardState::GoingToDeadTarget;
			MoveToLocation(Guard->LastKnownTargetDeathLocation);
			break;
		}

	case EBodyguardState::GoingToDeadTarget:
		{
			const float Dist = FVector::Dist(
				Guard->GetActorLocation(),
				Guard->LastKnownTargetDeathLocation
			);

			if (Dist < 120.f)
			{
				StopMovement();
				BodyguardState = EBodyguardState::SearchingDeadTarget;
				BeginSearch();
			}
			break;
		}

	case EBodyguardState::SearchingDeadTarget:
		{
			// Vänta tills EndSearch() kallas
			break;
		}

	case EBodyguardState::GuardingExit:
		{
			// Stå still / Gå till exit
			break;
		}
	}
}


void ABodyguardAIController::EndSearch()
{
	Super::EndSearch();
	
	//UE_LOG(LogTemp, Warning, TEXT("ABodyguardAIController::EndSearch()"));

	if (BodyguardState == EBodyguardState::SearchingDeadTarget)
	{
		if (ATargetEnemyExit* Exit = GetClosestExit())
		{
			ABodyguardEnemy* Guard = Cast<ABodyguardEnemy>(ControlledEnemy);
			if (Guard)
			{
				Guard->AssignedExit = Exit;
			}

			BodyguardState = EBodyguardState::GuardingExit;
			MoveToLocation(Exit->GetActorLocation(), 120.f);
		}
	}
}


ATargetEnemyExit* ABodyguardAIController::GetClosestExit() const
{
	UWorld* World = GetWorld();
	if (!World || !ControlledEnemy)
		return nullptr;

	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(World);
	if (!NavSys)
		return nullptr;

	ABodyguardEnemy* Guard = Cast<ABodyguardEnemy>(ControlledEnemy);
	if (!Guard || !Guard->ProtectedTarget)
		return nullptr;

	ATargetEnemy* TargetEnemy = Guard->ProtectedTarget;
	if (!TargetEnemy)
		return nullptr;

	// Projektera bodyguardens position till navmesh
	FNavLocation ProjectedLocation;
	if (!NavSys->ProjectPointToNavigation(
			ControlledEnemy->GetActorLocation(),
			ProjectedLocation,
			FVector(200.f, 200.f, 500.f)))
	{
		return nullptr;
	}

	float BestDistance = TNumericLimits<float>::Max();
	ATargetEnemyExit* BestExit = nullptr;

	for (ATargetEnemyExit* Exit : TargetEnemy->RunTowardsExits)
	{
		if (!Exit)
			continue;

		UNavigationPath* NavPath = NavSys->FindPathToLocationSynchronously(
			World,
			ProjectedLocation.Location,
			Exit->GetActorLocation()
		);

		if (!NavPath || !NavPath->IsValid() || NavPath->PathPoints.Num() < 2)
			continue;

		float PathLength = 0.f;
		for (int32 i = 1; i < NavPath->PathPoints.Num(); i++)
		{
			PathLength += FVector::Dist(
				NavPath->PathPoints[i - 1],
				NavPath->PathPoints[i]
			);
		}

		if (PathLength < BestDistance)
		{
			BestDistance = PathLength;
			BestExit = Exit;
		}
	}

	if (BestExit)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("Bodyguard %s chose exit %s (PathDist: %.1f)"),
			*GetNameSafe(ControlledEnemy),
			*GetNameSafe(BestExit),
			BestDistance
		);
	}

	if (!BestExit)
	{
		UE_LOG(LogTemp, Warning, TEXT("No TargetEnemyExit found in level!"));
	}

	return BestExit;
}
