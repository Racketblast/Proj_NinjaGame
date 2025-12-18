// Fill out your copyright notice in the Description page of Project Settings.
#include "Enemy.h"

#include "DialogueInfo.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"
#include "EnemyHandler.h"
#include "MeleeAIController.h"
#include "SmokeBombObject.h"
#include "StealthCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/AudioComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/OverlapResult.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GeometryCollection/GeometryCollectionComponent.h"


AEnemy::AEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	//Audio
	FootstepsAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("FootstepsAudioComponent"));
	FootstepsAudioComponent->SetupAttachment(RootComponent);
	StateAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("StateAudioComponent"));
	StateAudioComponent->SetupAttachment(RootComponent);
	VoiceAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("VoiceAudioComponent"));
	VoiceAudioComponent->SetupAttachment(RootComponent);
	ActionAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("ActionAudioComponent"));
	ActionAudioComponent->SetupAttachment(RootComponent);
	
	StateAudioComponent->bAutoActivate = false; 	// styr ljuden i koden, så detta ska vara false
	VoiceAudioComponent->bAutoActivate = false;
	ActionAudioComponent->bAutoActivate = false;

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
	GetCharacterMovement()->bUseControllerDesiredRotation = false;
	GetCharacterMovement()->bOrientRotationToMovement = false;

	// Helmet
	HelmetMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HelmetMesh"));
	HelmetMesh->SetupAttachment(SkeletalMeshComp, TEXT("Head")); 
	HelmetMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}


void AEnemy::BeginPlay()
{
	Super::BeginPlay();
	PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;

	
	if (AssassinationCapsule)
	{
		AssassinationCapsule->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::OnAssasinationOverlapBegin);
		AssassinationCapsule->OnComponentEndOverlap.AddDynamic(this, &AEnemy::OnAssasinationOverlapEnd);
	}

	OriginalSuspiciousVisionRange = SuspiciousVisionRange;
	OriginalVisionRange = VisionRange;
	
	OriginalHearingRange = HearingRange;

	//Helmet
	if (HelmetMesh)
	{
		HelmetMesh->SetVisibility(bHasHelmet);
	}
}

void AEnemy::FaceRotation(FRotator NewRotation, float DeltaTime)
{
	if (bRotationLocked) // För stunen
	{
		return;
	}
	
	FVector Vel = GetVelocity();
	Vel.Z = 0;

	if (Vel.Size() > 10.f)
	{
		NewRotation = Vel.Rotation();
	}

	Super::FaceRotation(NewRotation, DeltaTime);
}

void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CheckChaseProximityDetection();
	
	CheckPlayerVisibility();

	CheckCloseDetection();

	SpreadAgroToNearbyEnemies();

	// Används just nu bara för debug
	if (AEnemyAIController* AI = Cast<AEnemyAIController>(GetController()))
	{
		State = AI->GetCurrentState();
	}
}

void AEnemy::CheckChaseProximityDetection()
{
	if (!bIsChasing) return; 
	if (!PlayerPawn) return;

	AStealthCharacter* StealthPlayer = Cast<AStealthCharacter>(PlayerPawn);
	if (!StealthPlayer) return;
	
	if (StealthPlayer->bIsHiddenFromEnemy) return;

	const float ChaseProximityRadius = 400.f; // var 350
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
		
		if (AEnemyAIController* AI = Cast<AEnemyAIController>(GetController()))
		{
			AI->RefreshChaseTarget();
		}
	}
}

void AEnemy::CheckCloseDetection()
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
		GetWorld()->OverlapMultiByObjectType(
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
				/*FVector Start = GetActorLocation() + FVector(0, 0, 65.f);
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
				}*/

				// Kålla om linetracen träffar spelaren 
				if (HasLineOfSightToPlayer())
				{
					bCanSeePlayer = true;
					UpdateLastSeenPlayerLocation();
				}

				return;
			}
		}
	}
}

