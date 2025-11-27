// Fill out your copyright notice in the Description page of Project Settings.


#include "MeleeWeapon.h"

#include "BreakableObject.h"
#include "MeleeEnemy.h"
#include "SecurityCamera.h"
#include "SoundUtility.h"
#include "StealthCharacter.h"
#include "Components/BoxComponent.h"
#include "Field/FieldSystemActor.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AMeleeWeapon::AMeleeWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	RootComponent = StaticMeshComponent;
	StaticMeshComponent->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::FirstPerson;
}

void AMeleeWeapon::StartMeleeAttack()
{
	SpawnFieldActor();
	
	TArray<AActor*> HitActors;
	Player->PlayerMeleeBox->GetOverlappingActors(HitActors);
	for (auto HitActor : HitActors)
	{
		if (AMeleeEnemy* Enemy = Cast<AMeleeEnemy>(HitActor))
		{
			if (HitSound)
			{
				UGameplayStatics::PlaySoundAtLocation(GetWorld(), HitSound, GetActorLocation());
			}
			UGameplayStatics::ApplyDamage(
				Enemy,
				MeleeDamage,
				UGameplayStatics::GetPlayerController(this,0),
				UGameplayStatics::GetPlayerCharacter(this,0),
				UDamageType::StaticClass()
				);
				
			//Sound för fienden
			float NoiseLevel = 4.0f;

			USoundUtility::ReportNoise(GetWorld(), GetActorLocation(), NoiseLevel);
		}
		else if (ASecurityCamera* Camera = Cast<ASecurityCamera>(HitActor))
		{
			
			if (HitSound)
			{
				UGameplayStatics::PlaySoundAtLocation(GetWorld(), HitSound, GetActorLocation());
			}
			UGameplayStatics::ApplyDamage(
				Camera,
				MeleeDamage,
				UGameplayStatics::GetPlayerController(this,0),
				UGameplayStatics::GetPlayerCharacter(this,0),
				UDamageType::StaticClass()
				);
				
			//Sound för fienden
			float NoiseLevel = 4.0f;

			USoundUtility::ReportNoise(GetWorld(), GetActorLocation(), NoiseLevel);
		}
		else if (ABreakableObject* BreakableObject = Cast<ABreakableObject>(HitActor))
		{
			BreakableObject->BreakObject();
		}
	}
}

void AMeleeWeapon::AssassinateEnemy()
{	
	TArray<AActor*> HitActors;
	Player->PlayerMeleeBox->GetOverlappingActors(HitActors);
	TArray<AActor*> ThatCanBeStabbed;
	TArray<AMeleeEnemy*> ThatCannotBeStabbed;
	
	for (auto HitActor : HitActors)
	{
		if (AMeleeEnemy* Enemy = Cast<AMeleeEnemy>(HitActor))
		{
			if (Enemy->bCanBeAssassinated && !Enemy->CanSeePlayer())
			{
				ThatCanBeStabbed.Add(Enemy);
			}
			else
			{
				ThatCannotBeStabbed.Add(Enemy);
			}
		}
		else if (ABreakableObject* BreakableObject = Cast<ABreakableObject>(HitActor))
		{
			BreakableObject->BreakObject();
		}
	}

	AMeleeEnemy* AssassinatedEnemy = GetEnemyClosestToCrosshair(ThatCanBeStabbed);
	//Hit an Assassination
	if (AssassinatedEnemy)
	{
		if (HitSound)
		{
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), HitSound, GetActorLocation());
		}
				
		UGameplayStatics::ApplyDamage(
			AssassinatedEnemy,
			AssassinatedEnemy->GetHealth(),
			UGameplayStatics::GetPlayerController(this,0),
			UGameplayStatics::GetPlayerCharacter(this,0),
			UDamageType::StaticClass()
		);
	}
	//Missed an Assassination
	else
	{
		for (auto Enemy : ThatCannotBeStabbed)
		{
			if (HitSound)
			{
				UGameplayStatics::PlaySoundAtLocation(GetWorld(), HitSound, GetActorLocation());
			}
			UGameplayStatics::ApplyDamage(
				Enemy,
				MeleeDamage,
				UGameplayStatics::GetPlayerController(this,0),
				UGameplayStatics::GetPlayerCharacter(this,0),
				UDamageType::StaticClass()
				);
				
			//Sound för fienden
			float NoiseLevel = 4.0f;

			USoundUtility::ReportNoise(GetWorld(), GetActorLocation(), NoiseLevel);
		}
	}
}

void AMeleeWeapon::SpawnFieldActor()
{
	if (!FieldActorClass)
		return;
	
	FieldActor = GetWorld()->SpawnActor<AFieldSystemActor>(
		FieldActorClass,
		Player->PlayerMeleeBox->GetComponentLocation(),
		Player->PlayerMeleeBox->GetComponentRotation()
	);

	FieldActor->AttachToComponent(Player->PlayerMeleeBox, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
}

AMeleeEnemy* AMeleeWeapon::GetEnemyClosestToCrosshair(const TArray<AActor*>& HitActors)
{
	AMeleeEnemy* BestEnemy = nullptr;
	float BestDot = -1.f;

	FVector CamLoc;
	FRotator CamRot;
	UGameplayStatics::GetPlayerController(this,0)->GetPlayerViewPoint(CamLoc, CamRot);
	FVector CamForward = CamRot.Vector();

	for (AActor* HitActor : HitActors)
	{
		AMeleeEnemy* Enemy = Cast<AMeleeEnemy>(HitActor);
		if (!Enemy) continue;

		FVector DirToEnemy = (Enemy->GetActorLocation() - CamLoc).GetSafeNormal();
		float Dot = FVector::DotProduct(CamForward, DirToEnemy);

		if (Dot > BestDot)
		{
			BestDot = Dot;
			BestEnemy = Enemy;
		}
	}

	return BestEnemy;
}

void AMeleeWeapon::MeleeAttackEnd()
{
	GetWorld()->GetTimerManager().ClearTimer(MeleeAttackingTimer);
	if (Player->CurrentInteractState == EPlayerInteractState::Attack)
	{
		Player->CurrentInteractState = EPlayerInteractState::None;
	}
	ActorsHit={};
	bCanMeleeAttack = true;
	bMeleeAttacking = false;
	bAssassinatingEnemy = false;
}

// Called when the game starts or when spawned
void AMeleeWeapon::BeginPlay()
{
	Super::BeginPlay();

	if (AStealthCharacter* StealthPlayer = Cast<AStealthCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(),0)))
	{
		Player = StealthPlayer;
		Player->PlayerMeleeBox->SetBoxExtent({MeleeBoxLength, MeleeBoxWidth, MeleeBoxHeight});
		Player->PlayerMeleeBox->AddRelativeLocation({Player->PlayerMeleeBox->GetRelativeLocation().X + (MeleeBoxLength - Player->PlayerMeleeBox->GetRelativeLocation().X), Player->PlayerMeleeBox->GetRelativeLocation().Y, Player->PlayerMeleeBox->GetRelativeLocation().Z});
	}
}

// Called every frame
void AMeleeWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

