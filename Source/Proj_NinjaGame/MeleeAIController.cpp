// Fill out your copyright notice in the Description page of Project Settings.


#include "MeleeAIController.h"
#include "MeleeEnemy.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "AIController.h"
#include "NavigationPath.h"
#include "NavigationSystem.h"
#include "PatrolPoint.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Navigation/PathFollowingComponent.h"

AMeleeAIController::AMeleeAIController()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AMeleeAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	ControlledEnemy = Cast<AMeleeEnemy>(InPawn);

	if (ControlledEnemy)
	{
		CurrentPatrolIndex = 0;
		CurrentState = EEnemyState::Patrolling;

		ControlledEnemy->OnSuspiciousLocation.AddDynamic(this, &AMeleeAIController::HandleSuspiciousLocation);

		//UE_LOG(LogTemp, Warning, TEXT("AMeleeAIController OnPossess"));

		ControlledEnemy->UpdateStateVFX(CurrentState); // För VFX

		// Vänta en liten stund innan patrullering startar (Det fungerade inte utan denna delay)
		GetWorldTimerManager().SetTimer(StartPatrolTimerHandle, this, &AMeleeAIController::MoveToNextPatrolPoint, 0.2f, false);
	}
}

void AMeleeAIController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	if (!ControlledEnemy) return;

	if (bIsStunned)
	{
		return; 
	}

	// Rotation
	if (bIsRotating)
	{
		FRotator Current = ControlledEnemy->GetActorRotation();
		FRotator Interp = FMath::RInterpTo(Current, DesiredRotation, DeltaSeconds, CurrentRotationSpeed);

		Interp.Pitch = 0.f;
		Interp.Roll = 0.f;

		ControlledEnemy->SetActorRotation(Interp);

		if (Interp.Equals(DesiredRotation, 2.0f)) 
		{
			bIsRotating = false;
		}

		return; // Under rotation gör fienden inget annat
	}

	switch (CurrentState)
	{
	case EEnemyState::Patrolling:
		{
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
				//UE_LOG(LogTemp, Warning, TEXT("Patrolling: Heard sound at %s"), *ControlledEnemy->LastHeardSoundLocation.ToString());
				OnHeardSound(ControlledEnemy->LastHeardSoundLocation);
				bIsInvestigatingSound = true;
				
				GetWorldTimerManager().SetTimer(ResetSoundFlagHandle, this, &AMeleeAIController::ResetSoundFlag, 0.5f, false);
			}
			
			// Mission 
			if (bHasMission && CurrentMission != EEnemyMission::Patrol && CurrentState != EEnemyState::Alert)
			{
				StartMissionMoveTo(CurrentMissionLocation);
				break;
			}

			// Rotation
			if (bIsRotatingTowardPatrolPoint)
			{
				RotationProgress += DeltaSeconds / 2.0f; 

				FRotator Current = ControlledEnemy->GetActorRotation();
				FRotator Interp  = FMath::RInterpTo(Current, DesiredLookRotation, DeltaSeconds, 2.5f);

				// Lås pitch och roll igen 
				Interp.Pitch = 0.f;
				Interp.Roll  = 0.f;

				ControlledEnemy->SetActorRotation(Interp);

				// Kolla när vi är nästan klar
				if (Current.Equals(DesiredLookRotation, 3.0f)) // tolerans är 3 grader
				{
					bIsRotatingTowardPatrolPoint = false;

					// När klar så börjar fienden gå
					MoveToActor(
						ControlledEnemy->GetPatrolPoints()[CurrentPatrolIndex]
					);
				}
			}
			break;
		}
	case EEnemyState::Alert:
		{
			if (!GetWorldTimerManager().IsTimerActive(AlertTimerHandle))
			{
				GetWorldTimerManager().SetTimer(AlertTimerHandle, this, &AMeleeAIController::OnAlertTimerExpired, 3.f, false);
			}
			// Om fienden ser spelaren tillräckligt tydligt så börja jaga
			if (ControlledEnemy->CanSeePlayer())
			{
				StartChasing();
			}
			else
			{
				// titta mot spelaren 
				APawn* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
				if (Player)
				{
					FVector ToPlayer = Player->GetActorLocation() - ControlledEnemy->GetActorLocation();
					ToPlayer.Z = 0;
					if (!ToPlayer.IsNearlyZero())
					{
						FRotator TargetRot = ToPlayer.Rotation();
						FRotator NewRot = FMath::RInterpTo(
							ControlledEnemy->GetActorRotation(),
							TargetRot,
							DeltaSeconds,
							3.5f
						);
						ControlledEnemy->SetActorRotation(NewRot);
					}
				}
			}

			break;
		}
	case EEnemyState::Chasing:
		{
			APawn* Player = UGameplayStatics::GetPlayerPawn(GetWorld(),0);

			// Kamerans chase
			if (bChasingFromExternalOrder)
			{
				// Om vi ser spelaren så byt till normal chase
				if (ControlledEnemy->CanSeePlayer())
				{
					bChasingFromExternalOrder = false;
				}
				break;
			}
			
			// Vanlig chase
			if (ControlledEnemy->CanSeePlayer())
			{
				float Dist = FVector::Dist(GetPawn()->GetActorLocation(), Player->GetActorLocation());
				if (Dist <= ControlledEnemy->GetAttackRange())
				{
					StopMovement();
					ControlledEnemy->StartAttack();
				}
				else
				{
					MoveToActor(Player);
				}
				GetWorldTimerManager().ClearTimer(LoseSightTimerHandle);
			}
			else if (!GetWorldTimerManager().IsTimerActive(LoseSightTimerHandle))
			{
				UE_LOG(LogTemp, Warning, TEXT("lost sight of player during chase. Calling StopChasing"));
				GetWorldTimerManager().SetTimer(LoseSightTimerHandle, this, &AMeleeAIController::StopChasing, ControlledEnemy->GetLoseSightTime(), false);
			}
			
			
			break;
		}
	case EEnemyState::Searching:
		{
			if (bIsMovingToSound && bIsInvestigatingTarget)
			{
				// Gör inget om vi roterar
				if (!bIsRotating)
				{
					bIsDoingMissionMoveTo = false;
					MoveToLocation(InvestigateTarget, -1.f, true, true, false, false, 0, true);
				}
			}

			if (bHasLookAroundTarget && !bIsRotating)
			{
				bIsDoingMissionMoveTo = false;
				MoveToLocation(LookAroundTarget);
			}
			
			if (ControlledEnemy->bPlayerInAlertCone) 
			{
				StartAlert();
				//UE_LOG(LogTemp, Warning, TEXT("StartAlert"));
			}
			
			// Om fienden ser spelaren igen så börjar den jaga direkt
			if (ControlledEnemy->CanSeePlayer())
			{
				GetWorldTimerManager().ClearTimer(LookAroundTimerHandle);
				GetWorldTimerManager().ClearTimer(EndSearchTimerHandle);
				bIsLookingAround = false;
				StartChasing();
				break;
			}

			// Börjar leta efter spelaren. Ser till att BeginSearch() inte kallas flera gånger. 
			if (!bIsLookingAround && FVector::Dist(GetPawn()->GetActorLocation(), LastKnownPlayerLocation) < 150.f)
			{
				bIsLookingAround = true;
				//UE_LOG(LogTemp, Warning, TEXT("AMeleeAIController Searching Tick"));
				BeginSearch();
			}


			
			// Movement Failsafe
			FVector CurrentLocation = GetPawn()->GetActorLocation();
			FVector Delta = CurrentLocation - LastSearchLocation;
			float DistanceMoved = Delta.Size();

			// Kolla hastighet 
			float Speed = ControlledEnemy->GetVelocity().Size();

			if (Speed < SearchFailSpeedThreshold && DistanceMoved < 10.f)
			{
				// Fienden står stilla eller rör sig knappt
				TimeWithoutMovement += DeltaSeconds;
			}
			else
			{
				// Fienden rör sig igen, så reset failsafe
				TimeWithoutMovement = 0.f;
			}

			// Spara senaste positionen
			LastSearchLocation = CurrentLocation;

			// Trigga failsafe
			if (TimeWithoutMovement >= SearchFailTime)
			{
				UE_LOG(LogTemp, Warning, TEXT("Search failsafe triggered, returning to patrol."));

				TimeWithoutMovement = 0.f;
				bIsLookingAround = false;
				bIsMovingToSound = false;
				bIsInvestigatingSound = false;

				CurrentState = EEnemyState::Patrolling;
				ControlledEnemy->UpdateStateVFX(CurrentState);
				MoveToNextPatrolPoint();
			}

			//Mission failsafe
			if (bIsDoingMissionMoveTo)
			{
				FVector CurrentLocationMission = GetPawn()->GetActorLocation();
				FVector DeltaMission = CurrentLocationMission - MissionLastLocation;
				float DistanceMovedMission = DeltaMission.Size();
				float SpeedMission = GetPawn()->GetVelocity().Size();

				if (SpeedMission < MissionFailSafeSpeedThreshold && DistanceMovedMission < 5.f)
				{
					// Fienden står still eller fastnat
					MissionTimeWithoutMovement += DeltaSeconds;
				}
				else
				{
					// Nollställ failsafe om fienden börjar röra sig igen
					MissionTimeWithoutMovement = 0.f;
				}

				// Uppdatera sista positionen
				MissionLastLocation = CurrentLocationMission;

				// Trigga failsafe
				if (MissionTimeWithoutMovement >= MissionFailSafeTime)
				{
					UE_LOG(LogTemp, Warning, TEXT("MISSION FAILSAFE TRIGGERED: Enemy stuck, aborting mission-move."));

					MissionTimeWithoutMovement = 0.f;
					bIsDoingMissionMoveTo = false;

					StartMissionMoveTo(CurrentMissionLocation);
					return;
				}
			}

			break;
		}
	}
	
	// Om fienden nyligen hört ett ljud
	if (!bIsInvestigatingSound && ControlledEnemy->bHeardSoundRecently && CurrentState != EEnemyState::Chasing)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Enemy heard sound at %s"), *ControlledEnemy->LastHeardSoundLocation.ToString());
		OnHeardSound(ControlledEnemy->LastHeardSoundLocation);
		
		bIsInvestigatingSound = true;
		
		GetWorldTimerManager().SetTimer(ResetSoundFlagHandle, this, &AMeleeAIController::ResetSoundFlag, 0.5f, false);
	}

	RunChaseFailsafe(DeltaSeconds);

}

