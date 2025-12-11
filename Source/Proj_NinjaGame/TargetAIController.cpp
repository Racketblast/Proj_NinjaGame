// Fill out your copyright notice in the Description page of Project Settings.


#include "TargetAIController.h"

#include "Enemy.h"
#include "NavigationPath.h"
#include "NavigationSystem.h"
#include "TargetEnemy.h"
#include "TargetEnemyExit.h"
#include "GameFramework/CharacterMovementComponent.h"

ATargetAIController::ATargetAIController()
{
}

void ATargetAIController::BeginPlay()
{
	Super::BeginPlay();


	if (ATargetEnemy* Enemy = Cast<ATargetEnemy>(ControlledEnemy))
	{
		TargetEnemy = Enemy;
	}
}

void ATargetAIController::HandleChasing(float DeltaSeconds)
{
	
}

void ATargetAIController::HandleSearching(float DeltaSeconds)
{
	if (!bIsRunningAway)
		Super::HandleSearching(DeltaSeconds);
}

void ATargetAIController::HandleAlert(float DeltaSeconds)
{
	if (!bIsRunningAway)
		Super::HandleAlert(DeltaSeconds);
}

void ATargetAIController::HandlePatrolling(float DeltaSeconds)
{
	if (!bIsRunningAway)
		Super::HandlePatrolling(DeltaSeconds);
}

void ATargetAIController::StartChasing()
{
	
	GetWorldTimerManager().ClearTimer(LoseSightTimerHandle);
	GetWorldTimerManager().ClearTimer(LookAroundTimerHandle);
	GetWorldTimerManager().ClearTimer(EndSearchTimerHandle);
	GetWorldTimerManager().ClearTimer(AlertTimerHandle);
	CurrentState = EEnemyState::Chasing;
	bIsRunningAway = true;

	if (ControlledEnemy)
	{
		ControlledEnemy->UpdateStateVFX(CurrentState); // För VFX
		ControlledEnemy->GetCharacterMovement()->MaxWalkSpeed = ControlledEnemy->GetRunSpeed(); 
	}
	
	if (TargetEnemy)
	{
		MoveToLocation(GetClosetExit());

		TArray<AActor*> OverlapActors;
		ControlledEnemy->GetOverlappingActors(OverlapActors);
		for (auto OverlapActor : OverlapActors)
		{
			for (auto ExitActor : TargetEnemy->RunTowardsExits)
			{
				if (ExitActor == Cast<ATargetEnemyExit>(OverlapActor))
				{
					ExitActor->PlayerLoses();
				}
			}
		}
	}
}

void ATargetAIController::StopChasing()
{
}

FVector ATargetAIController::GetClosetExit()
{
	UWorld* World = GetWorld();
	if (!World) return {};

	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(World);
	if (!NavSys) return {};

	// Projektera målpunkten till navmesh
	FNavLocation ProjectedLocation;
	if (!NavSys->ProjectPointToNavigation(
			ControlledEnemy->GetActorLocation(),
			ProjectedLocation,
			FVector(200.f, 200.f, 500.f) 
		))
	{
		return {};
	}

	FVector BestLocation = {};
	float BestDistance = TNumericLimits<float>::Max();

	if (!TargetEnemy)
		return{};
	for (auto Exit : TargetEnemy->RunTowardsExits)
	{
		if (!Exit)
			continue;
		

		UNavigationPath* NavPath = NavSys->FindPathToLocationSynchronously(
			World,
			ProjectedLocation.Location,
			Exit->GetActorLocation()
		);

		if (NavPath && NavPath->IsValid() && NavPath->PathPoints.Num() > 1)
		{
			float PathLength = 0.f;

			for (int32 i = 1; i < NavPath->PathPoints.Num(); i++)
			{
				PathLength += FVector::Dist(NavPath->PathPoints[i - 1], NavPath->PathPoints[i]);
			}

			if (PathLength < BestDistance)
			{
				BestDistance = PathLength;
				BestLocation = Exit->GetActorLocation();
			}
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("BestLocation: %s"), *BestLocation.ToString());
	return BestLocation;
}
