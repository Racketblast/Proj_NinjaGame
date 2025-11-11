// Fill out your copyright notice in the Description page of Project Settings.


#include "KunaiObject.h"

#include "StealthCharacter.h"
#include "ThrowableWeapon.h"
#include "Kismet/GameplayStatics.h"

AKunaiObject::AKunaiObject()
{
}

void AKunaiObject::HandlePickup(class AStealthCharacter* Player)
{
	if (InteractSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), InteractSound, GetActorLocation());
	}
	
	if (!Player->HeldThrowableWeapon)
	{
		if (ThrowableWeapon)
		{
			Player->HeldThrowableWeapon = GetWorld()->SpawnActor<AThrowableWeapon>(ThrowableWeapon);
			Player->HeldThrowableWeapon->AttachToComponent(Player->FirstPersonMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("HandGrip_R"));
		}
	}

	Player->AmountOfKunai++;
	Destroy();
}
