// Fill out your copyright notice in the Description page of Project Settings.


#include "KunaiObject.h"

#include "KunaiWeapon.h"
#include "StealthCharacter.h"
#include "StealthGameInstance.h"
#include "ThrowableWeapon.h"
#include "Kismet/GameplayStatics.h"

AKunaiObject::AKunaiObject()
{
}

void AKunaiObject::HandlePickup(class AStealthCharacter* Player)
{
	if (UStealthGameInstance* GI = Cast<UStealthGameInstance>(UGameplayStatics::GetGameInstance(GetWorld())))
	{
		if (GI->CurrentOwnThrowWeaponEnum != EPlayerOwnThrowWeapon::Kunai)
		{
			if (InteractSound)
			{
				UGameplayStatics::PlaySoundAtLocation(GetWorld(), InteractSound, GetActorLocation());
			}
			
			GI->SwitchOwnWeapon(EPlayerOwnThrowWeapon::Kunai);
			Destroy();
			return;
		}
	}
	if (Player->AmountOfOwnWeapon < Player->MaxAmountOfOwnWeapon)
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
		}

		Player->AmountOfOwnWeapon++;
		Destroy();
	}
}
