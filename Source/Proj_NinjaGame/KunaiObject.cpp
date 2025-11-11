// Fill out your copyright notice in the Description page of Project Settings.


#include "KunaiObject.h"

#include "StealthCharacter.h"
#include "ThrowableWeapon.h"

AKunaiObject::AKunaiObject()
{
}

void AKunaiObject::HandlePickup(class AStealthCharacter* Player)
{
	if (!Player->HeldThrowableWeapon)
	{
		if (ThrowableWeapon)
		{
			Player->HeldThrowableWeapon = GetWorld()->SpawnActor<AThrowableWeapon>(ThrowableWeapon);
			Player->HeldThrowableWeapon->AttachToComponent(Player->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("HandGrip_R"));
		}
	}

	Player->AmountOfKunai++;
	Destroy();
}