void AEnemy::CheckPlayerVisibility()
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
	//FVector LookDirection = Forward.RotateAngleAxis(10.f, GetActorRightVector()); // gör konen lite nedåtriktad
	FVector LookDirection;

	if (bIsChasing)
	{
		// Titta direkt mott spelaren när fienden är i chase 
		LookDirection = (PlayerPawn->GetActorLocation() - EnemyLocation).GetSafeNormal();
	}
	else
	{
		// gör konen lite nedåtriktad
		LookDirection = Forward.RotateAngleAxis(10.f, GetActorRightVector());
	}
	/*FVector ToPlayer = PlayerPawn->GetActorLocation() - EnemyLocation;
	float Distance = ToPlayer.Size();*/
	ToPlayer.Normalize();

	// Skillnad mellan patrull och chase läge 
	float EffectiveVisionRange = bIsChasing ? VisionRange * 1.4f : VisionRange * 0.6f; 
	float EffectiveVisionAngle = bIsChasing ? VisionAngle * 1.5f : VisionAngle * 0.5f; 

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

			/*bool bHit = GetWorld()->LineTraceSingleByChannel(
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

				if (bVisionDebug)
				{
					DrawDebugLine(GetWorld(), EnemyLocation, PlayerPawn->GetActorLocation(), FColor::Green, false, 0.1f);
				}
				return;
			}*/

			if (HasLineOfSightToPlayer())
			{
				bCanSeePlayer = true;
				bHasDirectVisualOnPlayer = true;
				bIsSuspicious = false;
				SuspiciousTimer = 0.f;
				UpdateLastSeenPlayerLocation();
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

			/*bool bHit = GetWorld()->LineTraceSingleByChannel(
				Hit,
				EnemyLocation,
				PlayerPawn->GetActorLocation(),
				ECC_Visibility,
				Params
			);*/

			if (HasLineOfSightToPlayer()) // !bHit || Hit.GetActor() == PlayerPawn
			{
				bPlayerInSuspiciousZone = true;
				bHasDirectVisualOnPlayer = true;
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

	if (!bPlayerWithinChaseProximity && State == EEnemyState::Chasing)
	{
		bCanSeePlayer = false;
	}
	bHasDirectVisualOnPlayer = false;
	bPlayerInSuspiciousZone = false;
	bPlayerInAlertCone = false;
}


bool AEnemy::HasLineOfSightToPlayer()
{
	if (!PlayerPawn) return false;
	AStealthCharacter* StealthPlayer = Cast<AStealthCharacter>(PlayerPawn);
	if (!StealthPlayer) return false;

	FVector EnemyEyes = GetActorLocation() + FVector(0,0,75);

	TArray<FVector> TargetPoints;

	// Mitten av spelaren
	TargetPoints.Add(PlayerPawn->GetActorLocation());
	
	// kamera = huvud
	TargetPoints.Add(StealthPlayer->GetFirstPersonCameraComponent()->GetComponentLocation());
	
	// vänster arm
	TargetPoints.Add(StealthPlayer->GetLeftArmVisionPoint());

	// höger arm
	TargetPoints.Add(StealthPlayer->GetRightArmVisionPoint());

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	Params.AddIgnoredActor(PlayerPawn);

	for (const FVector& Point : TargetPoints)
	{
		FHitResult Hit;

		bool bHit = GetWorld()->LineTraceSingleByChannel(
			Hit,
			EnemyEyes,
			Point,
			ECC_Visibility,
			Params
		);

		// Om det inte träffade något, eller träffade player först
		if (!bHit || Hit.GetActor() == PlayerPawn)
		{
			if (bVisionDebug)
			{
				DrawDebugLine(GetWorld(), EnemyEyes, Point, FColor::Green, false, 0.1f);
			}
			return true;
		}

		if (bVisionDebug)
		{
			DrawDebugLine(GetWorld(), EnemyEyes, Point, FColor::Red, false, 0.1f);
		}
	}

	return false;
}



void AEnemy::OnSuspiciousLocationDetected()
{
	UE_LOG(LogTemp, Warning, TEXT("OnSuspiciousLocationDetected()"));
	OnSuspiciousLocation.Broadcast(LastSeenPlayerLocation);
}


void AEnemy::UpdateLastSeenPlayerLocation()
{
	//UE_LOG(LogTemp, Warning, TEXT("UpdateLastSeenPlayerLocation"));
	if (PlayerPawn)
	{
		LastSeenPlayerLocation = PlayerPawn->GetActorLocation();
	}
}


float AEnemy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
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
			if (DamageCauser && !DamageCauser->IsA(AStealthCharacter::StaticClass()))
			{
				if (!DamageCauser->IsA(ASmokeBombObject::StaticClass()))
				{
					UE_LOG(LogTemp, Warning, TEXT("Enemy hit by throwing object and survived, Enemy spotted player!"));
					if (AEnemyAIController* AI = Cast<AEnemyAIController>(GetController()))
					{
						AI->StartChasingFromExternalOrder(PlayerPawn->GetActorLocation());
					}
				}
			}
		}

		// Uppdatera HP 
		Health = NewHealth;
		

		SetStartDialogueRowName("TakeDamage");
		StartDialogue();
		//PlayHurtSound();

		if (Health <= 0.0f)
		{
			Die();

			//For Ragdoll velocity
			if (DamageCauser)
			{
				FVector Direction = GetCapsuleComponent()->GetComponentLocation() - DamageCauser->GetActorLocation();
				Direction.Normalize();
				//UE_LOG(LogTemp, Warning, TEXT("Direction: %s"), *Direction.ToString());
				float ImpulseStrength = 1000.0f;
				SkeletalMeshComp->AddImpulse(Direction * ImpulseStrength, NAME_None, true);
			}
		}
	}

	return ActualDamage;
}

