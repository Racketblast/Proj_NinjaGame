// Fill out your copyright notice in the Description page of Project Settings.


#include "MeleeEnemy.h"

#include "AIController.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"
#include "EnemyHandler.h"
#include "MeleeAIController.h"
#include "StealthCharacter.h"
#include "Components/AudioComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/OverlapResult.h"
#include "GameFramework/CharacterMovementComponent.h"


AMeleeEnemy::AMeleeEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	//Audio
	FootstepsAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("FootstepsAudioComponent"));
	FootstepsAudioComponent->SetupAttachment(RootComponent);
	StateAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("StateAudioComponent"));
	StateAudioComponent->SetupAttachment(RootComponent);
	VoiceAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("VoiceAudioComponent"));
	VoiceAudioComponent->SetupAttachment(RootComponent);
	
	StateAudioComponent->bAutoActivate = false; 	// styr ljuden i koden, så detta ska vara false
	VoiceAudioComponent->bAutoActivate = false;

	// Skapa hitbox och fäst vid mesh 
	MeleeHitBox = CreateDefaultSubobject<UBoxComponent>(TEXT("MeleeHitBox"));
	MeleeHitBox->SetupAttachment(GetMesh()); 
	// Positionera hitbox 
	MeleeHitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeleeHitBox->SetCollisionObjectType(ECC_WorldDynamic);
	MeleeHitBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	MeleeHitBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	SkeletalMeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMeshComp"));
	SkeletalMeshComp->SetupAttachment(GetCapsuleComponent());
	
	AssassinationCapsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("AssassinationCapsule"));
	AssassinationCapsule->SetupAttachment(GetMesh());

	HeadCapsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("HeadCapsule"));
	HeadCapsule->SetupAttachment(GetMesh());


	StateVFXComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("StateVFX"));
	StateVFXComponent->SetupAttachment(GetMesh());
	StateVFXComponent->SetRelativeLocation(FVector(0.f, 0.f, 120.f));

	// För att undvika att gå in i andra fiender
	GetCharacterMovement()->bUseRVOAvoidance = true;
	GetCharacterMovement()->AvoidanceWeight = 0.5f;
	GetCharacterMovement()->AvoidanceConsiderationRadius = 300.f;

	// För bättre rotation
	/*GetCharacterMovement()->bUseControllerDesiredRotation = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 1.f, 0.f);*/
	GetCharacterMovement()->bUseControllerDesiredRotation = false;
	GetCharacterMovement()->bOrientRotationToMovement = false;

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

void AMeleeEnemy::FaceRotation(FRotator NewRotation, float DeltaTime)
{
	FVector Vel = GetVelocity();
	Vel.Z = 0;

	if (Vel.Size() > 10.f)
	{
		NewRotation = Vel.Rotation();
	}

	Super::FaceRotation(NewRotation, DeltaTime);
}

void AMeleeEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//CheckImmediateProximityDetection();

	CheckChaseProximityDetection();
	
	CheckPlayerVisibility();

	CheckCloseDetection();

	SpreadAgroToNearbyEnemies();
}

void AMeleeEnemy::CheckImmediateProximityDetection()
{
	if (!PlayerPawn) return;
	
	const float ProximityThreshold = 150.f;

	const float DistanceToPlayer = FVector::Dist(GetActorLocation(), PlayerPawn->GetActorLocation());

	if (DistanceToPlayer > ProximityThreshold)
		return;


	AStealthCharacter* StealthPlayer = Cast<AStealthCharacter>(PlayerPawn);
	if (!StealthPlayer) return;

	if (StealthPlayer->bIsHiddenFromEnemy) return;

	if (StealthPlayer->GetPlayerMovementState() == EPlayerMovementState::Crouch)
		return; // om Spelaren smyger så ska fienden inte autoupptäcka

	//UE_LOG(LogTemp, Warning, TEXT("CheckImmediateProximityDetection"));

	// direkt upptäck spelaren
	bCanSeePlayer = true;
	UpdateLastSeenPlayerLocation();
	
	OnSuspiciousLocation.Broadcast(PlayerPawn->GetActorLocation()); 

	if (bVisionDebug)
	{
		DrawDebugSphere(
		GetWorld(),
		GetActorLocation(),
		ProximityThreshold,
		16,
		FColor::Red,
		false,
		0.1f,
		0,
		2.f
	);
	}
}

