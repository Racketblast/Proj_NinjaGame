// Fill out your copyright notice in the Description page of Project Settings.


#include "CollectableMissionObject.h"

#include "StealthCharacter.h"

ACollectableMissionObject::ACollectableMissionObject()
{
}

void ACollectableMissionObject::Use_Implementation(class AStealthCharacter* Player)
{
	Super::Use_Implementation(Player);

	Player->bHasCompletedTheMission = true;
	Destroy();
}

void ACollectableMissionObject::BeginPlay()
{
	Super::BeginPlay();
	
	StartVector = StaticMeshComponent->GetRelativeLocation();

	//Should be deleted when all guns uses the TakeDamage-function
	FVector HelpVector = GetActorLocation();
	HelpVector.Z += 10;
	SetActorLocation(HelpVector);
}

void ACollectableMissionObject::MoveObject(float DeltaTime)
{
	if(ShouldObjectReturn())
	{
		const FVector MoveDirection = MoveVelocity.GetSafeNormal();
		StartVector = StartVector + MoveDirection * GetDistanceMoved();
		StaticMeshComponent->SetRelativeLocation(StartVector);
		MoveVelocity = -MoveVelocity;
	}
	else
	{
		FVector LocalVector = StaticMeshComponent->GetRelativeLocation();
		LocalVector = LocalVector + MoveVelocity * DeltaTime;
		StaticMeshComponent->SetRelativeLocation(LocalVector);
	}
}

void ACollectableMissionObject::RotateObject(float DeltaTime)
{
	StaticMeshComponent->AddLocalRotation(RotationVelocity*DeltaTime);
}

bool ACollectableMissionObject::ShouldObjectReturn() const
{
	return GetDistanceMoved() > DistFloat;
}

float ACollectableMissionObject::GetDistanceMoved() const
{
	return FVector::Dist(StartVector, StaticMeshComponent->GetRelativeLocation());
}

void ACollectableMissionObject::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	MoveObject(DeltaTime);
	RotateObject(DeltaTime);
}
