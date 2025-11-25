// Fill out your copyright notice in the Description page of Project Settings.


#include "KunaiWeapon.h"
#include "StealthCharacter.h"
#include "ThrowableObject.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"

AKunaiWeapon::AKunaiWeapon()
{
}

void AKunaiWeapon::Throw(AStealthCharacter* Player)
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
		ThrownObject->ThrowVelocity = Player->FirstPersonCameraComponent->GetForwardVector() * ThrowSpeed;
		
		ThrownObject->StaticMeshComponent->SetSimulatePhysics(true);
		ThrownObject->StaticMeshComponent->SetNotifyRigidBodyCollision(true);
		ThrownObject->StaticMeshComponent->SetCanEverAffectNavigation(false);
		
		ThrownObject->StaticMeshComponent->SetPhysicsLinearVelocity(ThrownObject->ThrowVelocity, false);
	}

	Player->AmountOfKunai--;
	if (Player->AmountOfKunai <= 0)
	{
		UE_LOG(LogTemp, Display, TEXT("No Kunai"));
		if (Player->LastHeldWeapon != nullptr)
		{
			Player->HeldThrowableWeapon = GetWorld()->SpawnActor<AThrowableWeapon>(Player->LastHeldWeapon);
			Player->HeldThrowableWeapon->AttachToComponent(Player->FirstPersonMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("HandGrip_L"));
		}
		else
		{
			Player->HeldThrowableWeapon = nullptr;
			Player->AimEnd();
		}
		Destroy();
	}
}
