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
	
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	RootComponent = StaticMeshComponent;
}

void AThrowableWeapon::Throw(AStealthCharacter* Player)
{
	FVector SpawnLocation = Player->FirstPersonCameraComponent->GetComponentLocation() + Player->FirstPersonCameraComponent->GetForwardVector() * Player->CameraForwardMultiplier;
	FRotator SpawnRotation =  Player->FirstPersonCameraComponent->GetComponentRotation();
	if (ThrownWeaponObject)
	{
		AThrowableObject* ThrownObject = GetWorld()->SpawnActor<AThrowableObject>(ThrownWeaponObject, SpawnLocation, SpawnRotation);
		ThrownObject->Thrown = true;
		ThrownObject->bBreaksOnImpact = bBreakOnImpact;
		ThrownObject->DealtDamage = ThrowDamage;
		ThrownObject->ThrowVelocity = Player->FirstPersonCameraComponent->GetForwardVector() * ThrowSpeed;
		ThrownObject->StaticMeshComponent->SetPhysicsLinearVelocity(ThrownObject->ThrowVelocity, false);
	}

	if (Player->AmountOfKunai > 0)
	{
		Player->HeldThrowableWeapon = GetWorld()->SpawnActor<AThrowableWeapon>(Player->KunaiWeapon);
		Player->HeldThrowableWeapon->AttachToComponent(Player->FirstPersonMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("HandGrip_L"));
	}
	else
	{
		Player->HeldThrowableWeapon = nullptr;
		Player->LastHeldWeapon = nullptr;
		Player->AimEnd();
	}
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

