// Fill out your copyright notice in the Description page of Project Settings.


#include "MeleeEnemy.h"

#include "AIController.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/CharacterMovementComponent.h"


AMeleeEnemy::AMeleeEnemy()
{
	PrimaryActorTick.bCanEverTick = true;
}


void AMeleeEnemy::BeginPlay()
{
	Super::BeginPlay();
	PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

void AMeleeEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	CheckPlayerVisibility();
}

void AMeleeEnemy::CheckPlayerVisibility()
{
	if (!PlayerPawn) return;

	FVector ToPlayer = PlayerPawn->GetActorLocation() - GetActorLocation();
	float Distance = ToPlayer.Size();

	if (Distance > VisionRange)
	{
		bCanSeePlayer = false;
		return;
	}

	FVector Forward = GetActorForwardVector();
	ToPlayer.Normalize();

	float Dot = FVector::DotProduct(Forward, ToPlayer);
	float Angle = FMath::Acos(Dot) * (180.f / PI);

	if (Angle > VisionAngle)
	{
		bCanSeePlayer = false;
		return;
	}

	// Line trace check
	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		Hit,
		GetActorLocation() + FVector(0,0,50),
		PlayerPawn->GetActorLocation(),
		ECC_Visibility,
		Params
	);

	if (!bHit || Hit.GetActor() == PlayerPawn)
	{
		bCanSeePlayer = true;
		DrawDebugLine(GetWorld(), GetActorLocation(), PlayerPawn->GetActorLocation(), FColor::Green, false, 0.05f);
		if (bCanSeePlayer)
		{
			UpdateLastSeenPlayerLocation();
		}
	}
	else
	{
		bCanSeePlayer = false;
		DrawDebugLine(GetWorld(), GetActorLocation(), Hit.Location, FColor::Red, false, 0.05f);
	}
}

void AMeleeEnemy::UpdateLastSeenPlayerLocation()
{
	if (PlayerPawn)
	{
		LastSeenPlayerLocation = PlayerPawn->GetActorLocation();
	}
}

