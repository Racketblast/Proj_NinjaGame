// Fill out your copyright notice in the Description page of Project Settings.


#include "MeleeEnemy.h"

#include "AIController.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"
#include "MeleeAIController.h"
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

/*void AMeleeEnemy::CheckPlayerVisibility()
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
}*/

void AMeleeEnemy::CheckPlayerVisibility()
{
    if (!PlayerPawn) return;

	// Rita endast konen när vi patrullerar
	if (!bIsChasing)
	{
		FVector EnemyLocation = GetActorLocation() + FVector(0, 0, 50);
		FVector Forward = GetActorForwardVector();
		FVector LookDirection = Forward.RotateAngleAxis(20.f, GetActorRightVector()); 

		FColor ConeColor = FColor::Yellow;
		DrawDebugCone(
			GetWorld(),
			EnemyLocation,
			LookDirection,
			VisionRange * 0.6f,
			FMath::DegreesToRadians(VisionAngle * 0.5f),
			FMath::DegreesToRadians(VisionAngle * 0.5f),
			12,
			ConeColor,
			false,
			0.1f 
		);
	}

	//UE_LOG(LogTemp, Warning, TEXT("CheckPlayerVisibility"));

    // Skillnad mellan patrull och chase-läge 
    float EffectiveVisionRange = bIsChasing ? VisionRange : VisionRange * 0.6f;
    float EffectiveVisionAngle = bIsChasing ? VisionAngle : VisionAngle * 0.5f;

    FVector EnemyLocation = GetActorLocation() + FVector(0, 0, 50);
    FVector Forward = GetActorForwardVector();

    // Rikta synfältet nedåt 
    FVector LookDirection = Forward.RotateAngleAxis(20.f, GetActorRightVector()); 

    FVector ToPlayer = PlayerPawn->GetActorLocation() - EnemyLocation;
    float Distance = ToPlayer.Size();

    if (Distance > EffectiveVisionRange)
    {
        bCanSeePlayer = false;
        return;
    }

    ToPlayer.Normalize();
    float Dot = FVector::DotProduct(LookDirection, ToPlayer);
    float Angle = FMath::Acos(Dot) * (180.f / PI);

    if (Angle > EffectiveVisionAngle)
    {
        bCanSeePlayer = false;
        return;
    }

    // Line trace för sikt 
    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    bool bHit = GetWorld()->LineTraceSingleByChannel(
        Hit,
        EnemyLocation,
        PlayerPawn->GetActorLocation(),
        ECC_Visibility,
        Params
    );

    if (!bHit || Hit.GetActor() == PlayerPawn)
    {
        bCanSeePlayer = true;
        UpdateLastSeenPlayerLocation();

        DrawDebugLine(GetWorld(), EnemyLocation, PlayerPawn->GetActorLocation(), FColor::Green, false, 0.05f);
    }
    else
    {
        bCanSeePlayer = false;
        DrawDebugLine(GetWorld(), EnemyLocation, Hit.Location, FColor::Red, false, 0.05f);
    }
}


void AMeleeEnemy::UpdateLastSeenPlayerLocation()
{
	if (PlayerPawn)
	{
		LastSeenPlayerLocation = PlayerPawn->GetActorLocation();
	}
}