void AEnemy::Die()
{
	UE_LOG(LogTemp, Warning, TEXT("Enemy died!"));
	bIsDead = true;
	
	SetStartDialogueRowName("Death");
	StartDialogue();
	bCanPlayDialogue = false;
	
	SetActorTickEnabled(false);

	// Rensa alla timers 
	GetWorldTimerManager().ClearAllTimersForObject(this);

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

void AEnemy::StartAttack()
{
	// Gör detta i en sub klass
}

void AEnemy::OnAssasinationOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                                        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (AStealthCharacter* Player = Cast<AStealthCharacter>(OtherActor))
	{
		bCanBeAssassinated = true;
	}
}

void AEnemy::OnAssasinationOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (AStealthCharacter* Player = Cast<AStealthCharacter>(OtherActor))
	{
		bCanBeAssassinated = false;
	}
}


void AEnemy::ApplyDamageTo(AActor* Target)
{
	if (!Target) return;

	UGameplayStatics::ApplyDamage(Target, AttackDamage, GetController(), this, nullptr);
}

void AEnemy::ReduceEnemyRange(bool bShouldReduce)
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

void AEnemy::ReduceEnemyHearingRange(bool bShouldReduce)
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


void AEnemy::HearSoundAtLocation(FVector SoundLocation)
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
		GetWorldTimerManager().SetTimer(ForgetSoundHandle, this, &AEnemy::ForgetHeardSound, HearingMemoryTime, false);
	}
}