void AMeleeAIController::StartAlert()
{
	if (!ControlledEnemy) return;

	StopMovement();
	CurrentState = EEnemyState::Alert;
	ControlledEnemy->UpdateStateVFX(CurrentState); // För VFX 
	ControlledEnemy->GetCharacterMovement()->MaxWalkSpeed = 0.f; // stannar helt

	bIsDoingMissionMoveTo = false;

	// Titta mot spelaren direkt
	APawn* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (Player)
	{
		FVector ToPlayer = Player->GetActorLocation() - ControlledEnemy->GetActorLocation();
		ToPlayer.Z = 0.f;
		if (!ToPlayer.IsNearlyZero())
		{
			ControlledEnemy->SetActorRotation(ToPlayer.Rotation());
		}
	}

	// Efter några sekunder om fienden fortfarande inte ser spelaren så återgår den till patrullering
	GetWorldTimerManager().SetTimer(AlertTimerHandle, this, &AMeleeAIController::OnAlertTimerExpired, 3.f, false);
}


void AMeleeAIController::MoveToNextPatrolPoint()
{
	if (!ControlledEnemy || !IsValid(ControlledEnemy))
		return;
	if (GetWorld()->bIsTearingDown)
		return;

	const TArray<AActor*>& PatrolPoints = ControlledEnemy->GetPatrolPoints();
	const int32 NumPoints = PatrolPoints.Num();
	if (NumPoints == 0) return;

	// Säkerställ att index är inom intervallet
	if (CurrentPatrolIndex < 0 || CurrentPatrolIndex >= NumPoints)
	{
		CurrentPatrolIndex = 0;
	}

	APawn* MyPawn = GetPawn();
	if (!MyPawn) return;
	
	// Sätt den nuvarande target positionen att gå mott
	AActor* TargetPoint = PatrolPoints[CurrentPatrolIndex];
	if (!TargetPoint) return;
	
	const float Distance = FVector::Dist(MyPawn->GetActorLocation(), TargetPoint->GetActorLocation());
	if (Distance < 300.f) 
	{
		// Gå vidare till nästa punkt i stället för att fastna
		CurrentPatrolIndex = (CurrentPatrolIndex + 1) % PatrolPoints.Num();
		TargetPoint = PatrolPoints[CurrentPatrolIndex];
		if (!TargetPoint) return;
	}


	
	
	// Beräkna rotation
	FVector Dir = (TargetPoint->GetActorLocation() - ControlledEnemy->GetActorLocation());
	DesiredLookRotation = Dir.Rotation();

	// Tvinga pitch och roll till 0
	DesiredLookRotation.Pitch = 0.f;
	DesiredLookRotation.Roll  = 0.f;

	// Starta långsam rotation
	bIsRotatingTowardPatrolPoint = true;
	RotationProgress = 0.f;

	// Stoppa allt moveTo medan vi roterar
	StopMovement();

	
	
	
	//UE_LOG(LogTemp, Warning, TEXT("PatrolPoints num: %d, index: %d"), ControlledEnemy->GetPatrolPoints().Num(), CurrentPatrolIndex);
	//MoveToActor(TargetPoint);
}

