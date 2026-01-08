// Fill out your copyright notice in the Description page of Project Settings.


#include "DoorFacility.h"

#include "DoorNavLink.h"
#include "KeyCard.h"
#include "StealthCharacter.h"
#include "Components/AudioComponent.h"

// Called when the game starts or when spawned
void ADoorFacility::BeginPlay()
{
	Super::BeginPlay();
	ClosedDoorLocation = StaticMeshComponent->GetRelativeLocation();
	
	UpdateDoorVFX();
	UpdateDoorMaterial();

	if (GetWorld())
	{
		if (bNeedsToBeUnlocked)
		{
			bOverrideInteractText = true;
			InteractText = DoorLockedText;
		}
		else
		{
			bOverrideInteractText = false;
			InteractText = DoorOpenText;
			
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = this;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			FVector SpawnLocation = DoorNavLinkPos->GetComponentLocation();
			SpawnLocation.Z = StaticMeshComponent->GetComponentLocation().Z;
			FRotator SpawnRotation = DoorNavLinkPos->GetComponentRotation();
			
			if (DoorNavLinkClass)
			{
				DoorNavLink = GetWorld()->SpawnActor<ADoorNavLink>(
					DoorNavLinkClass,
					SpawnLocation,
					SpawnRotation,
					SpawnParams
				);
			
				DoorNavLink->DoorAttached = this;
			}
		}
	}
}

void ADoorFacility::OpenCloseDoor()
{
	if (DoorOpenSound && DoorSoundComponent)
	{
		DoorSoundComponent->SetSound(DoorOpenSound);
		DoorSoundComponent->Play();
	}
	
	if (bOpen)
	{
		DoorTargetLocation = ClosedDoorLocation;
	}
	else
	{
		DoorTargetLocation = ClosedDoorLocation + OpenDoorLocation;
	}

	bOpen = !bOpen;
	bIsMoving = true;
	DoorMesh->SetCanEverAffectNavigation(false);
}

void ADoorFacility::MoveDoor(float DeltaSeconds)
{
	if (!bIsMoving) return;

	FVector CurrentLocation = StaticMeshComponent->GetRelativeLocation();

	FVector NewLocation = FMath::VInterpConstantTo(
			CurrentLocation,
			DoorTargetLocation,
			DeltaSeconds,
			DoorSpeed
		);

	StaticMeshComponent->SetRelativeLocation(NewLocation);

	if (NewLocation.Equals(DoorTargetLocation, 0.1f))
	{
		StaticMeshComponent->SetRelativeLocation(DoorTargetLocation);
		bIsMoving = false;
		DoorMesh->SetCanEverAffectNavigation(true);
	}
}

void ADoorFacility::DoorBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!bIsMoving) return;

	ACharacter* Character = Cast<ACharacter>(OtherActor);
	if (!Character) return;

	
	if (bOpen)
	{
		DoorTargetLocation = ClosedDoorLocation;
	}
	else
	{
		DoorTargetLocation = ClosedDoorLocation + OpenDoorLocation;
	}

	bOpen = !bOpen;
	bIsMoving = true;
	DoorMesh->SetCanEverAffectNavigation(false);
}
