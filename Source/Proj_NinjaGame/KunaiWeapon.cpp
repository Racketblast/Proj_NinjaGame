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
	FVector SpawnLocation = Player->FirstPersonCameraComponent->GetComponentLocation() + Player->FirstPersonCameraComponent->GetForwardVector() * Player->CameraForwardMultiplier;
	FRotator SpawnRotation =  Player->FirstPersonCameraComponent->GetComponentRotation();
	if (ThrownWeaponObject)
	{
		AThrowableObject* ThrownObject = GetWorld()->SpawnActor<AThrowableObject>(ThrownWeaponObject, SpawnLocation, SpawnRotation);
		ThrownObject->Thrown = true;
		ThrownObject->bBreaksOnImpact = bBreakOnImpact;
		ThrownObject->ThrowVelocity = Player->FirstPersonCameraComponent->GetForwardVector() * ThrowSpeed + Player->GetVelocity();
		ThrownObject->StaticMeshComponent->SetPhysicsLinearVelocity(ThrownObject->ThrowVelocity, false);
	}

	Player->AmountOfKunai--;
	if (Player->AmountOfKunai <= 0)
	{
		UE_LOG(LogTemp, Display, TEXT("No Kunai"));
		if (Player->LastHeldWeapon != nullptr)
		{
			Player->HeldThrowableWeapon = GetWorld()->SpawnActor<AThrowableWeapon>(Player->LastHeldWeapon);
			Player->HeldThrowableWeapon->AttachToComponent(Player->FirstPersonMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("HandGrip_R"));
			Destroy();
		}
		else
		{
			Player->HeldThrowableWeapon = nullptr;
			Player->AimEnd();
			Destroy();
		}
	}
}
