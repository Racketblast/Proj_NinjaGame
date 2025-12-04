// Fill out your copyright notice in the Description page of Project Settings.


#include "TargetAIController.h"

#include "Enemy.h"
#include "NavigationSystem.h"
#include "TargetEnemy.h"
#include "GameFramework/CharacterMovementComponent.h"

ATargetAIController::ATargetAIController()
{
}

void ATargetAIController::HandleChasing(float DeltaSeconds)
{
	
}

void ATargetAIController::StartChasing()
{
	
	CurrentState = EEnemyState::Chasing;

	if (ControlledEnemy)
	{
		ControlledEnemy->UpdateStateVFX(CurrentState); // För VFX
		ControlledEnemy->GetCharacterMovement()->MaxWalkSpeed = ControlledEnemy->GetRunSpeed(); 
	}
	
	if (ATargetEnemy* TargetEnemy = Cast<ATargetEnemy>(ControlledEnemy))
	{
		if (TargetEnemy->RunTowardsActors[0])
		{
			FVector GroundedLocation = TargetEnemy->RunTowardsActors[0]->GetActorLocation();

			FHitResult Hit;
			FCollisionQueryParams Params;
			Params.AddIgnoredActor(ControlledEnemy);

			FVector TraceStart = TargetEnemy->RunTowardsActors[0]->GetActorLocation() + FVector(0, 0, 200);
			FVector TraceEnd   = TargetEnemy->RunTowardsActors[0]->GetActorLocation() - FVector(0, 0, 500);

			if (GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, Params))
			{
				GroundedLocation = Hit.ImpactPoint;
			}
			else
			{
				GroundedLocation.Z = ControlledEnemy->GetActorLocation().Z;
			}

			// Project to NavMesh 
			FNavLocation Projected;
			UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());

			if (NavSys && NavSys->ProjectPointToNavigation(
					GroundedLocation,
					Projected,
					FVector(ControlledEnemy->HearingRange, ControlledEnemy->HearingRange, ControlledEnemy->HearingRange)  
				))
			{
				GroundedLocation = Projected.Location;
			}
			
			MoveToLocation(GroundedLocation);
		}
	}
}
