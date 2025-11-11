// Fill out your copyright notice in the Description page of Project Settings.


#include "ThrowableWeapon.h"

#include "StealthCharacter.h"
#include "ThrowableObject.h"
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
	FVector SpawnLocation = Player->FirstPersonCameraComponent->GetComponentLocation() + Player->FirstPersonCameraComponent->GetForwardVector() * 100.f;
	FRotator SpawnRotation =  Player->FirstPersonCameraComponent->GetComponentRotation();
	if (ThrownWeaponObject)
	{
		AThrowableObject* ThrownObject = GetWorld()->SpawnActor<AThrowableObject>(ThrownWeaponObject, SpawnLocation, SpawnRotation);
		ThrownObject->Thrown = true;
		ThrownObject->bBreaksOnImpact = bBreakOnImpact;
		ThrownObject->DealtDamage = ThrowDamage;
	}
	
	Player->HeldThrowableWeapon = nullptr;
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

