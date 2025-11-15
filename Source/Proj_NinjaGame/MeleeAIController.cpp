// Fill out your copyright notice in the Description page of Project Settings.


#include "MeleeAIController.h"
#include "MeleeEnemy.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "AIController.h"
#include "NavigationSystem.h"
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

	switch (CurrentState)
	{
	case EEnemyState::Patrolling:
		{
			if (ControlledEnemy->bPlayerInAlertCone) 
			{
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
			break;
		}
	case EEnemyState::Alert:
		{
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
				GetWorldTimerManager().SetTimer(LoseSightTimerHandle, this, &AMeleeAIController::StopChasing, ControlledEnemy->GetLoseSightTime(), false);
			}
			break;
		}
	case EEnemyState::Searching:
		{
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
			break;
		}
	}
	
	// Om fienden nyligen hört ett ljud
	if (!bIsInvestigatingSound && ControlledEnemy->bHeardSoundRecently && CurrentState != EEnemyState::Chasing)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Enemy heard sound at %s"), *ControlledEnemy->LastHeardSoundLocation.ToString());
		OnHeardSound(ControlledEnemy->LastHeardSoundLocation);
		
		bIsInvestigatingSound = true;
		
		/*GetWorldTimerManager().SetTimer(ResetSoundFlagHandle, [this]()
		{
			bIsInvestigatingSound = false;
			ControlledEnemy->bHeardSoundRecently = false;
		}, 0.5f, false);*/
		GetWorldTimerManager().SetTimer(ResetSoundFlagHandle, this, &AMeleeAIController::ResetSoundFlag, 0.5f, false);
	}
}

void AMeleeAIController::StartAlert()
{
	if (!ControlledEnemy) return;

	StopMovement();
	CurrentState = EEnemyState::Alert;
	ControlledEnemy->UpdateStateVFX(CurrentState); // För VFX 
	ControlledEnemy->GetCharacterMovement()->MaxWalkSpeed = 0.f; // stannar helt

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
	/*GetWorldTimerManager().SetTimer(AlertTimerHandle, [this]()
	{
		if (!ControlledEnemy->CanSeePlayer())
		{
			CurrentState = EEnemyState::Patrolling;
			ControlledEnemy->UpdateStateVFX(CurrentState);
			ControlledEnemy->GetCharacterMovement()->MaxWalkSpeed = ControlledEnemy->GetWalkSpeed();
			MoveToNextPatrolPoint();
		}
	}, 3.f, false);*/
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
	
	//UE_LOG(LogTemp, Warning, TEXT("PatrolPoints num: %d, index: %d"), ControlledEnemy->GetPatrolPoints().Num(), CurrentPatrolIndex);
	MoveToActor(TargetPoint);
}

