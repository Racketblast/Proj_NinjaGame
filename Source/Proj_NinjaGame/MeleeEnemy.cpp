// Fill out your copyright notice in the Description page of Project Settings.


#include "MeleeEnemy.h"

#include "AIController.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"
#include "MeleeAIController.h"
#include "Components/BoxComponent.h"
#include "GameFramework/CharacterMovementComponent.h"


AMeleeEnemy::AMeleeEnemy()
{
	PrimaryActorTick.bCanEverTick = true;


	// Skapa hitbox och fäst vid mesh (t.ex. hand-socket) eller root
	MeleeHitBox = CreateDefaultSubobject<UBoxComponent>(TEXT("MeleeHitBox"));
	MeleeHitBox->SetupAttachment(GetMesh()); // eller AttachToComponent(GetMesh(), ...)
	// Positionera hitbox relativt i blueprint eller här via SetRelativeLocation/Rotation
	MeleeHitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeleeHitBox->SetCollisionObjectType(ECC_WorldDynamic);
	MeleeHitBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	MeleeHitBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}


void AMeleeEnemy::BeginPlay()
{
	Super::BeginPlay();
	PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;


	if (MeleeHitBox)
	{
		MeleeHitBox->OnComponentBeginOverlap.AddDynamic(this, &AMeleeEnemy::OnMeleeOverlapBegin);
	}
}

void AMeleeEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	CheckPlayerVisibility();
}

/*void AMeleeEnemy::CheckPlayerVisibility()
{
	if (!PlayerPawn) return;

	FVector ToPlayer = PlayerPawn->GetActorLocation() - GetActorLocation();
	float Distance = ToPlayer.Size();

	if (Distance > VisionRange)
	{
		bCanSeePlayer = false;
		return;
	}

	FVector Forward = GetActorForwardVector();
	ToPlayer.Normalize();

	float Dot = FVector::DotProduct(Forward, ToPlayer);
	float Angle = FMath::Acos(Dot) * (180.f / PI);

	if (Angle > VisionAngle)
	{
		bCanSeePlayer = false;
		return;
	}

	// Line trace check
	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		Hit,
		GetActorLocation() + FVector(0,0,50),
		PlayerPawn->GetActorLocation(),
		ECC_Visibility,
		Params
	);

	if (!bHit || Hit.GetActor() == PlayerPawn)
	{
		bCanSeePlayer = true;
		DrawDebugLine(GetWorld(), GetActorLocation(), PlayerPawn->GetActorLocation(), FColor::Green, false, 0.05f);
		if (bCanSeePlayer)
		{
			UpdateLastSeenPlayerLocation();
		}
	}
	else
	{
		bCanSeePlayer = false;
		DrawDebugLine(GetWorld(), GetActorLocation(), Hit.Location, FColor::Red, false, 0.05f);
	}
}*/

void AMeleeEnemy::CheckPlayerVisibility()
{
    if (!PlayerPawn) return;

	// Rita endast konen när vi patrullerar
	if (!bIsChasing)
	{
		FVector EnemyLocation = GetActorLocation() + FVector(0, 0, 50);
		FVector Forward = GetActorForwardVector();
		FVector LookDirection = Forward.RotateAngleAxis(20.f, GetActorRightVector()); 

		FColor ConeColor = FColor::Yellow;
		DrawDebugCone(
			GetWorld(),
			EnemyLocation,
			LookDirection,
			VisionRange * 0.6f,
			FMath::DegreesToRadians(VisionAngle * 0.5f),
			FMath::DegreesToRadians(VisionAngle * 0.5f),
			12,
			ConeColor,
			false,
			0.1f 
		);
	}

	//UE_LOG(LogTemp, Warning, TEXT("CheckPlayerVisibility"));

    // Skillnad mellan patrull och chase-läge 
    float EffectiveVisionRange = bIsChasing ? VisionRange : VisionRange * 0.6f;
    float EffectiveVisionAngle = bIsChasing ? VisionAngle : VisionAngle * 0.5f;

    FVector EnemyLocation = GetActorLocation() + FVector(0, 0, 50);
    FVector Forward = GetActorForwardVector();

    // Rikta synfältet nedåt 
    FVector LookDirection = Forward.RotateAngleAxis(20.f, GetActorRightVector()); 

    FVector ToPlayer = PlayerPawn->GetActorLocation() - EnemyLocation;
    float Distance = ToPlayer.Size();

    if (Distance > EffectiveVisionRange)
    {
        bCanSeePlayer = false;
        return;
    }

    ToPlayer.Normalize();
    float Dot = FVector::DotProduct(LookDirection, ToPlayer);
    float Angle = FMath::Acos(Dot) * (180.f / PI);

    if (Angle > EffectiveVisionAngle)
    {
        bCanSeePlayer = false;
        return;
    }

    // Line trace för sikt 
    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    bool bHit = GetWorld()->LineTraceSingleByChannel(
        Hit,
        EnemyLocation,
        PlayerPawn->GetActorLocation(),
        ECC_Visibility,
        Params
    );

    if (!bHit || Hit.GetActor() == PlayerPawn)
    {
        bCanSeePlayer = true;
        UpdateLastSeenPlayerLocation();

        DrawDebugLine(GetWorld(), EnemyLocation, PlayerPawn->GetActorLocation(), FColor::Green, false, 0.05f);
    }
    else
    {
        bCanSeePlayer = false;
        DrawDebugLine(GetWorld(), EnemyLocation, Hit.Location, FColor::Red, false, 0.05f);
    }
}


