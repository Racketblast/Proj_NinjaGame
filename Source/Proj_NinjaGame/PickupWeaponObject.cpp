// Fill out your copyright notice in the Description page of Project Settings.


#include "PickupWeaponObject.h"

#include "StealthCharacter.h"
#include "ThrowableWeapon.h"
#include "Kismet/GameplayStatics.h"

APickupWeaponObject::APickupWeaponObject()
{
}

void APickupWeaponObject::Use_Implementation(AStealthCharacter* Player)
{
	if (!Player->LastHeldWeapon)
	{
		if (InteractSound)
		{
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), InteractSound, GetActorLocation());
		}
		if (ThrowableWeapon)
		{
			if (Player->HeldThrowableWeapon)
			{
				Player->HeldThrowableWeapon->Destroy();
			}
			Player->HeldThrowableWeapon = GetWorld()->SpawnActor<AThrowableWeapon>(ThrowableWeapon);
			Player->HeldThrowableWeapon->AttachToComponent(Player->FirstPersonMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("HandGrip_L"));
			Player->LastHeldWeapon = ThrowableWeapon;
		}
		
		Destroy();
	}
}

void APickupWeaponObject::BeginPlay()
{
	Super::BeginPlay();
}
