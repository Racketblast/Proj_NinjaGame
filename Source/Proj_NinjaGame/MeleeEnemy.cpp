// Fill out your copyright notice in the Description page of Project Settings.


#include "MeleeEnemy.h"

#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"
#include "MeleeAIController.h"
#include "StealthCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/OverlapResult.h"
#include "GameFramework/CharacterMovementComponent.h"


AMeleeEnemy::AMeleeEnemy()
{
	PrimaryActorTick.bCanEverTick = true;
	
	// Skapa hitbox och fäst vid mesh 
	MeleeHitBox = CreateDefaultSubobject<UBoxComponent>(TEXT("MeleeHitBox"));
	MeleeHitBox->SetupAttachment(GetMesh()); 
	// Positionera hitbox 
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
	if (AssassinationCapsule)
	{
		AssassinationCapsule->OnComponentBeginOverlap.AddDynamic(this, &AMeleeEnemy::OnAssasinationOverlapBegin);
		AssassinationCapsule->OnComponentEndOverlap.AddDynamic(this, &AMeleeEnemy::OnAssasinationOverlapEnd);
	}

	OriginalSuspiciousVisionRange = SuspiciousVisionRange;
	OriginalVisionRange = VisionRange;
	
	OriginalHearingRange = HearingRange;
}

void AMeleeEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CheckChaseProximityDetection();
	
	CheckPlayerVisibility();

	CheckCloseDetection();
}

void AMeleeEnemy::CheckChaseProximityDetection()
{
	Super::CheckChaseProximityDetection();
	if (!bIsChasing) return; 
	if (!PlayerPawn) return;

	AStealthCharacter* StealthPlayer = Cast<AStealthCharacter>(PlayerPawn);
	if (!StealthPlayer) return;
	
	if (StealthPlayer->bIsHiddenFromEnemy) return;

	const float ChaseProximityRadius = 350.f; 
	const float Distance = FVector::Dist(GetActorLocation(), PlayerPawn->GetActorLocation());

	// Debug sphere
	if (bVisionDebug)
	{
		DrawDebugSphere(
			GetWorld(),
			GetActorLocation(),
			ChaseProximityRadius,
			20,
			FColor::Blue,
			false,
			0.1f,
			0,
			2.f
		);
	}

	bPlayerWithinChaseProximity = (Distance <= ChaseProximityRadius);

	// Om spelaren är inom sfären blir dem direkt upptäck
	if (Distance <= ChaseProximityRadius)
	{
		bCanSeePlayer = true;
		UpdateLastSeenPlayerLocation();
		
		if (AMeleeAIController* AI = Cast<AMeleeAIController>(GetController()))
		{
			AI->RefreshChaseTarget();
		}
	}
}

