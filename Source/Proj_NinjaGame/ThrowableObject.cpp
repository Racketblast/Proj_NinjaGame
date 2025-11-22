// Fill out your copyright notice in the Description page of Project Settings.


#include "ThrowableObject.h"

#include "MeleeEnemy.h"
#include "SecurityCamera.h"
#include "StealthCharacter.h"
#include "StealthGameInstance.h"
#include "ThrowableWeapon.h"
#include "SoundUtility.h"
#include "Components/AudioComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/DamageType.h"

AThrowableObject::AThrowableObject()
{
	StaticMeshComponent->OnComponentHit.AddDynamic(this, &AThrowableObject::ThrowableOnComponentHit);
	StaticMeshComponent->SetGenerateOverlapEvents(false);
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
	StaticMeshComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	
	ThrowableOnComponentHitFunction(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
}

void AThrowableObject::ThrowableOnComponentHitFunction(UPrimitiveComponent* HitComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (AMeleeEnemy* Enemy = Cast<AMeleeEnemy>(OtherActor))
	{
		if (Hit.Component == Enemy->GetHeadComponent())
		{
			UGameplayStatics::ApplyPointDamage(
				Enemy,
				Enemy->GetHealth(),
				GetVelocity().GetSafeNormal(),
				Hit,
				UGameplayStatics::GetPlayerController(this,0),
				this,
				UDamageType::StaticClass()
			);
		}
		else
		{
			UGameplayStatics::ApplyPointDamage(
				Enemy,
				DealtDamage,
				GetVelocity().GetSafeNormal(),
				Hit,
				UGameplayStatics::GetPlayerController(this,0),
				this,
				UDamageType::StaticClass()
			);
		}
		
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
	else if (ASecurityCamera* Camera = Cast<ASecurityCamera>(OtherActor)) 
	{
		UE_LOG(LogTemp, Warning, TEXT("Throwable hit Camera"));
        
		UGameplayStatics::ApplyPointDamage(
			Camera,
			DealtDamage,
			GetVelocity().GetSafeNormal(),
			Hit,
			UGameplayStatics::GetPlayerController(this, 0),
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

	USoundUtility::ReportNoise(GetWorld(), Hit.ImpactPoint, NoiseLevel, this);
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
