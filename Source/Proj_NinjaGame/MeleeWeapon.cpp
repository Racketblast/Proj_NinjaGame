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
	UE_LOG(LogTemp, Error, TEXT("Started attack"));
	bCanMeleeAttack = false;
	GetWorld()->GetTimerManager().SetTimer(MeleeAttackingTimer, this, &AMeleeWeapon::MeleeAttackLoop, 0.01, true);
	bMeleeAttacking = true;
}

void AMeleeWeapon::MeleeAttackLoop()
{
	UE_LOG(LogTemp, Error, TEXT("Swinging"));
	DrawDebugLine(GetWorld(), StartOfBladePos->GetComponentLocation(), EndOfBladePos->GetComponentLocation(), FColor::Red, true);

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	Params.AddIgnoredActors(ActorsHit);

	if (GetWorld()->LineTraceSingleByChannel(HitResult, StartOfBladePos->GetComponentLocation(), EndOfBladePos->GetComponentLocation(), ECC_Camera, Params))
	{
		Params.AddIgnoredActor(HitResult.GetActor());
		if (AMeleeEnemy* Enemy = Cast<AMeleeEnemy>(HitResult.GetActor()))
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

void AMeleeWeapon::MeleeAttackEnd()
{
	UE_LOG(LogTemp, Error, TEXT("Ended attack"));
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