void AMeleeEnemy::CheckCloseDetection()
{
	Super::CheckCloseDetection();
	if (!PlayerPawn) return;
	if (bIsChasing) return;
	AStealthCharacter* StealthPlayer = Cast<AStealthCharacter>(PlayerPawn);
	if (!StealthPlayer) return;
	
	if (StealthPlayer->bIsHiddenFromEnemy) return;

	FVector Forward = GetActorForwardVector();
	FVector BoxCenter = GetActorLocation() + FVector(0,0,40) + Forward * 130.f; // flytta boxen framåt

	FVector BoxHalfSize = FVector(80.f, 50.f, 120.f); // längd, bredd, höjd

	
	// Rotera boxen så den matchar fiendens rotation
	FQuat BoxRotation = GetActorRotation().Quaternion();
	FCollisionShape Box = FCollisionShape::MakeBox(BoxHalfSize);

	bool bHit = GetWorld()->OverlapBlockingTestByChannel(
		BoxCenter,
		BoxRotation,
		ECC_Pawn,
		Box,
		FCollisionQueryParams(FName(), false, this)
	);

	// Debug
	if (bVisionDebug)
	{
		DrawDebugBox(GetWorld(), BoxCenter, BoxHalfSize, BoxRotation, FColor::Purple, false, 0.05f, 0, 0.f); 
	}

	if (bHit)
	{
		// kontrollera att spelaren faktiskt är i boxen
		TArray<FOverlapResult> Results;
		bool bOverlap = GetWorld()->OverlapMultiByObjectType(
			Results,
			BoxCenter,
			BoxRotation,
			FCollisionObjectQueryParams(ECC_Pawn),
			Box
		);

		for (auto& R : Results)
		{
			if (R.GetActor() == PlayerPawn)
			{
				FVector Start = GetActorLocation() + FVector(0, 0, 65.f);
				FVector End = PlayerPawn->GetActorLocation() + FVector(0, 0, 50.f);

				FHitResult Hit;
				FCollisionQueryParams Params;
				Params.AddIgnoredActor(this);

				bool bLineHit = GetWorld()->LineTraceSingleByChannel(
					Hit,
					Start,
					End,
					ECC_Visibility, 
					Params
				);

				if (bVisionDebug)
				{
					DrawDebugLine(GetWorld(), Start, End, bLineHit ? FColor::Red : FColor::Green, false, 0.05f, 0, 1.f);
				}

				// Kålla om linetracen träffar spelaren 
				if (!bLineHit || Hit.GetActor() == PlayerPawn)
				{
					bCanSeePlayer = true;
					UpdateLastSeenPlayerLocation();
				}

				return;
			}
		}
	}
}