void AMeleeEnemy::CheckChaseProximityDetection()
{
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
		if (SuspiciousTimer > 2.f && SuspiciousTimer < TimeToSpotPlayer)
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


void AMeleeEnemy::OnSuspiciousLocationDetected()
{
	UE_LOG(LogTemp, Warning, TEXT("OnSuspiciousLocationDetected()"));
	OnSuspiciousLocation.Broadcast(LastSeenPlayerLocation);
}


void AMeleeEnemy::UpdateLastSeenPlayerLocation()
{
	//UE_LOG(LogTemp, Warning, TEXT("UpdateLastSeenPlayerLocation"));
	if (PlayerPawn)
	{
		LastSeenPlayerLocation = PlayerPawn->GetActorLocation();
	}
}


float AMeleeEnemy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	if (!bIsDead)
	{
		//Health -= ActualDamage;
		float NewHealth = Health - ActualDamage;

		// Kolla om fienden kan överleva attacken 
		bool bWillSurvive = NewHealth > 0.0f;

		// Om fienden kommer överleva och DamageCauser är spelaren så upptäcker fienden spelaren
		if (bWillSurvive)
		{
			if (DamageCauser && DamageCauser->IsA(AStealthCharacter::StaticClass()))
			{
				UE_LOG(LogTemp, Warning, TEXT("Enemy hit by player and survived, Enemy spotted player!"));
				bCanSeePlayer = true;     
			}
		}

		// Uppdatera HP 
		Health = NewHealth;
		

		PlayHurtSound();

		if (Health <= 0.0f)
		{
			Die();
			
			if (DamageCauser)
			{
				FVector Direction = GetCapsuleComponent()->GetComponentLocation() - DamageCauser->GetActorLocation();
				Direction.Normalize();
				UE_LOG(LogTemp, Warning, TEXT("Direction: %s"), *Direction.ToString());
				float ImpulseStrength = 1000.0f;
				SkeletalMeshComp->AddImpulse(Direction * ImpulseStrength, NAME_None, true);
			}
		}
	}

	return ActualDamage;
}

void AMeleeEnemy::Die()
{
	UE_LOG(LogTemp, Warning, TEXT("Enemy died!"));
	bIsDead = true;
	
	SetActorTickEnabled(false);

	// Rensa alla timers 
	GetWorldTimerManager().ClearAllTimersForObject(this);

	// ta bort hitbox-komponenten 
	if (MeleeHitBox)
	{
		MeleeHitBox->OnComponentBeginOverlap.Clear(); 
		MeleeHitBox->DestroyComponent();
		MeleeHitBox = nullptr;
	}

	if (AssassinationCapsule)
	{
		AssassinationCapsule->OnComponentBeginOverlap.Clear(); 
		AssassinationCapsule->DestroyComponent();
		AssassinationCapsule = nullptr;
	}

	// ta bort VFX-komponent
	if (StateVFXComponent)
	{
		StateVFXComponent->Deactivate();
		StateVFXComponent->DestroyComponent();
		StateVFXComponent = nullptr;
	}

	// Ser till att AIControllern inte försöker använda fienden längre
	AController* MyController = GetController();
	if (MyController)
	{
		MyController->UnPossess();
		MyController->Destroy();
	}
	
	if (EnemyHandler)
	{
		if (EnemyHandler->GetAllEnemies().Contains(this))
		{
			EnemyHandler->RemoveEnemy(this);
		}
	}
	
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HeadCapsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	SkeletalMeshComp->SetCollisionObjectType(ECC_PhysicsBody);
	SkeletalMeshComp->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);

	SkeletalMeshComp->SetSimulatePhysics(true);
	
	SetLifeSpan(15.0f);
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

void AMeleeEnemy::OnAssasinationOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (AStealthCharacter* Player = Cast<AStealthCharacter>(OtherActor))
	{
		bCanBeAssassinated = true;
	}
}

void AMeleeEnemy::OnAssasinationOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (AStealthCharacter* Player = Cast<AStealthCharacter>(OtherActor))
	{
		bCanBeAssassinated = false;
	}
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