void AMeleeAIController::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
	if (!ControlledEnemy || !IsValid(ControlledEnemy))
		return;
	if (GetWorld()->bIsTearingDown)
		return;
	
	Super::OnMoveCompleted(RequestID, Result);

	if (Result.Code == EPathFollowingResult::Aborted)
	{
		// Endast ignorera aborter från gamla patrull-moves, inte ljud-moves
		if (CurrentState == EEnemyState::Patrolling)
		{
			//UE_LOG(LogTemp, Warning, TEXT("OnMoveCompleted RequestID: %s Result: %d CurrentState: %d bIsMovingToSound: %d"), *RequestID.ToString(), (int)Result.Code, (int)CurrentState, (int)bIsMovingToSound);
			UE_LOG(LogTemp, Warning, TEXT("Move aborted (ignored patrol move)."));
			return;
		}
		else
		{
			//UE_LOG(LogTemp, Warning, TEXT("Move aborted but new sound may override it."));
		}
	}

	const TArray<AActor*>& PatrolPoints = ControlledEnemy->GetPatrolPoints();
	if (PatrolPoints.Num() == 0)
	{
		//UE_LOG(LogTemp, Warning, TEXT("OnMoveCompleted aborted: PatrolPoints empty or enemy destroyed."));
		return;
	}

	if (CurrentPatrolIndex < 0 || CurrentPatrolIndex >= PatrolPoints.Num())
	{
		UE_LOG(LogTemp, Warning, TEXT("OnMoveCompleted invalid patrol index (%d / %d)."), CurrentPatrolIndex, PatrolPoints.Num());
		return;
	}

	/*UE_LOG(LogTemp, Warning, TEXT("Move completed for %s: Code=%d Reason=%d"),
	*ControlledEnemy->GetName(),
	(int)Result.Code, (int)Result.Flags);*/

	// För mission
	if (Result.IsSuccess() && bHasMission && bIsDoingMissionMoveTo)
	{
		if (CurrentMission != EEnemyMission::Patrol) //  && FVector::Dist(ControlledEnemy->GetActorLocation(), CurrentMissionLocation) < 350.f
		{
			UE_LOG(LogTemp, Warning, TEXT("OnMoveCompleted: CompleteMission"));
			bIsDoingMissionMoveTo = false;
			CompleteMission();
		}
	}
	/*if (Result.IsSuccess() && RequestID == MissionRequestID)
	{
		UE_LOG(LogTemp, Warning, TEXT("OnMoveCompleted: CompleteMission"));
		CompleteMission();
	}*/
	
	if (CurrentState == EEnemyState::Patrolling)
	{
		if (Result.IsSuccess())
		{
			// Rotation från PatrolPoint 
			AActor* TargetPoint = PatrolPoints[CurrentPatrolIndex];
			if (APatrolPoint* PatrolPoint = Cast<APatrolPoint>(TargetPoint))
			{
				if (PatrolPoint->bUseCustomRotation)
				{
					ControlledEnemy->SetActorRotation(PatrolPoint->CustomRotation);
				}
			}

			// Gå vidare till nästa patrullpunkt
			if (PatrolPoints.Num() > 1)
			{
				FTimerHandle WaitHandle;
				TWeakObjectPtr<AMeleeAIController> WeakThis(this); // Använde detta för att stoppa en tidigare krasch 
				GetWorldTimerManager().SetTimer(WaitHandle, [WeakThis]()
				{
					if (!WeakThis.IsValid()) return;
					AMeleeAIController* Self = WeakThis.Get();
					if (!Self->ControlledEnemy || !IsValid(Self->ControlledEnemy)) return;

					Self->CurrentPatrolIndex = (Self->CurrentPatrolIndex + 1) % Self->ControlledEnemy->GetPatrolPoints().Num();
					Self->MoveToNextPatrolPoint();
				}, ControlledEnemy->GetWaitTimeAtPoint(), false);
			}
		}
		else
		{
			// försök igen på samma punkt efter liten delay
			UE_LOG(LogTemp, Warning, TEXT("Patrol move failed at point index %d (%s). Retrying..."),
				CurrentPatrolIndex,
				*PatrolPoints[CurrentPatrolIndex]->GetName());
			
			FTimerHandle RetryHandle;
			GetWorldTimerManager().SetTimer(RetryHandle, this, &AMeleeAIController::RetryMoveToNextPatrolPoint, 1.0f, false);
		}
	}

	// när fienden nått ljudkällan 
	if (CurrentState == EEnemyState::Searching && bIsMovingToSound)
	{
		bIsMovingToSound = false;
		bIsInvestigatingTarget = false;
		//UE_LOG(LogTemp, Warning, TEXT("AI finished moving to sound, starting search."));
		bIsMovingToSound = false;
	}

	if (bHasLookAroundTarget && CurrentState == EEnemyState::Searching)
	{
		bHasLookAroundTarget = false;
	}
	
	float Dist = FVector::Dist(ControlledEnemy->GetActorLocation(), LastKnownPlayerLocation);
	if (bChasingFromExternalOrder && Dist < 150.f)
	{
		UE_LOG(LogTemp, Warning, TEXT("Chase move reached last known player position"));
		bChasingFromExternalOrder = false;

		return;
	}

	/*UE_LOG(LogTemp, Warning, TEXT("PatrolPoints num: %d, index: %d, success: %d"),
		PatrolPoints.Num(), CurrentPatrolIndex, Result.IsSuccess());*/
}