void AMeleeEnemy::UpdateLastSeenPlayerLocation()
{
	if (PlayerPawn)
	{
		LastSeenPlayerLocation = PlayerPawn->GetActorLocation();
	}
}

float AMeleeEnemy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	Health -= ActualDamage;

	if (Health <= 0.0f)
	{
		Die();
	}

	return ActualDamage;
}

void AMeleeEnemy::Die()
{
	UE_LOG(LogTemp, Warning, TEXT("Enemy died!"));
	Destroy(); 
}





void AMeleeEnemy::StartAttack()
{
	if (!bCanAttack) return;

	bCanAttack = false;
	bHitRegisteredThisSwing = false;

	// Spela animation här, KOM IHÅG ATT GÖRA EN ANIMATION SENARE!!!!!!
	if (AttackMontage)
	{
		UAnimInstance* AnimInst = GetMesh()->GetAnimInstance();
		if (AnimInst)
		{
			AnimInst->Montage_Play(AttackMontage);
		}
	}

	// aktiverar hitbox en kort stund 
	EnableHitbox(0.2f);
	
	GetWorldTimerManager().SetTimer(AttackCooldownHandle, [this]()
	{
		bCanAttack = true;
	}, AttackCooldown, false);
}

void AMeleeEnemy::EnableHitbox(float WindowSeconds)
{
	if (!MeleeHitBox) return;

	bHitRegisteredThisSwing = false;
	MeleeHitBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	
	GetWorldTimerManager().SetTimer(HitboxWindowHandle, this, &AMeleeEnemy::DisableHitbox, WindowSeconds, false);
}

void AMeleeEnemy::DisableHitbox()
{
	if (MeleeHitBox)
	{
		MeleeHitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	GetWorldTimerManager().ClearTimer(HitboxWindowHandle);
}

void AMeleeEnemy::OnMeleeOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                                     UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                                     bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == this) return;
	if (bHitRegisteredThisSwing) return; // för att se till att det bara blir en hit per sving 
	
	APawn* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (OtherActor == Player)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Fienden skadade spelaren"));
		bHitRegisteredThisSwing = true;
		ApplyDamageTo(OtherActor);

		// Disable hitbox direkt efter träff för att undvika multi-hits, alltså att fienden skadar spelaren flera gånger när den bara träffar en gång. 
		DisableHitbox();
	}
}

void AMeleeEnemy::ApplyDamageTo(AActor* Target)
{
	if (!Target) return;

	UGameplayStatics::ApplyDamage(Target, AttackDamage, GetController(), this, nullptr);
}



void AMeleeEnemy::HearSoundAtLocation(FVector SoundLocation)
{
	// Kontrollera avstånd
	const float Distance = FVector::Dist(GetActorLocation(), SoundLocation);
	if (Distance <= HearingRange)
	{
		bHeardSoundRecently = true;
		LastHeardSoundLocation = SoundLocation;

		//UE_LOG(LogTemp, Warning, TEXT("Enemy heard sound at %s"), *SoundLocation.ToString());

		// Starta timer för att glömma ljudet efter ett tag
		GetWorldTimerManager().SetTimerForNextTick([this]()
		{
			FTimerHandle ForgetSoundHandle;
			GetWorldTimerManager().SetTimer(ForgetSoundHandle, [this]()
			{
				bHeardSoundRecently = false;
			}, HearingMemoryTime, false);
		});
	}
}