//Spread agro
void AEnemy::SpreadAgroToNearbyEnemies() 
{
	// Cooldown check
	float TimeNow = GetWorld()->GetTimeSeconds();
	if (TimeNow - LastAgroSpreadTime < AgroSpreadCooldown) return; 

	// Uppdatera timestamp
	LastAgroSpreadTime = TimeNow;
	
    // Hämta denna fiendes AI controller
    AEnemyAIController* MyAI = Cast<AEnemyAIController>(GetController());
    if (!MyAI) return;

    // Endast sprida agro om denna fiende faktiskt jagar
    if (MyAI->GetCurrentState() != EEnemyState::Chasing || !bHasDirectVisualOnPlayer)
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
        AEnemy* OtherEnemy = Cast<AEnemy>(Result.GetActor());
        if (!OtherEnemy || OtherEnemy == this)
            continue;

        // Hämta andra fiendens AI controller
        AEnemyAIController* OtherAI = Cast<AEnemyAIController>(OtherEnemy->GetController());
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


void AEnemy::OnAgroSpreadTriggered()
{
	bCanSeePlayer = true;
	//UE_LOG(LogTemp, Warning, TEXT("%s is now agro and is chasing!"), *GetName());
}


void AEnemy::UpdateStateVFX(EEnemyState NewState)
{
	if (!StateVFXComponent) return;

	if (StunnedVFX)
	{
		if (AEnemyAIController* AI = Cast<AEnemyAIController>(GetController()))
		{
			if (AI->GetIsStunned())
			{
				StateVFXComponent->SetAsset(StunnedVFX);
				StateVFXComponent->Activate(true);
				PreviousState = EEnemyState::Patrolling;
				return;
			}
		}
	}

	if (NewState == EEnemyState::Patrolling)
	{
		if (AEnemyAIController* AI = Cast<AEnemyAIController>(GetController()))
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
			StartDialogueRowName = "Alert";
			StartDialogue();
			//PlayStateSound(AlertSound);
			break;
		case EEnemyState::Chasing:
			StartDialogueRowName = "Chasing";
			StartDialogue();
			PlayStateSound(ChasingSound);
			break;
		case EEnemyState::Searching:
			StartDialogueRowName = "Searching";
			StartDialogue();
			//PlayStateSound(SearchingSound);
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



void AEnemy::PlayStateSound(USoundBase* NewSound)
{
	if (!StateAudioComponent) return;

	if (!NewSound)
		return;
	// Gör inget om samma ljud redan spelar 
	if (StateAudioComponent->Sound == NewSound && StateAudioComponent->IsPlaying())
		return;

	StateAudioComponent->Stop();
	StateAudioComponent->SetSound(NewSound);

	if (NewSound)
		StateAudioComponent->Play();
}

void AEnemy::PlayHurtSound()
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


void AEnemy::SetLastSeenPlayerLocation(FVector NewLocation)
{
	LastSeenPlayerLocation = NewLocation;
}

void AEnemy::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	
	GetWorldTimerManager().ClearAllTimersForObject(this);

	if (StateVFXComponent)
	{
		StateVFXComponent->Deactivate();
	}
}

//Helmet
void AEnemy::RemoveHelmet()
{
	if (!bHasHelmet) return;

	bHasHelmet = false;

	if (AEnemyAIController* AI = Cast<AEnemyAIController>(GetController()))
	{
		AI->StartChasingFromExternalOrder(PlayerPawn->GetActorLocation());
		AI->StunEnemy(1.0f, EEnemyState::Chasing); 
	}

	if (HelmetMesh)
	{
		FTransform HelmetTransform = HelmetMesh->GetComponentTransform();

		// Dölj hjälm
		HelmetMesh->SetVisibility(false);
		HelmetMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		
		if (HelmetBreakGeoCollection)
		{
			UGeometryCollectionComponent* GeoComp =
				NewObject<UGeometryCollectionComponent>(this, UGeometryCollectionComponent::StaticClass());

			if (GeoComp)
			{
				GeoComp->SetupAttachment(GetMesh()); 
				GeoComp->SetWorldTransform(HelmetTransform);
				GeoComp->RegisterComponent();

				GeoComp->SetRelativeTransform(FTransform::Identity);

				GeoComp->SetRestCollection(HelmetBreakGeoCollection);
				GeoComp->SetPerLevelCollisionProfileNames({ "Debris","Debris","Debris"} );
				GeoComp->SetCanEverAffectNavigation(false);
			}
		}
	}
}

void AEnemy::SetHaveHelmet(bool bHelmet)
{
	bHasHelmet = bHelmet;

	if (HelmetMesh)
	{
		HelmetMesh->SetVisibility(bHasHelmet);
		//HelmetMesh->SetCollisionEnabled(bHasHelmet ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
	}
}


// Time handle Funktioner:
void AEnemy::ForgetHeardSound()
{
	bHeardSoundRecently = false;
}




//Throw
void AEnemy::EnemyThrow()
{
	// Den riktiga implementationen finns i melee fienden
}

float AEnemy::GetThrowRange() const
{
	return 0;
}

float AEnemy::GetThrowCooldown() const
{
	return 0;
}

bool AEnemy::IsLocationStillSeeingPlayer(const FVector& Location) const
{
	return true;
}

//Enemy Voice
void AEnemy::StartDialogue()
{
	if (!bCanPlayDialogue)
		return;
	
	if (!EnemyVoiceInfo || EnemyVoiceInfo->GetRowStruct() != FDialogueInfo::StaticStruct())
		return;
	
	if (StartDialogueRowName == "")
		return;
	
	CurrentDialogueRowName = StartDialogueRowName;

	if (FDialogueInfo* Row = EnemyVoiceInfo->FindRow<FDialogueInfo>(StartDialogueRowName, TEXT("")))
	{
		NextDialogueRowName = Row->NextDialogue;

		//Plays the dialogue for the amount of time the sound plays
		float TimeUntilNextDialogue = 0.0f;
		if (Row->DialogueSound)
		{
			if (VoiceAudioComponent)
			{
				VoiceAudioComponent->SetSound(Row->DialogueSound);
				VoiceAudioComponent->Play();
			}
			
			TimeUntilNextDialogue = Row->DialogueSound->GetDuration();
		}
		//Goes to next dialogue
		//This is the reason why the dialogue is broken into two functions, because it needs a delay between each dialog
		GetWorld()->GetTimerManager().SetTimer(DialogueTimerHandle, this, &AEnemy::NextDialogue, TimeUntilNextDialogue, false);
	}
}

void AEnemy::NextDialogue()
{
	if (!bCanPlayDialogue)
		return;
	
	if (!EnemyVoiceInfo || EnemyVoiceInfo->GetRowStruct() != FDialogueInfo::StaticStruct())
		return;
	
	if (NextDialogueRowName != "")
	{
		CurrentDialogueRowName = NextDialogueRowName;
		if (FDialogueInfo* Row = EnemyVoiceInfo->FindRow<FDialogueInfo>(NextDialogueRowName, TEXT("")))
		{
			NextDialogueRowName = Row->NextDialogue;
			//Plays the dialogue for the amount of time the sound plays
			float TimeUntilNextDialogue = 0.0f;
			if (Row->DialogueSound)
			{
				if (VoiceAudioComponent)
				{
					VoiceAudioComponent->SetSound(Row->DialogueSound);
					VoiceAudioComponent->Play();
				}
				TimeUntilNextDialogue = Row->DialogueSound->GetDuration();
			}
			
			//Goes to next dialogue
			GetWorld()->GetTimerManager().SetTimer(DialogueTimerHandle, this, &AEnemy::NextDialogue, TimeUntilNextDialogue, false);
		}
	}
}

void AEnemy::StopDialogue()
{
	GetWorld()->GetTimerManager().ClearTimer(DialogueTimerHandle);

	if (VoiceAudioComponent && VoiceAudioComponent->IsPlaying())
	{
		VoiceAudioComponent->Stop();
	}

	StartDialogueRowName = "";
	CurrentDialogueRowName = "";
	NextDialogueRowName = "";
}

float AEnemy::GetDialogueDuration()
{
	float TimeUntilNextDialogue = 0.0f;
	
	if (!EnemyVoiceInfo || EnemyVoiceInfo->GetRowStruct() != FDialogueInfo::StaticStruct())
		return TimeUntilNextDialogue;
	
	if (CurrentDialogueRowName != "")
	{
		if (FDialogueInfo* Row = EnemyVoiceInfo->FindRow<FDialogueInfo>(CurrentDialogueRowName, TEXT("")))
		{
			if (Row->DialogueSound)
			{
				TimeUntilNextDialogue = Row->DialogueSound->GetDuration();
			}
		}
	}
	return TimeUntilNextDialogue;
}