// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyHandler.h"

#include "BodyguardEnemy.h"
#include "DialogueInfo.h"
#include "EnemyMarkerWidget.h"
#include "MeleeEnemy.h"
#include "MissionHandler.h"
#include "Kismet/GameplayStatics.h"
#include "MeleeAIController.h"
#include "NavigationPath.h"
#include "NavigationSystem.h"
#include "SecurityCamera.h"
#include "TargetEnemy.h"
#include "Algo/RandomShuffle.h"


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


	/*for (auto Enemy : AllEnemies)
	{
		if (AEnemy* MeleeEnemy = Cast<AEnemy>(Enemy))
		{
			MeleeEnemy->SetEnemyHandler(this);
		}
	}*/

	TArray<UDataTable*> SelectableVoices = GetAllSelectableVoices();
	TArray<AEnemy*> EnemyList;
	for (AActor* Actor : AllEnemies)
	{
		if (AEnemy* Enemy = Cast<AEnemy>(Actor))
		{
			EnemyList.Add(Enemy);
			Enemy->SetEnemyHandler(this);

			if (!Enemy->GetVoiceDataTable())
			{
				Enemy->SetVoiceDataTable(GetRandomEnemyVoice(SelectableVoices));
			}
		}
	}

	int32 TotalEnemies = EnemyList.Num();

	if (TotalEnemies == 0)
		return;

	// Hur många fiender som ska ha hjälm
	int32 TargetHelmetCount = FMath::RoundToInt((HelmetChancePercent / 100.f) * TotalEnemies);

	UE_LOG(LogTemp, Warning, TEXT(" %d fiender ska få en hjälm"), TargetHelmetCount );

	// Räkna hur många som redan har hjälm 
	TArray<AEnemy*> WithoutHelmetEnemies;

	for (AEnemy* Enemy : EnemyList)
	{
		if (!Enemy->DoesHaveHelmet())
		{
			WithoutHelmetEnemies.Add(Enemy);
		}
	}

	// Om vi behöver lägga till hjälmar
	if (TargetHelmetCount > 0)
	{
		// shuffle den WithoutHelmetEnemies listan för att slumpa vilka som får hjälm
		Algo::RandomShuffle(WithoutHelmetEnemies);

		int32 AssignCount = FMath::Min(TargetHelmetCount, WithoutHelmetEnemies.Num());

		for (int32 i = 0; i < AssignCount; i++)
		{
			WithoutHelmetEnemies[i]->SetHaveHelmet(true);
			UE_LOG(LogTemp, Warning, TEXT(" %d fiender har fått hjälm"), i + 1 );
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

	
	MissionHandler = Cast<AMissionHandler>(
		UGameplayStatics::GetActorOfClass(GetWorld(), AMissionHandler::StaticClass())
	);

	if (!MissionHandler)
	{
		UE_LOG(LogTemp, Error, TEXT("EnemyHandler: Could not find MissionHandler!"));
	}
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
		
		if (!Cast<ATargetEnemy>(EnemyRemoved))
		{
			// Endast sätt false om den inte är en target enemy
			bAreAllEnemiesAlive = false;
		}
		
		bAreAllEnemiesDead = (AllEnemies.Num() == 0);

		if (!bEnemySeesPlayer && !bAnyAlert)
		{
			if (MissionHandler)
			{
				MissionHandler->AddStealthKillScore();

				UE_LOG(LogTemp, Warning, TEXT("EnemyHandler: Stealth kill detected! Calling AddStealthKillScore()"));
			}
		}
	} 
}

void AEnemyHandler::RemoveCamera(AActor* CameraRemoved)
{
	if (AllSecurityCameras.Contains(CameraRemoved))
	{
		AllSecurityCameras.Remove(CameraRemoved);
	}
}


UDataTable* AEnemyHandler::GetRandomEnemyVoice(TArray<UDataTable*> SelectableVoices)
{
	if (SelectableVoices.Num() <= 0)
		return nullptr;
	
	if (SelectableVoices.Num() == 1)
	{
		return SelectableVoices[0];
	}
	
	int RandomNumber = FMath::RandRange(0, SelectableVoices.Num() - 1);

	if (RandomNumber == PreviousVoiceIndex)
	{
		//Wraps back to 0 if to high number, works better than a while loop
		RandomNumber = (RandomNumber + 1) % SelectableVoices.Num();
	}
	
	PreviousVoiceIndex = RandomNumber;
	//UE_LOG(LogTemp, Warning, TEXT("Voice number: %d"), RandomNumber);
	return SelectableVoices[RandomNumber];
}

