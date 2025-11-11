// Fill out your copyright notice in the Description page of Project Settings.


#include "MeleeAIController.h"
#include "MeleeEnemy.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "AIController.h"
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

		//UE_LOG(LogTemp, Warning, TEXT("AMeleeAIController OnPossess"));

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
			if (ControlledEnemy->CanSeePlayer())
			{
				StartChasing();
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
	if (ControlledEnemy->bHeardSoundRecently && CurrentState != EEnemyState::Chasing)
	{
		UE_LOG(LogTemp, Warning, TEXT("Enemy heard sound at %s"), *ControlledEnemy->LastHeardSoundLocation.ToString());
		OnHeardSound(ControlledEnemy->LastHeardSoundLocation);
		ControlledEnemy->bHeardSoundRecently = false;
	}
}

void AMeleeAIController::MoveToNextPatrolPoint()
{
	if (!ControlledEnemy) return;

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
	Super::OnMoveCompleted(RequestID, Result);

	const TArray<AActor*>& PatrolPoints = ControlledEnemy->GetPatrolPoints();
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
	}

	/*UE_LOG(LogTemp, Warning, TEXT("PatrolPoints num: %d, index: %d, success: %d"),
		PatrolPoints.Num(), CurrentPatrolIndex, Result.IsSuccess());*/
}


void AMeleeAIController::StartChasing()
{
	CurrentState = EEnemyState::Chasing;

	if (ControlledEnemy)
	{
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



void AMeleeAIController::OnTargetLost()
{
	if (ControlledEnemy)
	{
		ControlledEnemy->GetCharacterMovement()->MaxWalkSpeed = ControlledEnemy->GetWalkSpeed();
		LastKnownPlayerLocation = ControlledEnemy->GetLastSeenPlayerLocation();
		CurrentState = EEnemyState::Searching;
		MoveToLocation(LastKnownPlayerLocation);
	}
}


void AMeleeAIController::BeginSearch()
{
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
	GetWorldTimerManager().ClearTimer(LookAroundTimerHandle);
	GetWorldTimerManager().ClearTimer(EndSearchTimerHandle);
	
	if (ControlledEnemy && ControlledEnemy->CanSeePlayer())
	{
		bIsLookingAround = false;
		StartChasing();
		return;
	}
	
	bIsLookingAround = false;

	// Gå tillbaka till patrull
	CurrentState = EEnemyState::Patrolling;
	MoveToNextPatrolPoint();
}


void AMeleeAIController::OnHeardSound(FVector SoundLocation)
{
	if (!ControlledEnemy) return;

	// Bara reagera om fienden inte redan jagar spelaren
	if (CurrentState == EEnemyState::Chasing) return;

	UE_LOG(LogTemp, Warning, TEXT("AI heard a sound and is investigating"));

	StopMovement();

	CurrentState = EEnemyState::Searching;
	LastKnownPlayerLocation = SoundLocation;

	MoveToLocation(SoundLocation);
}

void AMeleeAIController::OnUnPossess()
{
	Super::OnUnPossess();

	// Rensa timers 
	GetWorldTimerManager().ClearTimer(StartPatrolTimerHandle);
	GetWorldTimerManager().ClearTimer(LoseSightTimerHandle);
	GetWorldTimerManager().ClearTimer(LookAroundTimerHandle);
	GetWorldTimerManager().ClearTimer(EndSearchTimerHandle);

	ControlledEnemy = nullptr;
}