void AMeleeAIController::StartChasing()
{
	CurrentState = EEnemyState::Chasing;

	if (ControlledEnemy)
	{
		ControlledEnemy->UpdateStateVFX(CurrentState); // För VFX
		ControlledEnemy->bIsChasing = true;
		ControlledEnemy->GetCharacterMovement()->MaxWalkSpeed = ControlledEnemy->GetRunSpeed(); 
	}
	
	MoveToActor(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
}

void AMeleeAIController::StopChasing()
{
	GetWorldTimerManager().ClearTimer(LoseSightTimerHandle);
	if (ControlledEnemy)
	{
		ControlledEnemy->bIsChasing = false;
		OnTargetLost(); 
	}
}

void AMeleeAIController::RefreshChaseTarget()
{
	if (!ControlledEnemy || !IsValid(ControlledEnemy)) return;

	APawn* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (!Player) return;

	if (CurrentState == EEnemyState::Chasing)
	{
		const float Dist = FVector::Dist(Player->GetActorLocation(), GetPawn()->GetActorLocation());

		// Uppdatera bara path om spelaren rört sig tillräckligt
		if (Dist > 10.f) 
		{
			MoveToActor(Player);
		}
	}
}



void AMeleeAIController::OnTargetLost()
{
	if (ControlledEnemy)
	{
		ControlledEnemy->GetCharacterMovement()->MaxWalkSpeed = ControlledEnemy->GetWalkSpeed();
		
		if (!bChasingFromExternalOrder)
		{
			LastKnownPlayerLocation = ControlledEnemy->GetLastSeenPlayerLocation();
		}
		
		CurrentState = EEnemyState::Searching;
		TimeWithoutMovement = 0.f; // För failsafe
		LastSearchLocation = ControlledEnemy->GetActorLocation(); // För failsafe
		ControlledEnemy->UpdateStateVFX(CurrentState); // För VFX

		bIsDoingMissionMoveTo = false;
		MoveToLocation(LastKnownPlayerLocation);
	}
}


void AMeleeAIController::BeginSearch()
{
	if (!ControlledEnemy || !IsValid(ControlledEnemy)) return;
	if (!GetPawn()) return;
	if (GetWorld()->bIsTearingDown) return;
	
	StopMovement();

	LookAroundCount = 0; // reset varje gång en ny sökning startas

	//UE_LOG(LogTemp, Warning, TEXT("AMeleeAIController BeginSearch 1"));
	
	LookAround();
	
	// kallar på LookAround() några gånger
	GetWorldTimerManager().SetTimer(LookAroundTimerHandle, this, &AMeleeAIController::LookAround, 1.5f, true);
	
	// Efter SearchTime, avsluta sökningen
	GetWorldTimerManager().SetTimerForNextTick([this]()
	{
		GetWorldTimerManager().SetTimer(EndSearchTimerHandle, this, &AMeleeAIController::EndSearch, ControlledEnemy->GetSearchTime(), false);
	});
}

void AMeleeAIController::LookAround()
{
	if (!ControlledEnemy || !IsValid(ControlledEnemy)) return;
	if (GetWorld()->bIsTearingDown) return;
	//UE_LOG(LogTemp, Warning, TEXT("AMeleeAIController LookAround 1"));
	
	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn) return;

	LookAroundCount++;
	if (LookAroundCount > LookAroundMax)
	{
		// stoppa timer
		//UE_LOG(LogTemp, Warning, TEXT("LookAround max count reached"));
		GetWorldTimerManager().ClearTimer(LookAroundTimerHandle);
		return;
	}

	//UE_LOG(LogTemp, Warning, TEXT("AMeleeAIController LookAround"));
	
	//Slumpar rotation
	FRotator Current = ControlledPawn->GetActorRotation();
	FRotator NewRotation = Current;
	
	//NewRotation.Yaw += FMath::RandRange(-360.f, 360.f);
	float BaseStep = FMath::RandBool() ? 90.f : -90.f;
	float Variation = FMath::RandRange(-20.f, 20.f);
	float Step = BaseStep + Variation;
	NewRotation.Yaw += Step;


	StartSmoothRotationTowards(NewRotation.Vector(), 2.0f);

	FVector LookDirection = NewRotation.Vector();


	// Hämta NavSys
	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
	if (!NavSys) return;

	const float MinDistance = 100.f;
	const float MaxDistance = 350.f;
	const float MaxAllowedPathLength = 400.f; 

	FVector Origin = ControlledPawn->GetActorLocation();

	bool bFoundGoodPoint = false;
	FVector BestPoint = Origin;

	// Gör upp till 5 försök att hitta en bra punkt
	for (int i = 0; i < 5; i++)
	{
		// Slumpa mål
		FVector RawTarget = Origin + LookDirection * FMath::RandRange(MinDistance, MaxDistance);

		// Project to NavMesh
		FNavLocation Projected;
		if (!NavSys->ProjectPointToNavigation(RawTarget, Projected))
			continue;

		// Kolla path
		UNavigationPath* Path = NavSys->FindPathToLocationSynchronously(GetWorld(), Origin, Projected.Location);

		if (Path && Path->IsValid() && Path->PathPoints.Num() > 1)
		{
			float PathLength = 0.f;

			for (int32 p = 1; p < Path->PathPoints.Num(); p++)
			{
				PathLength += FVector::Distance(Path->PathPoints[p - 1], Path->PathPoints[p]);
			}

			// DEBUG LOG
			//UE_LOG(LogTemp, Warning, TEXT("LookAround path length: %.1f"), PathLength);

			if (PathLength <= MaxAllowedPathLength)
			{
				BestPoint = Projected.Location;
				bFoundGoodPoint = true;
				break;
			}
		}
	}

	// Gå till punkten om en bra punkt hittadfes
	if (bFoundGoodPoint)
	{
		LookAroundTarget = BestPoint;
	}
	else
	{
		// Stå kvar om ingen bra punkt hittades
		LookAroundTarget = Origin;
	}

	bHasLookAroundTarget = true;
	

	/*// Slumpa ett forward move
	FVector RawTarget = ControlledPawn->GetActorLocation() + LookDirection * FMath::RandRange(100.f, 350.f);

	
	// Project to NavMesh 
	FNavLocation Projected;
	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());

	if (NavSys && NavSys->ProjectPointToNavigation(RawTarget, Projected))
	{
		LookAroundTarget = Projected.Location;
	}
	else
	{
		// Stå still om den inte hittar en plats på navmeshen 
		LookAroundTarget = ControlledPawn->GetActorLocation();
	}

	bHasLookAroundTarget = true;*/
	
	//MoveToLocation(ControlledPawn->GetActorLocation() + ControlledPawn->GetActorForwardVector() * FMath::RandRange(100.f, 250.f));
}