TArray<UDataTable*> AEnemyHandler::GetAllSelectableVoices()
{
	TArray<UDataTable*> SelectableVoices;
	
	if (EnemyVoiceTables.Num() <= 0)
		return TArray<UDataTable*>();
	
	for (auto VoiceTable : EnemyVoiceTables)
	{
		if (VoiceTable && VoiceTable->GetRowStruct() == FDialogueInfo::StaticStruct())
		{
			if (!SelectableVoices.Contains(VoiceTable))
			{
				//UE_LOG(LogTemp, Warning, TEXT("Voice that can be selected: %s"), *VoiceTable->GetName());
				SelectableVoices.Add(VoiceTable);
			}
		}
	}
	
	return SelectableVoices;
}

void AEnemyHandler::UpdateEnemyStates()
{
	bool bAnyChasing = false;
	bool bAnyAlertLocal = false;

	// Går igenom alla fiender för att se om någon ut av dem är i Chasing State eller Alert state
	for (AActor* EnemyActor : AllEnemies)
	{
		AEnemy* Enemy = Cast<AEnemy>(EnemyActor);
		if (!Enemy) continue;

		AEnemyAIController* AICon = Cast<AEnemyAIController>(Enemy->GetController());
		if (!AICon) continue;

		EEnemyState CurrentState = AICon->GetCurrentState();

		if (CurrentState == EEnemyState::Chasing)
		{
			AddActorSeesPlayer(Enemy);
			bAnyChasing = true;
		}

		if (CurrentState == EEnemyState::Alert)
		{
			AddActorSeesPlayer(Enemy);
			bAnyAlertLocal = true;
		}
	}

	for (AActor* CameraActor : AllSecurityCameras)
	{
		ASecurityCamera* Camera = Cast<ASecurityCamera>(CameraActor);
		if (!Camera) continue;

		if (Camera->GetHasSpottedPlayer())
		{
			AddActorSeesPlayer(Camera);
			bAnyChasing = true;
		}

		if (Camera->GetPlayerInCone())
		{
			AddActorSeesPlayer(Camera);
			bAnyAlertLocal = true;
		}
	}
	
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
	
	bAnyAlert = bAnyAlertLocal;

	bEnemySeesPlayer = bAnyChasing;
	
	// Debug
	//UE_LOG(LogTemp, Log, TEXT("EnemyHandler: Alert=%s | Chasing=%s"), bAnyAlert ? TEXT("TRUE") : TEXT("FALSE"), bEnemySeesPlayer ? TEXT("TRUE") : TEXT("FALSE"));
}

void AEnemyHandler::AddActorSeesPlayer(AActor* Actor)
{
	if (!AllSeesPlayer.Contains(Actor))
	{
		AllSeesPlayer.Add(Actor);
		
		if (EnemyMarkerWidget)
		{
			EnemyMarkerWidget->AddNewEnemyMarker(Actor);
		}
	}
}

void AEnemyHandler::RemoveActorSeesPlayer(AActor* Actor)
{
	if (AllSeesPlayer.Contains(Actor))
	{
		AllSeesPlayer.Remove(Actor);
	}
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

		if (Cast<ABodyguardEnemy>(EnemyActor))
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
		FVector(200.f, 200.f, 500.f)))
	{
		return {};
	}

	TargetLocation = ProjectedLocation.Location;

	for (AActor* Actor : AllEnemies)
	{
		AEnemy* Enemy = Cast<AEnemy>(Actor);
		if (!Enemy) continue;

		if (Cast<ATargetEnemy>(Enemy))
			continue;

		if (Cast<ABodyguardEnemy>(Enemy))
			continue;

		// Skippa fiender som redan jagar spelaren
		if (AEnemyAIController* AICon = Cast<AEnemyAIController>(Enemy->GetController()))
		{
			if (AICon->GetCurrentState() == EEnemyState::Chasing)
			{
				continue;
			}
		}

		UNavigationPath* NavPath = NavSys->FindPathToLocationSynchronously(
			World,
			Enemy->GetActorLocation(),
			TargetLocation
		);

		if (!NavPath || !NavPath->IsValid() || NavPath->IsPartial() || NavPath->PathPoints.Num() < 2)
		{
			continue;
		}
		
		FVector LastPoint = NavPath->PathPoints.Last();
		float EndDist = FVector::Dist(LastPoint, TargetLocation);

		if (EndDist > 500.f)
		{
			continue;
		}

		// Räkna ut total path längd
		float PathLength = 0.f;
		for (int32 i = 1; i < NavPath->PathPoints.Num(); i++)
		{
			PathLength += FVector::Dist(
				NavPath->PathPoints[i - 1],
				NavPath->PathPoints[i]
			);
		}

		Distances.Add({ Enemy, PathLength });
	}

	// Sortera närmast först
	Distances.Sort([](const FEnemyPathData& A, const FEnemyPathData& B)
	{
		return A.PathDistance < B.PathDistance;
	});

	TArray<AEnemy*> Result;
	if (Distances.Num() > 0) Result.Add(Distances[0].Enemy);
	if (Distances.Num() > 1) Result.Add(Distances[1].Enemy);

	return Result;
}


