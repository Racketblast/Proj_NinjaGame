// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyHandler.h"
#include "MeleeEnemy.h"
#include "Kismet/GameplayStatics.h"
#include "MeleeAIController.h"
#include "NavigationPath.h"
#include "NavigationSystem.h"
#include "SecurityCamera.h"


AEnemyHandler::AEnemyHandler()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AEnemyHandler::BeginPlay()
{
	Super::BeginPlay();

	// Hitta alla AMeleeEnemy i leveln
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMeleeEnemy::StaticClass(), AllEnemies);
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASecurityCamera::StaticClass(), AllSecurityCameras);

	for (auto Enemy : AllEnemies)
	{
		if (AMeleeEnemy* MeleeEnemy = Cast<AMeleeEnemy>(Enemy))
		{
			MeleeEnemy->SetEnemyHandler(this);
		}
	}
	
	for (auto Camera : AllSecurityCameras)
	{
		if (ASecurityCamera* SecurityCamera = Cast<ASecurityCamera>(Camera))
		{
			SecurityCamera->SetEnemyHandler(this);
		}
	}
	
	UE_LOG(LogTemp, Warning, TEXT("EnemyHandler found %d enemies"), AllEnemies.Num());
}

void AEnemyHandler::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdateEnemyStates();
}

void AEnemyHandler::RemoveEnemy(AActor* EnemyRemoved)
{
	if (AllEnemies.Contains(EnemyRemoved))
	{
		AllEnemies.Remove(EnemyRemoved);
	}
}

void AEnemyHandler::RemoveCamera(AActor* CameraRemoved)
{
	if (AllSecurityCameras.Contains(CameraRemoved))
	{
		AllSecurityCameras.Remove(CameraRemoved);
	}
}

void AEnemyHandler::UpdateEnemyStates()
{
	bool bAnyChasing = false;

	// Går igenom alla fiender för att se om någon ut av dem är i Chasing State
	for (AActor* EnemyActor : AllEnemies)
	{
		AMeleeEnemy* Enemy = Cast<AMeleeEnemy>(EnemyActor);
		if (!Enemy) continue;

		AMeleeAIController* AICon = Cast<AMeleeAIController>(Enemy->GetController());
		if (!AICon) continue;

		if (AICon->GetCurrentState() == EEnemyState::Chasing)
		{
			bAnyChasing = true;
			break;
		}
	}

	//Debug
	if (bEnemySeesPlayer != bAnyChasing)
	{
		bEnemySeesPlayer = bAnyChasing;

		if (bEnemySeesPlayer)
		{
			UE_LOG(LogTemp, Warning, TEXT("EnemyHandler: A enemy sees player: TRUE"));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("EnemyHandler: A enemy sees player: FALSE"));
		}
	}

	bEnemySeesPlayer = bAnyChasing;
}


AMeleeEnemy* AEnemyHandler::GetClosestEnemyToLocation(FVector TargetLocation)
{
	UWorld* World = GetWorld();
	if (!World) return nullptr;

	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(World);
	if (!NavSys) return nullptr;

	// Projektera målpunkten till navmesh
	FNavLocation ProjectedLocation;
	if (!NavSys->ProjectPointToNavigation(
			TargetLocation,
			ProjectedLocation,
			FVector(200.f, 200.f, 500.f) 
		))
	{
		return nullptr;
	}

	TargetLocation = ProjectedLocation.Location;

	AMeleeEnemy* BestEnemy = nullptr;
	float BestDistance = TNumericLimits<float>::Max();

	for (AActor* EnemyActor : AllEnemies)
	{
		AMeleeEnemy* Enemy = Cast<AMeleeEnemy>(EnemyActor);
		if (!Enemy) continue;

		UNavigationPath* NavPath = NavSys->FindPathToLocationSynchronously(
			World,
			Enemy->GetActorLocation(),
			TargetLocation
		);

		float PathLength = 0.f;

		if (NavPath && NavPath->IsValid() && NavPath->PathPoints.Num() > 1)
		{
			// Räkna ut total path längd
			for (int32 i = 1; i < NavPath->PathPoints.Num(); i++)
			{
				PathLength += FVector::Dist(NavPath->PathPoints[i - 1], NavPath->PathPoints[i]);
			}

			if (PathLength < BestDistance)
			{
				BestDistance = PathLength;
				BestEnemy = Enemy;
			}
		}
	}

	return BestEnemy;
}



TArray<AMeleeEnemy*> AEnemyHandler::GetTwoClosestEnemies(FVector TargetLocation)
{
	struct FEnemyPathData
	{
		AMeleeEnemy* Enemy;
		float PathDistance;
	};

	TArray<FEnemyPathData> Distances;

	UWorld* World = GetWorld();
	if (!World) return {};

	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(World);
	if (!NavSys) return {};

	for (AActor* Actor : AllEnemies)
	{
		AMeleeEnemy* Enemy = Cast<AMeleeEnemy>(Actor);
		if (!Enemy) continue;

		UNavigationPath* NavPath = NavSys->FindPathToLocationSynchronously(
			World,
			Enemy->GetActorLocation(),
			TargetLocation
		);

		float PathLength = 0.f;

		if (NavPath && NavPath->IsValid() && NavPath->PathPoints.Num() > 1)
		{
			for (int32 i = 1; i < NavPath->PathPoints.Num(); i++)
			{
				PathLength += FVector::Dist(NavPath->PathPoints[i - 1], NavPath->PathPoints[i]);
			}

			Distances.Add({ Enemy, PathLength });
		}
	}

	Distances.Sort([](const FEnemyPathData& A, const FEnemyPathData& B)
	{
		return A.PathDistance < B.PathDistance;
	});

	TArray<AMeleeEnemy*> Result;

	if (Distances.Num() > 0) Result.Add(Distances[0].Enemy);
	if (Distances.Num() > 1) Result.Add(Distances[1].Enemy);

	return Result;
}

