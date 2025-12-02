// Fill out your copyright notice in the Description page of Project Settings.


#include "KunaiWeapon.h"
#include "StealthCharacter.h"

AKunaiWeapon::AKunaiWeapon()
{
}

void AKunaiWeapon::Throw(AStealthCharacter* Player)
{
	ThrowObjectLogic(Player);

	Player->AmountOfOwnWeapon--;
	if (Player->AmountOfOwnWeapon <= 0)
	{
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

void AKunaiWeapon::Drop(AStealthCharacter* Player)
{
}
