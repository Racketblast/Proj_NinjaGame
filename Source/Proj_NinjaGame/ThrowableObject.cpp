// Fill out your copyright notice in the Description page of Project Settings.


#include "ThrowableObject.h"

#include "MeleeEnemy.h"
#include "StealthCharacter.h"
#include "StealthGameInstance.h"
#include "ThrowableWeapon.h"
#include "SoundUtility.h"
#include "Components/AudioComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/DamageType.h"

AThrowableObject::AThrowableObject()
{
	StaticMeshComponent->OnComponentHit.AddDynamic(this, &AThrowableObject::ThrowableOnComponentHit);
	StaticMeshComponent->SetGenerateOverlapEvents(false);
	StaticMeshComponent->SetNotifyRigidBodyCollision(true);
	StaticMeshComponent->SetSimulatePhysics(true);
}

void AThrowableObject::Use_Implementation(class AStealthCharacter* Player)
{
	IPlayerUseInterface::Use_Implementation(Player);
	
	if (!Player) return;
	
	if (Thrown) return;
	
	HandlePickup(Player);
}

void AThrowableObject::ShowInteractable_Implementation(bool bShow)
{
	IPlayerUseInterface::ShowInteractable_Implementation(bShow);
	
	if (Thrown) return;
	
	StaticMeshComponent->SetRenderCustomDepth(bShow);
	if (UStealthGameInstance* GI = Cast<UStealthGameInstance>(GetGameInstance()))
	{
		if (bShow)
		{
			if (HoverSound)
			{
				UGameplayStatics::PlaySoundAtLocation(GetWorld(), HoverSound, GetActorLocation());
			}
			
			GI->CurrentInteractText = InteractText;
		}
		else
		{
			GI->CurrentInteractText = "";
		}
	}
}

void AThrowableObject::ThrowableOnComponentHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!Thrown)
		return;
	
	Thrown = false;
	ThrowableOnComponentHitFunction(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
}

void AThrowableObject::ThrowableOnComponentHitFunction(UPrimitiveComponent* HitComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (AMeleeEnemy* Enemy = Cast<AMeleeEnemy>(OtherActor))
	{
		UE_LOG(LogTemp, Warning, TEXT("Throwable OnComponentHit %s"), *GetVelocity().GetSafeNormal().ToString());
		UGameplayStatics::ApplyPointDamage(
			Enemy,
			DealtDamage,
			GetVelocity().GetSafeNormal(),
			Hit,
			UGameplayStatics::GetPlayerController(this,0),
			this,
			UDamageType::StaticClass()
		);
		if (ImpactEnemySound)
		{
			
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), ImpactEnemySound, GetActorLocation());
		}
		else if (ImpactGroundSound)
		{
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), ImpactGroundSound, GetActorLocation());
		}
		
		Destroy();
	}
	else
	{
		if (ImpactGroundSound)
		{
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), ImpactGroundSound, GetActorLocation());
		}
		if (bBreaksOnImpact)
		{
			Destroy();
		}
	}

	//Sound för fienden
	float NoiseLevel = 4.0f;

	USoundUtility::ReportNoise(GetWorld(), GetActorLocation(), NoiseLevel);
}

void AThrowableObject::HandlePickup(AStealthCharacter* Player)
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

void AThrowableObject::BeginPlay()
{
	Super::BeginPlay();
}
