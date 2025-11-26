// Fill out your copyright notice in the Description page of Project Settings.


#include "SmokeBombObject.h"

#include "NiagaraComponent.h"
#include "SmokeBombWeapon.h"
#include "StealthCharacter.h"
#include "StealthGameInstance.h"
#include "ThrowableWeapon.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "MeleeAIController.h"
#include "MeleeEnemy.h"

ASmokeBombObject::ASmokeBombObject()
{
	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SphereComp->SetupAttachment(RootComponent);
	SmokeComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("SmokeComponent"));
    SphereComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SmokeComponent->SetupAttachment(SphereComp);
	SmokeComponent->SetAutoActivate(false);
}

void ASmokeBombObject::ThrowableOnComponentHitFunction(UPrimitiveComponent* HitComp, AActor* OtherActor,
                                                       UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	Super::ThrowableOnComponentHitFunction(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);


	SphereComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SmokeComponent->SetAsset(SmokeEffect);
	SmokeComponent->Activate();

	SphereComp->SetGenerateOverlapEvents(true);
	SphereComp->SetCollisionObjectType(ECC_WorldDynamic);
	SphereComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap); 
	SphereComp->UpdateOverlaps();

	// Stun
	TArray<AActor*> OverlappingActors;
	SphereComp->GetOverlappingActors(OverlappingActors);

	for (AActor* Actor : OverlappingActors)
	{
		AMeleeEnemy* Enemy = Cast<AMeleeEnemy>(Actor);
		if (Enemy)
		{
			AMeleeAIController* EnemyController = Cast<AMeleeAIController>(Enemy->GetController());
			if (EnemyController)
			{
				EnemyController->StunEnemy(3.0f, EEnemyState::Chasing);
				//UE_LOG(LogTemp, Warning, TEXT("SmokeBomb stunned: %s"), *Enemy->GetName());
			}
		}
	}
}

void ASmokeBombObject::HandlePickup(class AStealthCharacter* Player)
{
	if (UStealthGameInstance* GI = Cast<UStealthGameInstance>(UGameplayStatics::GetGameInstance(GetWorld())))
	{
		if (GI->CurrentOwnThrowWeaponEnum != EPlayerOwnThrowWeapon::SmokeBomb)
		{
			GI->SwitchOwnWeapon(EPlayerOwnThrowWeapon::SmokeBomb);
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