void AMeleeEnemy::ReduceEnemyRange(bool bShouldReduce)
{
	if (bShouldReduce)
	{
		VisionRange = OriginalVisionRange / 2;
		SuspiciousVisionRange = OriginalSuspiciousVisionRange / 2;
	}
	else
	{
		VisionRange = OriginalVisionRange;
		SuspiciousVisionRange = OriginalSuspiciousVisionRange;
	}
}

void AMeleeEnemy::ReduceEnemyHearingRange(bool bShouldReduce)
{
	if (bShouldReduce)
	{
		HearingRange = OriginalHearingRange / 4;
	}
	else
	{
		HearingRange = OriginalHearingRange;
	}
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
		FTimerHandle ForgetSoundHandle;
		GetWorldTimerManager().SetTimer(ForgetSoundHandle, this, &AMeleeEnemy::ForgetHeardSound, HearingMemoryTime, false);
	}
}

//Spread agro
void AMeleeEnemy::SpreadAgroToNearbyEnemies() 
{
	// Cooldown check
	float TimeNow = GetWorld()->GetTimeSeconds();
	if (TimeNow - LastAgroSpreadTime < AgroSpreadCooldown) return; 

	// Uppdatera timestamp
	LastAgroSpreadTime = TimeNow;
	
    // Hämta denna fiendes AI controller
    AMeleeAIController* MyAI = Cast<AMeleeAIController>(GetController());
    if (!MyAI) return;

    // Endast sprida agro om denna fiende faktiskt jagar
    if (MyAI->GetCurrentState() != EEnemyState::Chasing)
        return;

    UWorld* World = GetWorld();
    if (!World) return;

    TArray<FOverlapResult> Overlaps;
    FCollisionShape Sphere = FCollisionShape::MakeSphere(AgroSpreadRadius);

    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    bool bHit = World->OverlapMultiByChannel(
        Overlaps,
        GetActorLocation(),
        FQuat::Identity,
        ECC_Pawn,
        Sphere,
        Params
    );

    // Debug sphere
    if (bVisionDebug)
    {
        DrawDebugSphere(
            World,
            GetActorLocation(),
            AgroSpreadRadius,
            16,
            FColor::Green,
            false,
            0.1f,
            0,
            2.f
        );
    }

    if (!bHit) return;

    for (const FOverlapResult& Result : Overlaps)
    {
        AMeleeEnemy* OtherEnemy = Cast<AMeleeEnemy>(Result.GetActor());
        if (!OtherEnemy || OtherEnemy == this)
            continue;

        // Hämta andra fiendens AI controller
        AMeleeAIController* OtherAI = Cast<AMeleeAIController>(OtherEnemy->GetController());
        if (!OtherAI) continue;

        // Skip om andra fienden redan jagar
        if (OtherAI->GetCurrentState() == EEnemyState::Chasing)
            continue;

        //UE_LOG(LogTemp, Warning, TEXT("Agro candidate: %s"), *OtherEnemy->GetName());

        // Check line of sight
        if (bUseLineOfSightForAgroSpread)
        {
            FHitResult LineHit;
            FVector Start = GetActorLocation() + FVector(0, 0, 50);
            FVector End = OtherEnemy->GetActorLocation() + FVector(0, 0, 50);

            FCollisionQueryParams TraceParams;
            TraceParams.AddIgnoredActor(this);
            TraceParams.AddIgnoredActor(OtherEnemy);

            bool bBlocked = World->LineTraceSingleByChannel(
                LineHit,
                Start,
                End,
                ECC_Visibility,
                TraceParams
            );

            // Debug line
            if (bVisionDebug)
            {
                DrawDebugLine(
                    World,
                    Start,
                    End,
                    bBlocked ? FColor::Red : FColor::Green,
                    false,
                    0.15f,
                    0,
                    2.f
                );
            }

            if (bBlocked)
                continue;
        }

        // Sätt den andra fienden till chase läge
        //UE_LOG(LogTemp, Warning, TEXT("Spreading agro to %s"), *OtherEnemy->GetName());
        OtherEnemy->OnAgroSpreadTriggered();
    }
}


void AMeleeEnemy::OnAgroSpreadTriggered()
{
	bCanSeePlayer = true;

	//UE_LOG(LogTemp, Warning, TEXT("%s is now agro and is chasing!"), *GetName());
}


