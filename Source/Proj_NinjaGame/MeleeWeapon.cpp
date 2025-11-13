// Fill out your copyright notice in the Description page of Project Settings.


#include "MeleeWeapon.h"

#include "MeleeEnemy.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AMeleeWeapon::AMeleeWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	RootComponent = StaticMeshComponent;
	StartOfBladePos = CreateDefaultSubobject<USceneComponent>(TEXT("StartOfBladePos"));
	EndOfBladePos = CreateDefaultSubobject<USceneComponent>(TEXT("EndOfBladePos"));
	StartOfBladePos->SetupAttachment(StaticMeshComponent);
	EndOfBladePos->SetupAttachment(StaticMeshComponent);
	
}

void AMeleeWeapon::StartMeleeAttack()
{
	if (SwingSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), SwingSound, GetActorLocation());
	}
	GetWorld()->GetTimerManager().SetTimer(MeleeAttackingTimer, this, &AMeleeWeapon::MeleeAttackLoop, 0.01, true);
}

void AMeleeWeapon::MeleeAttackLoop()
{
	//DrawDebugLine(GetWorld(), StartOfBladePos->GetComponentLocation(), EndOfBladePos->GetComponentLocation(), FColor::Red, true);

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	Params.AddIgnoredActors(ActorsHit);

	if (GetWorld()->LineTraceSingleByChannel(HitResult, StartOfBladePos->GetComponentLocation(), EndOfBladePos->GetComponentLocation(), ECC_Camera, Params))
	{
		Params.AddIgnoredActor(HitResult.GetActor());
		if (AMeleeEnemy* Enemy = Cast<AMeleeEnemy>(HitResult.GetActor()))
		{
			if (Enemy->bCanBeAssassinated && !Enemy->CanSeePlayer())
			{
				if (HitSound)
				{
					UGameplayStatics::PlaySoundAtLocation(GetWorld(), HitSound, GetActorLocation());
				}
				ActorsHit.Add(Enemy);
				UGameplayStatics::ApplyDamage(
					Enemy,
					Enemy->GetHealth(),
					UGameplayStatics::GetPlayerController(this,0),
					UGameplayStatics::GetPlayerCharacter(this,0),
					UDamageType::StaticClass()
					);
			}
			else
			{
				
				ActorsHit.Add(Enemy);
				UGameplayStatics::ApplyDamage(
					Enemy,
					MeleeDamage,
					UGameplayStatics::GetPlayerController(this,0),
					UGameplayStatics::GetPlayerCharacter(this,0),
					UDamageType::StaticClass()
					);
			}
		}
	}
}

void AMeleeWeapon::MeleeAttackEnd()
{
	GetWorld()->GetTimerManager().ClearTimer(MeleeAttackingTimer);
	ActorsHit={};
	bCanMeleeAttack = true;
	bMeleeAttacking = false;
}

// Called when the game starts or when spawned
void AMeleeWeapon::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AMeleeWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

