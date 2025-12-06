// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyHandler.h"
#include "MeleeEnemy.h"
#include "Kismet/GameplayStatics.h"
#include "MeleeAIController.h"
#include "NavigationPath.h"
#include "NavigationSystem.h"
#include "SecurityCamera.h"
#include "TargetEnemy.h"


AEnemyHandler::AEnemyHandler()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AEnemyHandler::BeginPlay()
{
	Super::BeginPlay();

	// Hitta alla AEnemy i leveln
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AEnemy::StaticClass(), AllEnemies);
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASecurityCamera::StaticClass(), AllSecurityCameras);

	for (auto Enemy : AllEnemies)
	{
		if (AEnemy* MeleeEnemy = Cast<AEnemy>(Enemy))
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
		bAreAllEnemiesAlive = false;
		bAreAllEnemiesDead = (AllEnemies.Num() == 0);
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
		AEnemy* Enemy = Cast<AEnemy>(EnemyActor);
		if (!Enemy) continue;

		AEnemyAIController* AICon = Cast<AEnemyAIController>(Enemy->GetController());
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
			AmountOfTimesSpottet++;
			UE_LOG(LogTemp, Warning, TEXT("EnemyHandler: A enemy sees player: TRUE"));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("EnemyHandler: A enemy sees player: FALSE"));
		}
	}

	bEnemySeesPlayer = bAnyChasing;
}


AEnemy* AEnemyHandler::GetClosestEnemyToLocation(FVector TargetLocation)
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

	AEnemy* BestEnemy = nullptr;
	float BestDistance = TNumericLimits<float>::Max();

	for (AActor* EnemyActor : AllEnemies)
	{
		AEnemy* Enemy = Cast<AEnemy>(EnemyActor);
		if (!Enemy) continue;

		
		if (ATargetEnemy* TargetEnemy = Cast<ATargetEnemy>(EnemyActor))
			continue;
		
		// Skippa fiender som redan har ett mission
		AAIController* AICon = Cast<AAIController>(Enemy->GetController());
		if (AICon)
		{
			AEnemyAIController* AI = Cast<AEnemyAIController>(AICon);
			if (AI && AI->GetHasMission())
			{
				continue; 
			}
		}

		UNavigationPath* NavPath = NavSys->FindPathToLocationSynchronously(
			World,
			Enemy->GetActorLocation(),
			TargetLocation
		);

		float PathLength = 0.f;

		if (NavPath && NavPath->IsValid() && NavPath->PathPoints.Num() > 1)
		{
			// fienden måste nå nära målet
			FVector LastPoint = NavPath->PathPoints.Last();
			float EndDist = FVector::Dist(LastPoint, TargetLocation);
    
			if (EndDist > 500.f) 
			{
				continue;
			}


			
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

TArray<AEnemy*> AEnemyHandler::GetTwoClosestEnemies(FVector TargetLocation)
{
	struct FEnemyPathData
	{
		AEnemy* Enemy;
		float PathDistance;
	};

	TArray<FEnemyPathData> Distances;

	UWorld* World = GetWorld();
	if (!World) return {};

	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(World);
	if (!NavSys) return {};

	// Projektera målpunkten till navmesh
	FNavLocation ProjectedLocation;
	if (!NavSys->ProjectPointToNavigation(
			TargetLocation,
			ProjectedLocation,
			FVector(200.f, 200.f, 500.f)
		))
	{
		return {};
	}

	TargetLocation = ProjectedLocation.Location;

	for (AActor* Actor : AllEnemies)
	{
		AEnemy* Enemy = Cast<AEnemy>(Actor);
		if (!Enemy) continue;

		if (ATargetEnemy* TargetEnemy = Cast<ATargetEnemy>(Enemy))
			continue;
		
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

	TArray<AEnemy*> Result;

	if (Distances.Num() > 0) Result.Add(Distances[0].Enemy);
	if (Distances.Num() > 1) Result.Add(Distances[1].Enemy);

	return Result;
}