void AMeleeAIController::EndSearch()
{
	if (!ControlledEnemy || !IsValid(ControlledEnemy)) return;
	if (!GetPawn()) return;
	if (GetWorld()->bIsTearingDown) return;
	
	GetWorldTimerManager().ClearTimer(LookAroundTimerHandle);
	GetWorldTimerManager().ClearTimer(EndSearchTimerHandle);
	
	if (ControlledEnemy && ControlledEnemy->CanSeePlayer())
	{
		bIsLookingAround = false;
		StartChasing();
		return;
	}
	
	bIsLookingAround = false;
	//bIsInvestigatingSound = false;

	// Endast återgå till patrull om fienden inte hört något nytt nyligen
	if (!ControlledEnemy->bHeardSoundRecently)
	{
		CurrentState = EEnemyState::Patrolling;
		ControlledEnemy->UpdateStateVFX(CurrentState); // För VFX
		MoveToNextPatrolPoint();
	}
}

void AMeleeAIController::OnHeardSound(FVector SoundLocation)
{
	if (!ControlledEnemy) return;

	bHasLookAroundTarget = false;
	LookAroundTarget = FVector::ZeroVector;

	// Reset all searching states
	GetWorldTimerManager().ClearTimer(LookAroundTimerHandle);
	GetWorldTimerManager().ClearTimer(EndSearchTimerHandle);
	bIsLookingAround = false;
	bIsMovingToSound = false;
	bIsInvestigatingSound = false;

	// Ingnorera ljud när fienden redan jagar spelaren. 
	if (CurrentState == EEnemyState::Chasing) return;

	// Ingnorera ljud ifall spelaren är i fiendens alert vision cone. 
	if (ControlledEnemy->bPlayerInAlertCone) return;
	
	StopMovement();
	
	CurrentState = EEnemyState::Searching;
	TimeWithoutMovement = 0.f; // För failsafe
	LastSearchLocation = ControlledEnemy->GetActorLocation(); // För failsafe
	ControlledEnemy->UpdateStateVFX(CurrentState);

	// Adjust sound height 
	FVector GroundedLocation = SoundLocation;

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(ControlledEnemy);

	FVector TraceStart = SoundLocation + FVector(0, 0, 200);
	FVector TraceEnd   = SoundLocation - FVector(0, 0, 500);

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
	
	LastKnownPlayerLocation = GroundedLocation;
	bIsMovingToSound = true;

	DrawDebugSphere(GetWorld(), SoundLocation, 25.f, 12, FColor::Yellow, false, 5.f);  
	DrawDebugSphere(GetWorld(), GroundedLocation, 25.f, 12, FColor::Green, false, 5.f);

	UE_LOG(LogTemp, Warning, TEXT("OnHeardSound triggered: Enemy moving toward sound at %s"), *GroundedLocation.ToString());

	
	bIsInvestigatingTarget = true;
	InvestigateTarget = GroundedLocation;
	StartSmoothRotationTowards(GroundedLocation, 4.0f);
	
	
	//MoveToLocation(GroundedLocation, -1.f, true, true, false, false, 0, true);
}