void AMeleeAIController::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
	if (!ControlledEnemy || !IsValid(ControlledEnemy))
		return;
	if (GetWorld()->bIsTearingDown)
		return;
	
	Super::OnMoveCompleted(RequestID, Result);

	/*const TArray<AActor*>& PatrolPoints = ControlledEnemy->GetPatrolPoints();
	if (CurrentState == EEnemyState::Patrolling)
	{
		static int32 FailCount = 0;
		if (!Result.IsSuccess())
		{
			FailCount++;
			if (FailCount > 3)
			{
				UE_LOG(LogTemp, Warning, TEXT("Too many failed patrol moves, aborting patrol temporarily."));
				return;
			}
		}
		else
		{
			FailCount = 0; 
		}
		
		if (Result.IsSuccess())
		{
			// Lyckad förflyttning 
			if (PatrolPoints.Num() > 1)
			{
				FTimerHandle WaitHandle;
				GetWorldTimerManager().SetTimer(WaitHandle, [this]()
				{
					CurrentPatrolIndex = (CurrentPatrolIndex + 1) % ControlledEnemy->GetPatrolPoints().Num();
					MoveToNextPatrolPoint();
				}, ControlledEnemy->GetWaitTimeAtPoint(), false);
			}
		}
		else
		{
			// Misslyckad förflyttning 
			UE_LOG(LogTemp, Warning, TEXT("Patrol move failed at point index %d (%s). Restarting patrol route."),
				CurrentPatrolIndex,
				*PatrolPoints[CurrentPatrolIndex]->GetName());

			// Starta om patrullen
			CurrentPatrolIndex = 0;

			// Säkerhetsfördröjning så att pathfinding hinner stabilisera sig
			FTimerHandle RetryHandle;
			GetWorldTimerManager().SetTimer(RetryHandle, [this]()
			{
				MoveToNextPatrolPoint();
			}, 0.5f, false);
		}

		if (!ControlledEnemy)
			return;
	}*/
	

	// Kolla att detta inte är ett gammalt move som avbrutits av ett nytt
	/*if (Result.Code == EPathFollowingResult::Aborted)
	{
		UE_LOG(LogTemp, Warning, TEXT("Move aborted (ignored old move)."));
		return;
	}*/

	if (Result.Code == EPathFollowingResult::Aborted)
	{
		// Endast ignorera aborter från gamla patrull-moves, inte ljud-moves
		if (CurrentState == EEnemyState::Patrolling)
		{
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
	
	if (CurrentState == EEnemyState::Patrolling)
	{
		if (Result.IsSuccess())
		{
			// Gå vidare till nästa patrullpunkt
			if (PatrolPoints.Num() > 1)
			{
				FTimerHandle WaitHandle;
				/*GetWorldTimerManager().SetTimer(WaitHandle, [this]()
				{
					CurrentPatrolIndex = (CurrentPatrolIndex + 1) % ControlledEnemy->GetPatrolPoints().Num();
					MoveToNextPatrolPoint();
				}, ControlledEnemy->GetWaitTimeAtPoint(), false);*/
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
			/*GetWorldTimerManager().SetTimer(RetryHandle, [this]()
			{
				MoveToNextPatrolPoint();
			}, 1.0f, false);*/
			GetWorldTimerManager().SetTimer(RetryHandle, this, &AMeleeAIController::RetryMoveToNextPatrolPoint, 1.0f, false);
		}
	}

	// när fienden nått ljudkällan 
	if (CurrentState == EEnemyState::Searching && bIsMovingToSound)
	{
		//UE_LOG(LogTemp, Warning, TEXT("AI finished moving to sound, starting search."));
		bIsMovingToSound = false;
	}

	/*UE_LOG(LogTemp, Warning, TEXT("PatrolPoints num: %d, index: %d, success: %d"),
		PatrolPoints.Num(), CurrentPatrolIndex, Result.IsSuccess());*/

	
	// Failsafe för searching moves 
	/*if (CurrentState == EEnemyState::Searching)
	{
		if (!Result.IsSuccess())
		{
			UE_LOG(LogTemp, Warning, TEXT("Searching move failed. Ending search early."));

			// Avsluta search
			StopMovement();
			bIsLookingAround = false;
			bIsMovingToSound = false;
			bIsInvestigatingSound = false;

			// Återgå till patrullering
			CurrentState = EEnemyState::Patrolling;
			ControlledEnemy->UpdateStateVFX(CurrentState);

			// Starta patrull igen
			MoveToNextPatrolPoint();
		}
	}*/
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
		LastKnownPlayerLocation = ControlledEnemy->GetLastSeenPlayerLocation();
		CurrentState = EEnemyState::Searching;
		ControlledEnemy->UpdateStateVFX(CurrentState); // För VFX
		MoveToLocation(LastKnownPlayerLocation);
	}
}


void AMeleeAIController::BeginSearch()
{
	if (!ControlledEnemy || !IsValid(ControlledEnemy)) return;
	if (!GetPawn()) return;
	if (GetWorld()->bIsTearingDown) return;
	
	StopMovement();

	//UE_LOG(LogTemp, Warning, TEXT("AMeleeAIController BeginSearch 1"));
	
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

	//Slumpar rotation
	FRotator NewRotation = ControlledPawn->GetActorRotation();
	NewRotation.Yaw += FMath::RandRange(-90.f, 90.f);
	ControlledPawn->SetActorRotation(NewRotation);

	// Rör fienden lite slumpmässigt
	MoveToLocation(ControlledPawn->GetActorLocation() + ControlledPawn->GetActorForwardVector() * FMath::RandRange(100.f, 250.f));
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

	// Reset all searching states
	GetWorldTimerManager().ClearTimer(LookAroundTimerHandle);
	GetWorldTimerManager().ClearTimer(EndSearchTimerHandle);
	bIsLookingAround = false;
	bIsMovingToSound = false;
	bIsInvestigatingSound = false;

	// Ignore sounds while chasing
	if (CurrentState == EEnemyState::Chasing) return;
	
	StopMovement();
	
	CurrentState = EEnemyState::Searching;
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
	
	MoveToLocation(GroundedLocation, -1.f, true, true, false, false, 0, true);
}



void AMeleeAIController::HandleSuspiciousLocation(FVector Location)
{
	if (!IsValid(this)) return;
	
	UE_LOG(LogTemp, Warning, TEXT("HandleSuspiciousLocation"));
	/*StopMovement();
	FRotator LookAtRot = (Location - GetPawn()->GetActorLocation()).Rotation();
	GetPawn()->SetActorRotation(LookAtRot);*/
	
	if (CurrentState == EEnemyState::Patrolling)
	{
		UE_LOG(LogTemp, Warning, TEXT("Enemy is investigating suspicious location!"));
		CurrentState = EEnemyState::Searching;
		ControlledEnemy->UpdateStateVFX(CurrentState);
		MoveToLocation(Location);
	}
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

	/*if (ControlledEnemy->bHeardSoundRecently)
	{
		return;
	}*/

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
	MoveToNextPatrolPoint();
}

