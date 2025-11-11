// Fill out your copyright notice in the Description page of Project Settings.


#include "PickupWeaponObject.h"

#include "StealthCharacter.h"
#include "ThrowableWeapon.h"

APickupWeaponObject::APickupWeaponObject()
{
}

void APickupWeaponObject::Use_Implementation(AStealthCharacter* Player)
{
	Super::Use_Implementation(Player);

	if (!Player->LastHeldWeapon)
	{
		if (ThrowableWeapon)
		{
			if (!Player->HeldThrowableWeapon)
			{
			Player->HeldThrowableWeapon = GetWorld()->SpawnActor<AThrowableWeapon>(ThrowableWeapon);
			Player->HeldThrowableWeapon->AttachToComponent(Player->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("HandGrip_R"));
			}
			Player->LastHeldWeapon = ThrowableWeapon;
		}
		
		Destroy();
	}
}

void APickupWeaponObject::BeginPlay()
{
	Super::BeginPlay();
}