void AMeleeAIController::HandleSuspiciousLocation(FVector Location)
{
    if (!IsValid(this) || !ControlledEnemy || !IsValid(ControlledEnemy)) return;

    //UE_LOG(LogTemp, Warning, TEXT("HandleSuspiciousLocation: Trying to move to: %s"), *Location.ToString());

	bool bIsMission = bHasMission && (Location == CurrentMissionLocation);
	if (!bIsMission)
	{
		if (CurrentState == EEnemyState::Chasing || CurrentState == EEnemyState::Searching)
		{
			UE_LOG(LogTemp, Warning, TEXT("HandleSuspiciousLocation ignored because currently chasing/searching."));
			return;
		}
	}

    UE_LOG(LogTemp, Warning, TEXT("Enemy is investigating suspicious location!"));
    CurrentState = EEnemyState::Searching;
    TimeWithoutMovement = 0.f; // För failsafe
    LastSearchLocation = ControlledEnemy->GetActorLocation(); // För failsafe
    ControlledEnemy->UpdateStateVFX(CurrentState);
    ControlledEnemy->GetCharacterMovement()->MaxWalkSpeed = ControlledEnemy->GetWalkSpeed();

    // Project to navmesh
    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
    FNavLocation NavLoc;
    if (NavSys && NavSys->ProjectPointToNavigation(Location, NavLoc, FVector(ControlledEnemy->HearingRange)))
    {
        LastKnownPlayerLocation = NavLoc.Location;
        //bIsMovingToSound = true;

        UE_LOG(LogTemp, Warning, TEXT("HandleSuspiciousLocation: Moving to projected navmesh point: %s"), *NavLoc.Location.ToString());

        // Debug draw
        DrawDebugSphere(GetWorld(), Location, 30.f, 12, FColor::Yellow, false, 8.f);
        DrawDebugSphere(GetWorld(), NavLoc.Location, 30.f, 12, FColor::Green, false, 8.f);
        //DrawDebugLine(GetWorld(), ControlledEnemy->GetActorLocation(), NavLoc.Location, FColor::Blue, false, 8.f, 0, 2.f);
    	
        const float AcceptanceRadius = 50.f;

    	bIsDoingMissionMoveTo = false;
        MoveToLocation(NavLoc.Location, AcceptanceRadius, true, true, true, false, nullptr);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Suspicious location not on navmesh! Location: %s"), *Location.ToString());
        // Fallback, ifall den inte kan ta sig till locationen så bara undersök lite på stället innan den går tillbaka till patrullering.  
        //bIsMovingToSound = false;
        BeginSearch();
    }
}


void AMeleeAIController::StartSmoothRotationTowards(const FVector& TargetLocation, float RotationSpeed)
{
	if (!ControlledEnemy) return;

	FVector Dir = TargetLocation - ControlledEnemy->GetActorLocation();
	Dir.Z = 0.f; 

	DesiredRotation = Dir.Rotation();
	DesiredRotation.Pitch = 0.f;
	DesiredRotation.Roll = 0.f;

	CurrentRotationSpeed = RotationSpeed;
	bIsRotating = true;
}

// Kallas just nu från kameran
void AMeleeAIController::StartChasingFromExternalOrder(FVector LastSpottedPlayerLocation)
{
	CurrentState = EEnemyState::Chasing;
	
	bChasingFromExternalOrder = true; 

	if (ControlledEnemy)
	{
		ControlledEnemy->UpdateStateVFX(CurrentState); // För VFX
		ControlledEnemy->bIsChasing = true;
		LastKnownPlayerLocation = LastSpottedPlayerLocation;
		ControlledEnemy->SetLastSeenPlayerLocation(LastSpottedPlayerLocation);
		ControlledEnemy->GetCharacterMovement()->MaxWalkSpeed = ControlledEnemy->GetRunSpeed(); 
	}

	StopMovement();
	
	UE_LOG(LogTemp, Warning, TEXT("CHASE: %s is starting chase to %s"),
	*ControlledEnemy->GetName(),
	*LastKnownPlayerLocation.ToString());

	bIsDoingMissionMoveTo = true;
	MoveToLocation(LastKnownPlayerLocation);
}


