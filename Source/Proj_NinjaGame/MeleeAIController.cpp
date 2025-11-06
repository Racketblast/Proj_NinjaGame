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
			if (ControlledEnemy->CanSeePlayer())
			{
				MoveToActor(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
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
			if (ControlledEnemy->CanSeePlayer())
			{
				// Om fienden ser spelaren igen så börjar den jaga direkt
				GetWorldTimerManager().ClearTimer(LookAroundTimerHandle);
				GetWorldTimerManager().ClearTimer(EndSearchTimerHandle);
				bIsLookingAround = false;
				StartChasing();
				break;
			}
			
			if (!bIsLookingAround && FVector::Dist(GetPawn()->GetActorLocation(), LastKnownPlayerLocation) < 150.f)
			{
				bIsLookingAround = true;
				//UE_LOG(LogTemp, Warning, TEXT("AMeleeAIController Searching Tick"));
				BeginSearch();
			}
			break;
		}
	}
}

void AMeleeAIController::MoveToNextPatrolPoint()
{
	if (!ControlledEnemy) return;

	const TArray<AActor*>& PatrolPoints = ControlledEnemy->GetPatrolPoints();
	if (PatrolPoints.Num() == 0) return;
	
	// Sätt den nuvarande target positionen att gå mott
	MoveToActor(PatrolPoints[CurrentPatrolIndex]);
}

void AMeleeAIController::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
	Super::OnMoveCompleted(RequestID, Result);

	if (CurrentState == EEnemyState::Patrolling && Result.IsSuccess())
	{
		// När vi når en patrul punkt, växla till nästa
		const TArray<AActor*>& PatrolPoints = ControlledEnemy->GetPatrolPoints();
		if (PatrolPoints.Num() > 1)
		{
			// Vänta lite innan vi går till nästa punkt
			FTimerHandle WaitHandle;
			GetWorldTimerManager().SetTimer(WaitHandle, [this]()
			{
				CurrentPatrolIndex = (CurrentPatrolIndex + 1) % ControlledEnemy->GetPatrolPoints().Num();
				MoveToNextPatrolPoint();
			}, ControlledEnemy->GetWaitTimeAtPoint(), false);
		}
	}
}


void AMeleeAIController::StartChasing()
{
	CurrentState = EEnemyState::Chasing;

	if (ControlledEnemy)
	{
		ControlledEnemy->GetCharacterMovement()->MaxWalkSpeed = ControlledEnemy->GetRunSpeed(); 
	}
	
	MoveToActor(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
}

void AMeleeAIController::StopChasing()
{
	GetWorldTimerManager().ClearTimer(LoseSightTimerHandle);
	if (ControlledEnemy)
	{
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
