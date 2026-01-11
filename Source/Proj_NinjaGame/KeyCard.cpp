// Fill out your copyright notice in the Description page of Project Settings.


#include "KeyCard.h"

#include "Door.h"
#include "StealthCharacter.h"

AKeyCard::AKeyCard()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	StaticMeshComponent->SetCanEverAffectNavigation(false);
}

void AKeyCard::Use_Implementation(class AStealthCharacter* Player)
{
	Super::Use_Implementation(Player);
	
	if (!Player) return;

	if (!Player->KeyCards.Contains(this->SpecificKeyCardType))
	{
		Player->KeyCards.Add(this->SpecificKeyCardType);
	}
	
	for (auto Door : DoorsToUnlock)
	{
		if (!Door->GetUnlockedDoor() && Door)
		{
			Door->UnlockDoor();
		}
	}
	
	Destroy();
}

bool AKeyCard::ContainsDoor(class ADoor* Door)
{
	return DoorsToUnlock.Contains(Door);
}

void AKeyCard::MoveObject(float DeltaTime)
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

void AKeyCard::RotateObject(float DeltaTime)
{
	StaticMeshComponent->AddLocalRotation(RotationVelocity*DeltaTime);
}

bool AKeyCard::ShouldObjectReturn() const
{
	return GetDistanceMoved() > DistFloat;
}

float AKeyCard::GetDistanceMoved() const
{
	return FVector::Dist(StartVector, StaticMeshComponent->GetRelativeLocation());
}

void AKeyCard::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	MoveObject(DeltaSeconds);
	RotateObject(DeltaSeconds);
}

void AKeyCard::BeginPlay()
{
	Super::BeginPlay();
	
	StartVector = StaticMeshComponent->GetRelativeLocation();

	//Should be deleted when all guns uses the TakeDamage-function
	FVector HelpVector = GetActorLocation();
	HelpVector.Z += 10;
	SetActorLocation(HelpVector);
}
