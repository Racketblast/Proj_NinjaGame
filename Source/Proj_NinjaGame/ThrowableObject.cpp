// Fill out your copyright notice in the Description page of Project Settings.


#include "ThrowableObject.h"

#include "MeleeEnemy.h"
#include "StealthCharacter.h"
#include "StealthGameInstance.h"
#include "ThrowableWeapon.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/DamageType.h"

AThrowableObject::AThrowableObject()
{
	ProjectileComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComponent"));

	StaticMeshComponent->OnComponentHit.AddDynamic(this, &AThrowableObject::ThrowableOnComponentHit);
	StaticMeshComponent->SetGenerateOverlapEvents(false);
	StaticMeshComponent->SetNotifyRigidBodyCollision(true);
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
		UGameplayStatics::ApplyPointDamage(
						Enemy,
						DealtDamage,
						ProjectileComponent->Velocity.GetSafeNormal(),
						Hit,
						UGameplayStatics::GetPlayerController(this,0),
						this,
						UDamageType::StaticClass()
						);
		Destroy();
	}
	else
	{
		if (bBreaksOnImpact)
		{
			Destroy();
		}
	}
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
			if (!Player->HeldThrowableWeapon)
			{
				Player->HeldThrowableWeapon = GetWorld()->SpawnActor<AThrowableWeapon>(ThrowableWeapon);
				Player->HeldThrowableWeapon->AttachToComponent(Player->FirstPersonMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("HandGrip_R"));
			}
			Player->LastHeldWeapon = ThrowableWeapon;
		}
		
		Destroy();
	}
}

void AThrowableObject::BeginPlay()
{
	Super::BeginPlay();
}
