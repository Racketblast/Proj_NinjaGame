// Fill out your copyright notice in the Description page of Project Settings.


#include "Door.h"

#include "DoorNavLink.h"
#include "KeyCard.h"
#include "NavModifierComponent.h"
#include "NiagaraComponent.h"
#include "StealthCharacter.h"
#include "StealthGameInstance.h"
#include "Components/AudioComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"

ADoor::ADoor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	DoorMesh = CreateDefaultSubobject<UStaticMeshComponent>("DoorMesh");
	DoorMesh->SetupAttachment(RootComponent);
	DoorNavLinkPos = CreateDefaultSubobject<USceneComponent>("DoorNavLinkPos");
	DoorNavLinkPos->SetupAttachment(DoorMesh);
	DoorSoundComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("DoorSoundComponent"));
	DoorSoundComponent->SetupAttachment(DoorMesh);
	LockSoundComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("LockSoundComponent"));
	LockSoundComponent->SetupAttachment(DoorMesh);
	DoorHitBox = CreateDefaultSubobject<UBoxComponent>(TEXT("DoorHitBox"));
	DoorHitBox->SetupAttachment(DoorMesh);
	SparkleComponent->SetupAttachment(DoorMesh);

	//See how this could work.
	//DoorNavModifierComponent = CreateDefaultSubobject<UNavModifierComponent>(TEXT("DoorMovementComponent"));
	
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
		if (PlayerCanUnlock)
		{
			bOverrideInteractText = false;
			InteractText = DoorOpenText;
			Execute_UpdateShowInteractable(this);
			
			if (UnlockSound && LockSoundComponent)
			{
				LockSoundComponent->SetSound(UnlockSound);
				LockSoundComponent->Play();
			}
			bNeedsToBeUnlocked = false;
			
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
		DoorMesh->SetCanEverAffectNavigation(true);
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
	DoorMesh->SetCanEverAffectNavigation(false);
}

void ADoor::UnlockDoor()
{
	bOverrideInteractText = false;
	InteractText = DoorUnlockText;
	PlayerCanUnlock = true;
	
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
		DoorMesh->SetCanEverAffectNavigation(true);
	}
}

void ADoor::DoorEndOverlap(UPrimitiveComponent* Overlapped, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	//This is used in the blueprint, it does not work unless its in the blueprint for some reason
	ACharacter* Character = Cast<ACharacter>(OtherActor);

	if (Character && Character == BlockingCharacter)
	{
		BlockingCharacter = nullptr;

		bIsMoving = true;
		DoorMesh->SetCanEverAffectNavigation(false);
	}
}

void ADoor::ChangeSparkleBasedOnSize()
{
	if (DoorMesh && DoorMesh->GetStaticMesh())
	{
		FVector LocalOrigin;
		FVector BoxExtent;

		DoorMesh->GetStaticMesh()->GetBounds().GetBox().GetCenterAndExtents(LocalOrigin, BoxExtent);
		BoxExtent = BoxExtent * DoorMesh->GetComponentScale();

		float SpriteSize = FMath::Clamp(BoxExtent.Size() / 4.0f, 10.0f, 30.0f);
		float SpawnRate = FMath::Clamp(BoxExtent.Size() / 20.0f, 2.0f, 6.0f);
		
		if (SparkleComponent)
		{
			SparkleComponent->SetVariableFloat(TEXT("SpriteSize"), SpriteSize);
			SparkleComponent->SetVariableFloat(TEXT("SpawnRate"), SpawnRate);
		}
	}
}
