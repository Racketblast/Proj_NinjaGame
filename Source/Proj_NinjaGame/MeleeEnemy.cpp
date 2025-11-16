// Fill out your copyright notice in the Description page of Project Settings.


#include "MeleeEnemy.h"

#include "AIController.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"
#include "MeleeAIController.h"
#include "StealthCharacter.h"
#include "Components/AudioComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"


AMeleeEnemy::AMeleeEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	//Audio
	StateAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("StateAudioComponent"));
	StateAudioComponent->SetupAttachment(RootComponent);
	
	StateAudioComponent->bAutoActivate = false; 	// styr ljuden i koden, så detta ska vara false

	// Skapa hitbox och fäst vid mesh 
	MeleeHitBox = CreateDefaultSubobject<UBoxComponent>(TEXT("MeleeHitBox"));
	MeleeHitBox->SetupAttachment(GetMesh()); 
	// Positionera hitbox 
	MeleeHitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeleeHitBox->SetCollisionObjectType(ECC_WorldDynamic);
	MeleeHitBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	MeleeHitBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	
	AssassinationCapsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("AssassinationCapsule"));
	AssassinationCapsule->SetupAttachment(GetMesh());


	StateVFXComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("StateVFX"));
	StateVFXComponent->SetupAttachment(GetMesh());
	StateVFXComponent->SetRelativeLocation(FVector(0.f, 0.f, 120.f));

	// För att undvika att gå in i andra fiender
	GetCharacterMovement()->bUseRVOAvoidance = true;
	GetCharacterMovement()->AvoidanceWeight = 0.5f;
	GetCharacterMovement()->AvoidanceConsiderationRadius = 300.f;
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

	if (StealthPlayer->GetPlayerMovementState() == EPlayerMovementState::Crouch)
		return; // om Spelaren smyger så ska fienden inte autoupptäcka

	//UE_LOG(LogTemp, Warning, TEXT("CheckImmediateProximityDetection"));

	// direkt upptäck spelaren
	bCanSeePlayer = true;
	UpdateLastSeenPlayerLocation();
	
	OnSuspiciousLocation.Broadcast(PlayerPawn->GetActorLocation()); 

	#if WITH_EDITOR
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
	#endif
}

void AMeleeEnemy::CheckChaseProximityDetection()
{
	if (!bIsChasing) return; 
	if (!PlayerPawn) return;

	const float ChaseProximityRadius = 350.f; 
	const float Distance = FVector::Dist(GetActorLocation(), PlayerPawn->GetActorLocation());

	// Debug sphere
	/*#if WITH_EDITOR
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
	#endif*/

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


/*void AMeleeEnemy::CheckPlayerVisibility()
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
}*/

void AMeleeEnemy::CheckPlayerVisibility()
{
	if (!PlayerPawn) return;

	// Variabler 
	FVector EnemyLocation = GetActorLocation() + FVector(0, 0, 50);
	FVector Forward = GetActorForwardVector();
	FVector LookDirection = Forward.RotateAngleAxis(10.f, GetActorRightVector()); // gör konen lite nedåtriktad
	FVector ToPlayer = PlayerPawn->GetActorLocation() - EnemyLocation;
	float Distance = ToPlayer.Size();
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
				/*FRotator LookAtRot = (PlayerPawn->GetActorLocation() - GetActorLocation()).Rotation();
				SetActorRotation(FMath::RInterpTo(GetActorRotation(), LookAtRot, GetWorld()->GetDeltaSeconds(), 2.f));*/
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

	bPlayerInSuspiciousZone = false;
	bCanSeePlayer = false;
	bPlayerInAlertCone = false;
}

void AMeleeEnemy::OnSuspiciousLocationDetected()
{
	UE_LOG(LogTemp, Warning, TEXT("OnSuspiciousLocationDetected()"));
	OnSuspiciousLocation.Broadcast(LastSeenPlayerLocation);
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
	
	/*GetWorldTimerManager().SetTimer(AttackCooldownHandle, [this]()
	{
		bCanAttack = true;
	}, AttackCooldown, false);*/
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


/*void AMeleeEnemy::UpdateStateVFX(EEnemyState NewState)
{
	if (!StateVFXComponent) return;
	
	// Om vi redan spelar samma vfx, gör inget
	if (StateVFXComponent->GetAsset() != nullptr)
	{
		UNiagaraSystem* CurrentAsset = StateVFXComponent->GetAsset();
		
		UNiagaraSystem* NewAsset = nullptr;
		switch (NewState)
		{
		case EEnemyState::Patrolling:
			NewAsset = nullptr;
			break;
		case EEnemyState::Alert:
			NewAsset = AlertVFX;
			break;
		case EEnemyState::Chasing:
			NewAsset = ChaseVFX;
			break;
		case EEnemyState::Searching:
			NewAsset = SearchVFX;
			break;
		default:
			NewAsset = nullptr;
			break;
		}
		
		if (NewAsset == CurrentAsset)
		{
			return;
		}
	}

	switch (NewState)
	{
	case EEnemyState::Patrolling:
		// Stäng av VFX
		StateVFXComponent->SetAsset(nullptr);
		StateVFXComponent->Deactivate();
		break;
	case EEnemyState::Alert:
		if (AlertVFX)
		{
			StateVFXComponent->SetAsset(AlertVFX);
			StateVFXComponent->Activate(true);
		}
		if (AlertSound)
		{
			PlayStateSound(AlertSound);
		}
		break;
	case EEnemyState::Chasing:
		if (ChaseVFX)
		{
			StateVFXComponent->SetAsset(ChaseVFX);
			StateVFXComponent->Activate(true);
		}
		if (ChasingSound)
		{
			PlayStateSound(ChasingSound);
		}
		break;

	case EEnemyState::Searching:
		if (SearchVFX)
		{
			StateVFXComponent->SetAsset(SearchVFX);
			StateVFXComponent->Activate(true);
		}
		if (SearchingSound)
		{
			PlayStateSound(SearchingSound);
		}
		break;

	default:
		// Stäng av VFX
		StateVFXComponent->SetAsset(nullptr);
		StateVFXComponent->Deactivate();

		// Stoppa ljud 
		PlayStateSound(nullptr); 
		break;
	}
}*/

void AMeleeEnemy::UpdateStateVFX(EEnemyState NewState)
{
	if (!StateVFXComponent) return;

	// Om state inte ändrats, gör inget, borde fungera både för VFX och ljud
	if (PreviousState == NewState)
	{
		return;
	}
	
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
