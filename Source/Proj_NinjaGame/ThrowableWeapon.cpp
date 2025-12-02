// Fill out your copyright notice in the Description page of Project Settings.


#include "ThrowableWeapon.h"

#include "StealthCharacter.h"
#include "ThrowableObject.h"
#include "KunaiWeapon.h"
#include "Camera/CameraComponent.h"

// Sets default values
AThrowableWeapon::AThrowableWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SceneRootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	RootComponent = SceneRootComponent;
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	StaticMeshComponent->SetupAttachment(SceneRootComponent);
	StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	StaticMeshComponent->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::FirstPerson;
}

void AThrowableWeapon::Throw(AStealthCharacter* Player)
{
	ThrowObjectLogic(Player);

	if (Player->AmountOfOwnWeapon > 0)
	{
		Player->HeldThrowableWeapon = GetWorld()->SpawnActor<AThrowableWeapon>(Player->CurrentOwnThrowWeapon);
		Player->HeldThrowableWeapon->AttachToComponent(Player->FirstPersonMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("HandGrip_L"));
		Player->UpdateSpawnMarkerMesh();
	}
	else
	{
		Player->HeldThrowableWeapon = nullptr;
		Player->AimEnd();
	}
	Player->LastHeldWeapon = nullptr;
	Destroy();
}

void AThrowableWeapon::ThrowObjectLogic(AStealthCharacter* Player)
{
	FVector Start = Player->FirstPersonCameraComponent->GetComponentLocation();
	FVector End = Player->FirstPersonCameraComponent->GetComponentLocation() + Player->FirstPersonCameraComponent->GetForwardVector() * Player->CameraForwardMultiplier;

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Player);
	Params.AddIgnoredActor(this);

	if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, Params))
	{
		End = Player->FirstPersonCameraComponent->GetComponentLocation();
	}
	
	FVector SpawnLocation = End;
	FRotator SpawnRotation =  Player->FirstPersonCameraComponent->GetComponentRotation();
	if (ThrownWeaponObject)
	{
		AThrowableObject* ThrownObject = GetWorld()->SpawnActor<AThrowableObject>(ThrownWeaponObject, SpawnLocation, SpawnRotation);
		ThrownObject->Thrown = true;
		ThrownObject->bBreaksOnImpact = bBreakOnImpact;
		ThrownObject->DealtDamage = ThrowDamage;
		ThrownObject->ThrowVelocity = Player->FirstPersonCameraComponent->GetForwardVector() * ThrowSpeed;

		ThrownObject->StaticMeshComponent->SetCollisionResponseToChannel(TRACE_CHANNEL_INTERACT,ECR_Ignore);
		ThrownObject->StaticMeshComponent->SetSimulatePhysics(true);
		ThrownObject->StaticMeshComponent->SetNotifyRigidBodyCollision(true);
		ThrownObject->StaticMeshComponent->SetCanEverAffectNavigation(false);
		ThrownObject->StaticMeshComponent->SetUseCCD(true);
		
		ThrownObject->StaticMeshComponent->SetPhysicsLinearVelocity(ThrownObject->ThrowVelocity, false);
	}
}

void AThrowableWeapon::Drop(AStealthCharacter* Player)
{
	FVector SpawnLocation = Player->FirstPersonCameraComponent->GetComponentLocation() + Player->FirstPersonCameraComponent->GetForwardVector() * Player->CameraForwardMultiplier;
	FRotator SpawnRotation =  Player->FirstPersonCameraComponent->GetComponentRotation();
	if (ThrownWeaponObject)
	{
		AThrowableObject* ThrownObject = GetWorld()->SpawnActor<AThrowableObject>(ThrownWeaponObject, SpawnLocation, SpawnRotation);
		
		ThrownObject->StaticMeshComponent->SetSimulatePhysics(true);
		ThrownObject->StaticMeshComponent->SetCanEverAffectNavigation(false);
		ThrownObject->StaticMeshComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	}
	if (Player->AmountOfOwnWeapon > 0)
	{
		Player->HeldThrowableWeapon = GetWorld()->SpawnActor<AThrowableWeapon>(Player->CurrentOwnThrowWeapon);
		Player->HeldThrowableWeapon->AttachToComponent(Player->FirstPersonMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("HandGrip_L"));
	}
	else
	{
		Player->HeldThrowableWeapon = nullptr;
	}
	
	Player->LastHeldWeapon = nullptr;
	Destroy();
}

// Called when the game starts or when spawned
void AThrowableWeapon::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AThrowableWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