void AMeleeAIController::OnUnPossess()
{
	if (ControlledEnemy)
	{
		ControlledEnemy->OnSuspiciousLocation.RemoveDynamic(this, &AMeleeAIController::HandleSuspiciousLocation);
		ControlledEnemy = nullptr;
	}

	// Rensa alla timers
	GetWorldTimerManager().ClearAllTimersForObject(this);
	
	Super::OnUnPossess();

	// Rensa timers 
	GetWorldTimerManager().ClearTimer(StartPatrolTimerHandle);
	GetWorldTimerManager().ClearTimer(LoseSightTimerHandle);
	GetWorldTimerManager().ClearTimer(LookAroundTimerHandle);
	GetWorldTimerManager().ClearTimer(EndSearchTimerHandle);
	GetWorldTimerManager().ClearTimer(AlertTimerHandle);
	GetWorldTimerManager().ClearTimer(StunTimerHandle);
}


void AMeleeAIController::RunChaseFailsafe(float DeltaSeconds)
{
	if (CurrentState != EEnemyState::Chasing) return;
		
	//UE_LOG(LogTemp, Warning, TEXT(" %s is starting to count for CHASE FAILSAFE "), *ControlledEnemy->GetName());
		
	FVector CurrentLocation = ControlledEnemy->GetActorLocation();
	FVector Delta = CurrentLocation - LastChaseLocation;
	float DistanceMoved = Delta.Size();

	float Speed = ControlledEnemy->GetVelocity().Size();
	
	if (!ControlledEnemy->CanSeePlayer())
	{
		if (Speed < ChaseFailSpeedThreshold && DistanceMoved < 10.f)
		{
			TimeWithoutMovement_Chase += DeltaSeconds;
		}
		else
		{
			TimeWithoutMovement_Chase = 0.f;
		}
	}

	LastChaseLocation = CurrentLocation;

	// Trigga failsafe
	if (TimeWithoutMovement_Chase >= ChaseFailTime)
	{
		UE_LOG(LogTemp, Warning, TEXT("CHASE FAILSAFE: Enemy %s stuck, returning to patrol."), *ControlledEnemy->GetName());

		TimeWithoutMovement_Chase = 0.f;
		bChasingFromExternalOrder = false;
		ControlledEnemy->bIsChasing = false;

		// Avbryt chase
		CurrentState = EEnemyState::Patrolling;
		ControlledEnemy->UpdateStateVFX(CurrentState);
		ControlledEnemy->GetCharacterMovement()->MaxWalkSpeed = ControlledEnemy->GetWalkSpeed();

		StopMovement();
		MoveToNextPatrolPoint();
	}
}

//Mission system
void AMeleeAIController::AssignMission(EEnemyMission NewMission, FVector MissionLocation)
{
	if (bHasMission || NewMission == EEnemyMission::Patrol)
		return;


	
	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
	FNavLocation ProjectedLoc;

	FVector Extent(50.f, 50.f, 500.f); 

	// Project to navmesh
	if (NavSys && NavSys->ProjectPointToNavigation(MissionLocation, ProjectedLoc, Extent))
	{
		CurrentMissionLocation = ProjectedLoc.Location;

		UE_LOG(LogTemp, Warning, TEXT("AssignMission: Projected mission point from %s to %s"),
			*MissionLocation.ToString(),
			*CurrentMissionLocation.ToString());

		// Debug
		DrawDebugSphere(GetWorld(), MissionLocation, 25.f, 12, FColor::Yellow, false, 8.f);
		DrawDebugSphere(GetWorld(), CurrentMissionLocation, 25.f, 12, FColor::Green, false, 8.f);
	}
	else
	{
		// Om projicering misslyckas så behåll originalet
		CurrentMissionLocation = MissionLocation;
		UE_LOG(LogTemp, Error, TEXT("AssignMission: FAILED to project mission point: %s"), *MissionLocation.ToString());
	}
	
	CurrentMission = NewMission;
	bHasMission = true;

	// Om fienden just nu patrullerar så starta direkt. 
	if (CurrentState == EEnemyState::Patrolling)
	{
		StartMissionMoveTo(MissionLocation);
	}

	//UE_LOG(LogTemp, Warning, TEXT("Assigned mission %d with target %s"), (int)NewMission, *MissionLocation.ToString());
}


void AMeleeAIController::CompleteMission()
{
	UE_LOG(LogTemp, Warning, TEXT("Mission complete!"));

	bHasMission = false;
	bIsDoingMissionMoveTo = false;
	CurrentMission = EEnemyMission::Patrol;

	// Gå tillbaka till patrull
	CurrentState = EEnemyState::Patrolling;
	ControlledEnemy->UpdateStateVFX(CurrentState);
	MoveToNextPatrolPoint();
}

