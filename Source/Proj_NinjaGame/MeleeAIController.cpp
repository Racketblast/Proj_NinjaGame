// Fill out your copyright notice in the Description page of Project Settings.

#include "MeleeAIController.h"
#include "MeleeEnemy.h"
#include "TimerManager.h"
#include "AIController.h"
#include "NavigationPath.h"
#include "NavigationSystem.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Navigation/PathFollowingComponent.h"

AMeleeAIController::AMeleeAIController()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AMeleeAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	ControlledEnemy = Cast<AMeleeEnemy>(InPawn);
}

void AMeleeAIController::Tick(float DeltaSeconds)
{
	if (bBackingOff)
	{
		return;
	}
	Super::Tick(DeltaSeconds);
}

void AMeleeAIController::HandleChasing(float DeltaSeconds)
{
	//Super::HandleChasing(DeltaSeconds);
	APawn* Player = UGameplayStatics::GetPlayerPawn(GetWorld(),0);

	if (bBackingOff)
	{
		//MoveToLocation(BackOffLocation, 1.f);
		//UE_LOG(LogTemp, Error, TEXT("HandleChasing: bBackingOff"))
		return;
	}

	//UE_LOG(LogTemp, Warning, TEXT("CHASE TICK: ChasingFromExternalOrder:%d  CanSeePlayer:%d"), bChasingFromExternalOrder, ControlledEnemy->CanSeePlayer());
	// Kamerans chase
	if (bChasingFromExternalOrder)
	{

		// Om vi ser spelaren så byt till normal chase
		if (ControlledEnemy->CanSeePlayer())
		{
			bChasingFromExternalOrder = false;
		}
		return; // Va tidigare break, när detta var i tick, vet inte om return fungerar på samma sätt 
	}
			
	// Vanlig chase
	if (ControlledEnemy->CanSeePlayer())
	{
		float Dist = FVector::Dist(GetPawn()->GetActorLocation(), Player->GetActorLocation());

		bool bWithinMeleeRange = Dist <= ControlledEnemy->GetAttackRange();
		bool bWithinThrowRange = Dist <= ControlledEnemy->GetThrowRange();
		bool bCannotReach = CannotReachPlayer(Player);

		// Melee attack
		if (bWithinMeleeRange && ControlledEnemy->bCanAttack && !ControlledEnemy->bIsAttacking && ControlledEnemy->bAllowedToAttack)
		{
			StopMovement();
			UE_LOG(LogTemp, Error, TEXT("Melee Attack"))
			//ControlledEnemy->bCanAttack = false;
			ControlledEnemy->bIsAttacking = true;
			// Fiendens Animation blueprint startar melee attack animationen och kallar på StartAttack via en anim notify state
			return;
		}

		// Ranged attack
		if (!bWithinMeleeRange && bWithinThrowRange && bCannotReach && !ControlledEnemy->bIsThrowing && ControlledEnemy->bAllowedToAttack)
		{
			StopMovement();
			// Rotation
			//StartSmoothRotationTowards(Player->GetActorLocation(), 2.0f);
			FVector ToPlayer = Player->GetActorLocation() - ControlledEnemy->GetActorLocation();
			ToPlayer.Z = 0.f;
			if (!ToPlayer.IsNearlyZero())
			{
				ControlledEnemy->SetActorRotation(ToPlayer.Rotation());
			}

			//UE_LOG(LogTemp, Error, TEXT("Ranged Attack"))
			ControlledEnemy->bIsThrowing = true;
			//ControlledEnemy->EnemyThrow();
			if (RangedThrowCooldown > 2.0f)
			{
				//UE_LOG(LogTemp, Error, TEXT("Ranged Attack"))
				//ControlledEnemy->EnemyThrow();
				RangedThrowCooldown = 0.f; 
			}
			else
			{
				RangedThrowCooldown += DeltaSeconds;
			}
			return;
		}
		if (!bWithinMeleeRange && bWithinThrowRange && bCannotReach && ControlledEnemy->bAllowedToAttack)
		{
			StopMovement();
			FVector ToPlayer = Player->GetActorLocation() - ControlledEnemy->GetActorLocation();
			ToPlayer.Z = 0.f;
			if (!ToPlayer.IsNearlyZero())
			{
				ControlledEnemy->SetActorRotation(ToPlayer.Rotation());
			}
			return;
		}
		if (ControlledEnemy->bIsThrowing)
		{
			StopMovement();
			FVector ToPlayer = Player->GetActorLocation() - ControlledEnemy->GetActorLocation();
			ToPlayer.Z = 0.f;
			if (!ToPlayer.IsNearlyZero())
			{
				ControlledEnemy->SetActorRotation(ToPlayer.Rotation());
			}
			return;
		}

		// Annars fortsätt springa efter spelaren
		if (!bBackingOff)
		{
			//UE_LOG(LogTemp, Warning, TEXT("MoveToActor(Player)"));
			MoveToActor(Player);
		}
		GetWorldTimerManager().ClearTimer(LoseSightTimerHandle);
	}
	else if (!GetWorldTimerManager().IsTimerActive(LoseSightTimerHandle))
	{
		UE_LOG(LogTemp, Warning, TEXT("lost sight of player during chase. Calling StopChasing"));
		GetWorldTimerManager().SetTimer(LoseSightTimerHandle, this, &AMeleeAIController::StopChasing, ControlledEnemy->GetLoseSightTime(), false);
	}
}

