// Fill out your copyright notice in the Description page of Project Settings.


#include "Door.h"

#include "StealthCharacter.h"
#include "Components/AudioComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"

ADoor::ADoor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	DoorMesh = CreateDefaultSubobject<UStaticMeshComponent>("DoorMesh");
	DoorMesh->SetupAttachment(RootComponent);
	DoorSoundComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("DoorSoundComponent"));
	DoorSoundComponent->SetupAttachment(DoorMesh);
	LockSoundComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("LockSoundComponent"));
	LockSoundComponent->SetupAttachment(DoorMesh);
	DoorHitBox = CreateDefaultSubobject<UBoxComponent>(TEXT("DoorHitBox"));
	DoorHitBox->SetupAttachment(DoorMesh);

	DoorHitBox->OnComponentBeginOverlap.AddDynamic(this, &ADoor::DoorBeginOverlap);
	//Does not work for some reason
	//DoorHitBox->OnComponentEndOverlap.AddDynamic(this, &ADoor::DoorEndOverlap);
}

void ADoor::Use_Implementation(class AStealthCharacter* Player)
{
	Super::Use_Implementation(Player);
	
	if (!Player) return;

	if (bNeedsToBeUnlocked)
	{
		if (Player->DoorsThatCanBeUnlocked.Contains(this))
		{
			if (UnlockSound && LockSoundComponent)
			{
				LockSoundComponent->SetSound(UnlockSound);
				LockSoundComponent->Play();
			}
			bNeedsToBeUnlocked = false;
			OpenCloseDoor();
		}
		else
		{
			if (LockedSound && LockSoundComponent)
			{
				LockSoundComponent->SetSound(LockedSound);
				LockSoundComponent->Play();
			}
		}
	}
	else
	{
		OpenCloseDoor();
	}
}

void ADoor::BeginPlay()
{
	Super::BeginPlay();
    ClosedDoorRotation = StaticMeshComponent->GetRelativeRotation();
}

void ADoor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	if (!bIsMoving) return;

	FRotator CurrentRotation = StaticMeshComponent->GetRelativeRotation();

	FRotator NewRotation = FMath::RInterpConstantTo(
		CurrentRotation,
		DoorTargetRotation,
		DeltaSeconds,
		DoorSpeed
	);

	StaticMeshComponent->SetRelativeRotation(NewRotation);

	if (NewRotation.Equals(DoorTargetRotation, 0.1f))
	{
		StaticMeshComponent->SetRelativeRotation(DoorTargetRotation);
		bIsMoving = false;
	}
}

void ADoor::OpenCloseDoor()
{
	if (DoorOpenSound && DoorSoundComponent)
	{
		DoorSoundComponent->SetSound(DoorOpenSound);
		DoorSoundComponent->Play();
	}
	
	if (bOpen)
	{
		DoorTargetRotation = ClosedDoorRotation;
	}
	else
	{
        DoorTargetRotation = ClosedDoorRotation + OpenDoorRotation;
	}

	bOpen = !bOpen;
	bIsMoving = true;
}


bool ADoor::CanPushCharacter(ACharacter* Character, FVector PushDir, float PushDistance)
{
	if (!Character) return true;

	UCapsuleComponent* Capsule = Character->GetCapsuleComponent();
	if (!Capsule) return true;

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Character);
	Params.AddIgnoredActor(this);

	FVector Start = Capsule->GetComponentLocation();
	FVector End = Start + (PushDir * PushDistance);

	FHitResult Hit;
	bool bHit = Capsule->GetWorld()->SweepSingleByChannel(
		Hit,
		Start,
		End,
		Character->GetActorQuat(),
		ECC_Pawn,
		FCollisionShape::MakeCapsule(
			Capsule->GetUnscaledCapsuleRadius(),
			Capsule->GetUnscaledCapsuleHalfHeight()
		),
		Params
	);

	// Only block if the sweep hits a *blocking hit*, not overlap.
	return !bHit || !Hit.bBlockingHit;
}

void ADoor::DoorBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!bIsMoving) return;

	ACharacter* Character = Cast<ACharacter>(OtherActor);
	if (!Character) return;

	FVector HingeLocation = StaticMeshComponent->GetComponentLocation();

	FVector HingeUp = StaticMeshComponent->GetUpVector();

	FVector ToHit = (Character->GetActorLocation() - HingeLocation).GetSafeNormal();

	FVector PushDirection = FVector::CrossProduct(HingeUp, ToHit).GetSafeNormal();

	if (!bOpen)
	{
		PushDirection *= -1.f;
	}

	float PushStrength = 10.f;
	if (CanPushCharacter(Character, PushDirection, (PushDirection * PushStrength).Length()))
	{
		Character->LaunchCharacter(PushDirection * PushStrength, false, false);
	}
	else
	{
		bIsMoving = false;
		BlockingCharacter = Character;
	}
}

void ADoor::DoorEndOverlap(UPrimitiveComponent* Overlapped, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	//This is used in the blueprint, it does not work unless its in the blueprint for some reason
	ACharacter* Character = Cast<ACharacter>(OtherActor);

	if (Character && Character == BlockingCharacter)
	{
		BlockingCharacter = nullptr;

		// Door was moving before the block; start it again
		bIsMoving = true;
	}
}