void AMeleeAIController::StartMissionMoveTo(FVector Location)
{
    if (!IsValid(this) || !ControlledEnemy || !IsValid(ControlledEnemy)) return;

	if (bIsDoingMissionMoveTo) return;

	bool bIsMission = bHasMission && (Location == CurrentMissionLocation);
	if (!bIsMission)
	{
		if (CurrentState == EEnemyState::Chasing || CurrentState == EEnemyState::Alert)
		{
			UE_LOG(LogTemp, Warning, TEXT("MissionMoveTo ignored because currently chasing/searching."));
			return;
		}
	}
	
	ControlledEnemy->UpdateStateVFX(CurrentState);
    ControlledEnemy->GetCharacterMovement()->MaxWalkSpeed = ControlledEnemy->GetWalkSpeed();
	
	// Initiera failsafe-data
	MissionTimeWithoutMovement = 0.f;
	MissionLastLocation = ControlledEnemy->GetActorLocation();

    // Project to navmesh
    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
    FNavLocation NavLoc;
    if (NavSys && NavSys->ProjectPointToNavigation(Location, NavLoc, FVector(ControlledEnemy->HearingRange)))
    {
        LastKnownPlayerLocation = NavLoc.Location;

        UE_LOG(LogTemp, Warning, TEXT("StartMissionMoveTo: Moving to projected navmesh point: %s"), *NavLoc.Location.ToString());

        // Debug draw
        DrawDebugSphere(GetWorld(), Location, 30.f, 12, FColor::Yellow, false, 8.f);
        DrawDebugSphere(GetWorld(), NavLoc.Location, 30.f, 12, FColor::Green, false, 8.f);
        //DrawDebugLine(GetWorld(), ControlledEnemy->GetActorLocation(), NavLoc.Location, FColor::Blue, false, 8.f, 0, 2.f);

    	bIsDoingMissionMoveTo = true;
    	
        float AcceptanceRadius = 50.f;

    	if (CurrentMission == EEnemyMission::Electrical)
    	{
    		AcceptanceRadius = 20;
    	}
    	
        MoveToLocation(NavLoc.Location, AcceptanceRadius, true, true, true, false, nullptr);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("MissionMoveTo location not on navmesh! Location: %s"), *Location.ToString());
        // Fallback, ifall den inte kan ta sig till locationen så bara undersök lite på stället innan den går tillbaka till patrullering.  
        BeginSearch();
    }
}

//Stun
void AMeleeAIController::StunEnemy(float Duration, TOptional<EEnemyState> WantedState)
{
	if (!IsValid(this) || !ControlledEnemy || !IsValid(ControlledEnemy)) return;
	
	if (bIsStunned) return; 

	bIsStunned = true;

	UE_LOG(LogTemp, Warning, TEXT("StunEnemy"));

	StateBeforeStun = CurrentState;

	// Stoppa all movement
	StopMovement();
	ControlledEnemy->GetCharacterMovement()->MaxWalkSpeed = 0.f; // stannar helt

	// Sätt StateAfterStun
	if (WantedState.IsSet())
	{
		StateAfterStun = WantedState.GetValue();
	}

	// Kör en timer som slutar stunen
	GetWorldTimerManager().SetTimer(
		StunTimerHandle,
		this,
		&AMeleeAIController::EndStun,
		Duration,
		false
	);
}


// Time handle funktioner:
void AMeleeAIController::ResetSoundFlag()
{
	bIsInvestigatingSound = false;
	if (ControlledEnemy && IsValid(ControlledEnemy))
	{
		ControlledEnemy->bHeardSoundRecently = false;
	}
}

void AMeleeAIController::OnAlertTimerExpired()
{
	if (!ControlledEnemy || !IsValid(ControlledEnemy)) return;

	if (CurrentState == EEnemyState::Searching)
	{
		ControlledEnemy->GetCharacterMovement()->MaxWalkSpeed = ControlledEnemy->GetWalkSpeed();
		return;
	}

	if (ControlledEnemy->bPlayerInAlertCone) return;

	if (!ControlledEnemy->CanSeePlayer())
	{
		CurrentState = EEnemyState::Patrolling;
		ControlledEnemy->UpdateStateVFX(CurrentState);
		ControlledEnemy->GetCharacterMovement()->MaxWalkSpeed = ControlledEnemy->GetWalkSpeed();
		MoveToNextPatrolPoint();
	}
}

void AMeleeAIController::RetryMoveToNextPatrolPoint()
{
	StopMovement();
	ControlledEnemy->GetCharacterMovement()->MaxWalkSpeed = ControlledEnemy->GetWalkSpeed();
	MoveToNextPatrolPoint();
}

void AMeleeAIController::EndStun()
{
	if (!ControlledEnemy || !IsValid(ControlledEnemy)) return;
	
	bIsStunned = false;

	if (StateBeforeStun == EEnemyState::Chasing && StateAfterStun != EEnemyState::Chasing)
	{
		ControlledEnemy->GetCharacterMovement()->MaxWalkSpeed = ControlledEnemy->GetWalkSpeed();
		StopChasing();
	}
	else if (StateBeforeStun == EEnemyState::Chasing && StateAfterStun == EEnemyState::Chasing)
	{
		LastKnownPlayerLocation = ControlledEnemy->GetLastSeenPlayerLocation();
		ControlledEnemy->GetCharacterMovement()->MaxWalkSpeed = ControlledEnemy->GetRunSpeed();
		StartChasingFromExternalOrder(LastKnownPlayerLocation);
	}
	else
	{
		ControlledEnemy->GetCharacterMovement()->MaxWalkSpeed = ControlledEnemy->GetWalkSpeed();
	}
	
	CurrentState = StateAfterStun;
	ControlledEnemy->UpdateStateVFX(CurrentState);
}