void AMeleeAIController::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
	if (!ControlledEnemy || !IsValid(ControlledEnemy))
		return;
	if (GetWorld()->bIsTearingDown)
		return;

	bool bReachedBackOff = FVector::DistSquared(GetPawn()->GetActorLocation(), BackOffLocation) < FMath::Square(100.f); 

	if (bBackingOff && bReachedBackOff)
	{
		UE_LOG(LogTemp, Warning, TEXT(
		"OnMoveCompleted for BackOff: Result=%s"),
		*UEnum::GetValueAsString(Result.Code)
		);
		
		//UE_LOG(LogTemp, Warning, TEXT("BackOff move completed"));
		StopBackOff();
		return;
	}
	if (bBackingOff && !bReachedBackOff)
	{
		MoveToLocation(BackOffLocation);
	}

	Super::OnMoveCompleted(RequestID, Result);
}


void AMeleeAIController::StartChasing()
{
	Super::StartChasing();
}

void AMeleeAIController::StopChasing()
{
	Super::StopChasing();
}

void AMeleeAIController::RefreshChaseTarget()
{
	if (bBackingOff)
	{
		return;
	}
	Super::RefreshChaseTarget();
}


// Kallas just nu från kameran
/*void AMeleeAIController::StartChasingFromExternalOrder(FVector LastSpottedPlayerLocation)
{
	Super::StartChasingFromExternalOrder(LastKnownPlayerLocation);
}*/


void AMeleeAIController::OnUnPossess()
{
	Super::OnUnPossess();

	GetWorldTimerManager().ClearTimer(BackOffTimerHandle);
}


bool AMeleeAIController::CannotReachPlayer(APawn* Player)
{
	if (!Player || !GetPawn() || !ControlledEnemy) return true;
	
	// Ignorera ifall spelaren är i luften, för att stoppa att fienden kastar när spelaren hoppar. 
	if (UCharacterMovementComponent* MoveComp = Cast<UCharacterMovementComponent>(Player->GetMovementComponent()))
	{
		if (MoveComp->IsFalling())
		{
			return false; 
		}
	}

	UWorld* World = GetWorld();
	if (!World) return true;

	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(World);
	if (!NavSys) return true;

	// Hitta path  
	UNavigationPath* Path = NavSys->FindPathToLocationSynchronously(
		World,
		GetPawn()->GetActorLocation(),
		Player->GetActorLocation()
	);

	// Path är null eller invalid 
	if (!Path || !Path->IsValid())
		return true;
	
	const TArray<FVector>& Points = Path->PathPoints;
	
	if (Points.Num() <= 1)
		return true;

	// Sista pathpunktens position
	const FVector PathEnd = Points.Last();
	const float EndDist = FVector::Dist(PathEnd, Player->GetActorLocation());

	// Debug
	/*for (int32 i = 0; i < Points.Num(); ++i)
	{
		DrawDebugSphere(World, Points[i], 12.f, 8, FColor::Yellow, false, 0.5f);
		if (i > 0)
			DrawDebugLine(World, Points[i-1], Points[i], FColor::Yellow, false, 0.5f, 0, 2.f);
	}*/

	// Accepterbart avstånd: 1.0 = exakt melee range
	const float AcceptableDistance = ControlledEnemy->GetAttackRange() * 1.5f;

	return EndDist > AcceptableDistance;
}

void AMeleeAIController::StartBackOff(FVector BackLocation)
{
	APawn* Player = UGameplayStatics::GetPlayerPawn(GetWorld(),0);
	if (!Player) return;

	if (bBackingOff)
	{
		UE_LOG(LogTemp, Warning, TEXT("Already backing off, ignoring"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("StartBackOff: bBackingOff"))
	
	UNavigationSystemV1* Nav = UNavigationSystemV1::GetCurrent(GetWorld());
	if (!Nav)
	{
		UE_LOG(LogTemp, Warning, TEXT("StartBackOff: No NavigationSystem found!"));
		return;
	}

	// Project to navmesh
	FNavLocation ProjectedLoc;
	bool bValid = Nav->ProjectPointToNavigation(
		BackLocation,
		ProjectedLoc,
		FVector(200.f, 200.f, 300.f) 
	);

	if (!bValid)
	{
		UE_LOG(LogTemp, Warning, TEXT("StartBackOff: Location not valid on NavMesh, cancelling backoff."));
		return;
	}

	FVector FinalLocation = ProjectedLoc.Location;

	UE_LOG(LogTemp, Warning, TEXT("StartBackOff: Moving to projected location: %s"),
		*FinalLocation.ToString()
	);

	float NewDist = FVector::Dist(FinalLocation, Player->GetActorLocation());

	if (NewDist > ControlledEnemy->GetThrowRange() || NewDist < ControlledEnemy->GetAttackRange())
	{
		UE_LOG(LogTemp, Warning, TEXT("Backoff cancelled: Out of allowed range"));
		return;
	}

	/*if (!ControlledEnemy->IsLocationStillSeeingPlayer(FinalLocation))
	{
		UE_LOG(LogTemp, Warning, TEXT("Backoff cancelled: Would lose line of sight"));
		return; 
	}*/

	if (ControlledEnemy->GetVisionDebug())
	{
		DrawDebugSphere(
			GetWorld(),
			FinalLocation,
			50.f,
			12,
			FColor::Red,
			false,
			2.f
		);
	}

	
	bBackingOff = true;

	StopMovement();
	BackOffLocation = FinalLocation;
	MoveToLocation(FinalLocation, 1.f);
	
	GetWorldTimerManager().SetTimer(
		BackOffTimerHandle,
		this,
		&AMeleeAIController::StopBackOff,
		BackOffDuration,
		false
	);
}

void AMeleeAIController::StopBackOff()
{
	bBackingOff = false;
	
	UE_LOG(LogTemp, Warning, TEXT("StopBackOff."));
	
	APawn* Player = UGameplayStatics::GetPlayerPawn(GetWorld(),0);
	if (!Player) return;
	StartSmoothRotationTowards(Player->GetActorLocation(), 2.0f);
}
