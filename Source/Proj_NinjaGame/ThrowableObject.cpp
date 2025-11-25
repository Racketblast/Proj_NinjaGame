// Fill out your copyright notice in the Description page of Project Settings.


#include "ThrowableObject.h"

#include "MeleeEnemy.h"
#include "SecurityCamera.h"
#include "StealthCharacter.h"
#include "StealthGameInstance.h"
#include "ThrowableWeapon.h"
#include "SoundUtility.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/DamageType.h"
#include "Field/FieldSystemActor.h"
#include "GeometryCollection/GeometryCollectionComponent.h"

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

void AThrowableObject::SpawnFieldActor()
{
	if (!FieldActorClass)
		return;
	
	FieldActor = GetWorld()->SpawnActor<AFieldSystemActor>(
		FieldActorClass,
		GetActorLocation(),
		GetActorRotation()
	);
}

void AThrowableObject::ThrowableOnComponentHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
                                               UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!Thrown)
		return;
	
	Thrown = false;
	StaticMeshComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	SpawnFieldActor();
	
	ThrowableOnComponentHitFunction(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
}

void AThrowableObject::ThrowableOnComponentHitFunction(UPrimitiveComponent* HitComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (AMeleeEnemy* Enemy = Cast<AMeleeEnemy>(OtherActor))
	{
		if (!Enemy->GetIsDead())
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
			
				UGameplayStatics::PlaySoundAtLocation(GetWorld(), ImpactEnemySound, GetActorLocation(), 1, 1,0, ThrowableAttenuation);
			}
			else if (ImpactGroundSound)
			{
				UGameplayStatics::PlaySoundAtLocation(GetWorld(), ImpactGroundSound, GetActorLocation(), 1, 1,0, ThrowableAttenuation);
			}
		
			DestroyObject();
		}
	}
	else if (ASecurityCamera* Camera = Cast<ASecurityCamera>(OtherActor)) 
	{
		if (!Camera->GetIsDead())
		{
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
			
				UGameplayStatics::PlaySoundAtLocation(GetWorld(), ImpactEnemySound, GetActorLocation(), 1, 1,0, ThrowableAttenuation);
			}
			else if (ImpactGroundSound)
			{
				UGameplayStatics::PlaySoundAtLocation(GetWorld(), ImpactGroundSound, GetActorLocation(), 1, 1,0, ThrowableAttenuation);
			}
		
			DestroyObject();
		}
	}
	else
	{
		if (ImpactGroundSound)
		{
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), ImpactGroundSound, GetActorLocation(), 1, 1,0, ThrowableAttenuation);
		}
		if (bBreaksOnImpact)
		{
			DestroyObject();
		}
	}

	//Sound för fienden
	float NoiseLevel = 4.0f;

	USoundUtility::ReportNoise(GetWorld(), Hit.ImpactPoint, NoiseLevel, this);
}

void AThrowableObject::HandlePickup(AStealthCharacter* Player)
{
	UE_LOG(LogTemp, Warning, TEXT("Pickup"));

	//Drop item if I have something in my inventory
	if (Player->LastHeldWeapon)
	{
		Player->HeldThrowableWeapon->Destroy();
		AThrowableWeapon* DropWeapon = GetWorld()->SpawnActor<AThrowableWeapon>(Player->LastHeldWeapon);
		DropWeapon->Drop(Player);
	}
	
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

void AThrowableObject::BeginPlay()
{
	Super::BeginPlay();
}

void AThrowableObject::DestroyObject()
{
	StaticMeshComponent->SetStaticMesh(nullptr);
	SetLifeSpan(10);
	
	if (ImpactDebris)
	{
		UGeometryCollectionComponent* GeoComp =
		NewObject<UGeometryCollectionComponent>(this, UGeometryCollectionComponent::StaticClass());

		if (GeoComp)
		{
			FTransform MeshTransform = StaticMeshComponent->GetComponentTransform();
			
			GeoComp->SetupAttachment(GetRootComponent());
			GeoComp->SetWorldTransform(MeshTransform);
			GeoComp->RegisterComponent();

			GeoComp->SetRelativeTransform(FTransform::Identity);

			GeoComp->SetRestCollection(ImpactDebris);

			GeoComp->SetCollisionProfileName(TEXT("Player"));
			GeoComp->SetPerLevelCollisionProfileNames({"None","Debris","Debris"});
		}
	}
}