void AMeleeEnemy::UpdateStateVFX(EEnemyState NewState)
{
	if (!StateVFXComponent) return;

	if (NewState == EEnemyState::Patrolling)
	{
		if (AMeleeAIController* AI = Cast<AMeleeAIController>(GetController()))
		{
			StateVFXComponent->SetAsset(nullptr);
			StateVFXComponent->Deactivate();
			
			if (AI->GetCurrentMission() != EEnemyMission::Patrol)
			{
				StateVFXComponent->SetAsset(SearchVFX);
				StateVFXComponent->Activate(true);
			}
		}
	}

	// Om state inte ändrats, gör inget, borde fungera både för VFX och ljud
	if (PreviousState == NewState)
	{
		return;
	}

	// Logga state-byten
	/*UE_LOG(LogTemp, Warning, TEXT("STATE CHANGE: %s -> %s"),
		*UEnum::GetValueAsString(PreviousState),
		*UEnum::GetValueAsString(NewState)
	);*/

	//UE_LOG(LogTemp, Warning, TEXT("Alert: AudioComponent playing? %d"), StateAudioComponent->IsPlaying());
	
	// Bara spela ljud om state faktiskt ändrats 
	if (PreviousState != NewState)
	{
		switch (NewState)
		{
		case EEnemyState::Alert:
			PlayStateSound(AlertSound);
			break;
		case EEnemyState::Chasing:
			PlayStateSound(ChasingSound);
			break;
		case EEnemyState::Searching:
			PlayStateSound(SearchingSound);
			break;
		default:
			// Stoppa ljud 
			PlayStateSound(nullptr);
			break;
		}
	}
	
	// Uppdatera VFX 
	switch (NewState)
	{
	case EEnemyState::Patrolling:
		// Stäng av VFX
		StateVFXComponent->SetAsset(nullptr);
		StateVFXComponent->Deactivate();
		break;

	case EEnemyState::Alert:
		StateVFXComponent->SetAsset(AlertVFX);
		StateVFXComponent->Activate(true);
		break;

	case EEnemyState::Chasing:
		StateVFXComponent->SetAsset(ChaseVFX);
		StateVFXComponent->Activate(true);
		break;

	case EEnemyState::Searching:
		StateVFXComponent->SetAsset(SearchVFX);
		StateVFXComponent->Activate(true);
		break;

	default:
		// Stäng av VFX
		StateVFXComponent->SetAsset(nullptr);
		StateVFXComponent->Deactivate();
		break;
	}

	// Spara state 
	PreviousState = NewState;
}



void AMeleeEnemy::PlayStateSound(USoundBase* NewSound)
{
	if (!StateAudioComponent) return;

	// Gör inget om samma ljud redan spelar 
	if (StateAudioComponent->Sound == NewSound)
		return;

	StateAudioComponent->Stop();
	StateAudioComponent->SetSound(NewSound);

	if (NewSound)
		StateAudioComponent->Play();
}

void AMeleeEnemy::PlayHurtSound()
{
	if (!VoiceAudioComponent) return;

	// Välj slumpmässigt mellan de två ljuden
	USoundBase* SoundToPlay = nullptr;

	if (HurtSoundOne && HurtSoundTwo)
	{
		SoundToPlay = (FMath::RandBool()) ? HurtSoundOne : HurtSoundTwo;
	}
	else if (HurtSoundOne)
	{
		SoundToPlay = HurtSoundOne;
	}
	else if (HurtSoundTwo)
	{
		SoundToPlay = HurtSoundTwo;
	}

	if (SoundToPlay)
	{
		VoiceAudioComponent->SetSound(SoundToPlay);
		VoiceAudioComponent->Play();
	}
}


void AMeleeEnemy::SetLastSeenPlayerLocation(FVector NewLocation)
{
	LastSeenPlayerLocation = NewLocation;
}

void AMeleeEnemy::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	
	GetWorldTimerManager().ClearAllTimersForObject(this);

	if (MeleeHitBox)
	{
		MeleeHitBox->OnComponentBeginOverlap.Clear();
	}

	if (StateVFXComponent)
	{
		StateVFXComponent->Deactivate();
	}
}

// Time handle Funktioner:
void AMeleeEnemy::ResetAttackCooldown()
{
	bCanAttack = true;
}

void AMeleeEnemy::ForgetHeardSound()
{
	bHeardSoundRecently = false;
}
