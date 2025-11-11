// Fill out your copyright notice in the Description page of Project Settings.


#include "KunaiWeapon.h"
#include "StealthCharacter.h"
#include "ThrowableObject.h"
#include "Camera/CameraComponent.h"

AKunaiWeapon::AKunaiWeapon()
{
}

void AKunaiWeapon::Throw(AStealthCharacter* Player)
{
	FVector SpawnLocation = Player->FirstPersonCameraComponent->GetComponentLocation() + Player->FirstPersonCameraComponent->GetForwardVector() * 100.f;
	FRotator SpawnRotation =  Player->FirstPersonCameraComponent->GetComponentRotation();
	if (ThrownWeaponObject)
	{
		AThrowableObject* ThrownObject = GetWorld()->SpawnActor<AThrowableObject>(ThrownWeaponObject, SpawnLocation, SpawnRotation);
		ThrownObject->Thrown = true;
		ThrownObject->bBreaksOnImpact = bBreakOnImpact;
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
			Destroy();
		}
	}
}
