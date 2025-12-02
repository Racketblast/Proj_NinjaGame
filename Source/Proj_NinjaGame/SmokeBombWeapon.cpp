// Fill out your copyright notice in the Description page of Project Settings.


#include "SmokeBombWeapon.h"
#include "StealthCharacter.h"

ASmokeBombWeapon::ASmokeBombWeapon()
{
}

void ASmokeBombWeapon::Throw(AStealthCharacter* Player)
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

void ASmokeBombWeapon::Drop(AStealthCharacter* Player)
{
}