void AMeleeEnemy::CheckPlayerVisibility()
{
	Super::CheckPlayerVisibility();
	
	if (!PlayerPawn) return;
	AStealthCharacter* StealthPlayer = Cast<AStealthCharacter>(PlayerPawn);
	if (!StealthPlayer) return;

	// För hide object
	if (StealthPlayer->bIsHiddenFromEnemy)
	{
		bCanSeePlayer = false;
		bPlayerInSuspiciousZone = false;
		bPlayerInAlertCone = false;
		return;
	}

	//UE_LOG(LogTemp, Warning, TEXT("CheckPlayerVisibility"));

	FVector EnemyEyes = GetActorLocation() + FVector(0, 0, 60);
	FVector PlayerLoc = PlayerPawn->GetActorLocation();
	FVector ToPlayer = PlayerLoc - EnemyEyes;
	float Distance = ToPlayer.Size();
	
	// Variabler 
	FVector EnemyLocation = GetActorLocation() + FVector(0, 0, 50);
	FVector Forward = GetActorForwardVector();
	FVector LookDirection = Forward.RotateAngleAxis(10.f, GetActorRightVector()); // gör konen lite nedåtriktad
	/*FVector ToPlayer = PlayerPawn->GetActorLocation() - EnemyLocation;
	float Distance = ToPlayer.Size();*/
	ToPlayer.Normalize();

	// Skillnad mellan patrull och chase läge 
	float EffectiveVisionRange = bIsChasing ? VisionRange : VisionRange * 0.6f;
	float EffectiveVisionAngle = bIsChasing ? VisionAngle : VisionAngle * 0.5f;

	// Rita debug för fiendens synfält, alltså konerna
	if (!bIsChasing && bVisionDebug)
	{
		// Vanliga syn-kon
		DrawDebugCone(
			GetWorld(),
			EnemyLocation,
			LookDirection,
			EffectiveVisionRange,
			FMath::DegreesToRadians(EffectiveVisionAngle * 0.5f),
			FMath::DegreesToRadians(EffectiveVisionAngle * 0.5f),
			12,
			FColor::Yellow,
			false,
			0.05f
		);

		// Suspicious-kon 
		DrawDebugCone(
			GetWorld(),
			EnemyLocation,
			LookDirection,
			SuspiciousVisionRange,
			FMath::DegreesToRadians(SuspiciousVisionAngle * 0.5f),
			FMath::DegreesToRadians(SuspiciousVisionAngle * 0.5f),
			12,
			FColor::Orange,
			false,
			0.05f
		);
	}

	// Kontrollera om spelaren är inom räckvidd
	if (Distance <= EffectiveVisionRange)
	{
		float Dot = FVector::DotProduct(LookDirection, ToPlayer);
		float Angle = FMath::Acos(Dot) * (180.f / PI);

		if (Angle <= EffectiveVisionAngle * 0.5f)
		{
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
				bIsSuspicious = false;
				SuspiciousTimer = 0.f;
				UpdateLastSeenPlayerLocation();

				DrawDebugLine(GetWorld(), EnemyLocation, PlayerPawn->GetActorLocation(), FColor::Green, false, 0.1f);
				return;
			}
		}
	}

	// Andra konen /  misstanke kon
	if (Distance <= SuspiciousVisionRange)
	{
		//UE_LOG(LogTemp, Warning, TEXT("SuspiciousVisionRange 1"));
		float SuspiciousDot = FVector::DotProduct(LookDirection, ToPlayer);
		float SuspiciousAngle = FMath::Acos(SuspiciousDot) * (180.f / PI);

		if (SuspiciousAngle <= SuspiciousVisionAngle * 0.5f)
		{
			//UE_LOG(LogTemp, Warning, TEXT("Suspicious: inom vinkel"));
			
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
				bPlayerInSuspiciousZone = true;
				bIsSuspicious = true;
				SuspiciousTimer += GetWorld()->GetDeltaSeconds();
				bPlayerInAlertCone = true;

				//UE_LOG(LogTemp, Warning, TEXT("SuspiciousVisionRange 2"));

				// Fienden tittar mot spelaren, men ignorerar pitch
				
				FVector ToPlayerFlat = PlayerPawn->GetActorLocation() - GetActorLocation();
				ToPlayerFlat.Z = 0;

				if (!ToPlayerFlat.IsNearlyZero())
				{
					FRotator LookAtRot = ToPlayerFlat.Rotation();
					FRotator NewRotation = FMath::RInterpTo(
						GetActorRotation(),
						LookAtRot,
						GetWorld()->GetDeltaSeconds(),
						2.f
					);
					SetActorRotation(NewRotation);
				}

				// Om spelaren stannar kvar tillräckligt länge så upptäcks spelaren
				if (SuspiciousTimer >= TimeToSpotPlayer)
				{
					//UE_LOG(LogTemp, Warning, TEXT("Upptäck spelaren efter timer!"))
					bCanSeePlayer = true;
					UpdateLastSeenPlayerLocation();
					return;
				}

				return;
			}
		}
	}

	// Om spelaren lämnar den andra konen
	if (bIsSuspicious && !bPlayerInSuspiciousZone)
	{
		if (SuspiciousTimer > 3.f && SuspiciousTimer < TimeToSpotPlayer)
		{
			UE_LOG(LogTemp, Warning, TEXT("SuspiciousLocationDetected"));
			UpdateLastSeenPlayerLocation();
			OnSuspiciousLocationDetected(); 
		}

		bIsSuspicious = false;
		SuspiciousTimer = 0.f;
	}

	if (!bPlayerWithinChaseProximity)
	{
		bCanSeePlayer = false;
	}
	bPlayerInSuspiciousZone = false;
	bPlayerInAlertCone = false;
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
	
	GetWorldTimerManager().SetTimer(AttackCooldownHandle, this, &AMeleeEnemy::ResetAttackCooldown, AttackCooldown, false);
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

void AMeleeEnemy::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	
	GetWorldTimerManager().ClearAllTimersForObject(this);

	if (MeleeHitBox)
	{
		MeleeHitBox->OnComponentBeginOverlap.Clear();
	}
}

// Time handle Funktioner:
void AMeleeEnemy::ResetAttackCooldown()
{
	bCanAttack = true;
}

