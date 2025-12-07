// Fill out your copyright notice in the Description page of Project Settings.


#include "MeleeEnemy.h"

#include "AIThrowableObject.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"
#include "StealthCharacter.h"
#include "Components/AudioComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
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
	// ProjectileSpawnPoint
	ProjectileSpawnPoint = CreateDefaultSubobject<USceneComponent>(TEXT("ProjectileSpawnPoint"));
	ProjectileSpawnPoint->SetupAttachment(GetMesh()); 
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

	/*if (ThrowCooldown > 2.0f)
	{
		EnemyThrow();
		ThrowCooldown = 0.f;
	}
	else
	{
		ThrowCooldown += DeltaTime;
	}*/
}

void AMeleeEnemy::CheckChaseProximityDetection()
{
	Super::CheckChaseProximityDetection();
}

void AMeleeEnemy::CheckCloseDetection()
{
	Super::CheckCloseDetection();
}

void AMeleeEnemy::CheckPlayerVisibility()
{
	Super::CheckPlayerVisibility();
}

void AMeleeEnemy::StartAttack()
{
	if (!bCanAttack) return;

	UE_LOG(LogTemp, Error, TEXT("StartAttack"))

	bCanAttack = false;
	//bIsAttacking = true;
	bHitRegisteredThisSwing = false;

	if (AttackSound)
	{
		ActionAudioComponent->SetSound(AttackSound);
		ActionAudioComponent->Play();
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

/*void AMeleeEnemy::EnemyThrow()
{
	if (!ProjectileSpawnPoint || !ThrowableObject) return;

	FVector Start = GetCapsuleComponent()->GetComponentLocation();
	FVector End = ProjectileSpawnPoint->GetComponentLocation();

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, Params))
	{
		End = Start;
	}

	//FVector SpawnLocation = ProjectileSpawnPoint->GetComponentLocation();
	FVector SpawnLocation = End;
	FRotator SpawnRotation = (PlayerPawn->GetActorLocation() - SpawnLocation).Rotation();

	AAIThrowableObject* ThrownObject = GetWorld()->SpawnActor<AAIThrowableObject>(
		ThrowableObject,
		SpawnLocation,
		SpawnRotation
	);

	if (ThrownObject)
	{
		ThrownObject->Thrown = true;
		ThrownObject->SetShowVFX(false);
		ThrownObject->bBreaksOnImpact = true;
		ThrownObject->DealtDamage = AttackDamage;

		// velocity
		FVector Direction = (PlayerPawn->GetActorLocation() - SpawnLocation).GetSafeNormal();
		ThrownObject->ThrowVelocity = Direction * ThrowVelocity;

		ThrownObject->StaticMeshComponent->SetCollisionResponseToChannel(TRACE_CHANNEL_INTERACT, ECR_Ignore);
		ThrownObject->StaticMeshComponent->SetSimulatePhysics(true);
		ThrownObject->StaticMeshComponent->SetNotifyRigidBodyCollision(true);
		ThrownObject->StaticMeshComponent->SetCanEverAffectNavigation(false);
		ThrownObject->StaticMeshComponent->SetUseCCD(true);

		ThrownObject->StaticMeshComponent->SetPhysicsLinearVelocity(ThrownObject->ThrowVelocity, false);
	}
}*/

void AMeleeEnemy::EnemyThrow()
{
	if (!ProjectileSpawnPoint || !ThrowableObject) return;

	FVector SpawnLocation = ProjectileSpawnPoint->GetComponentLocation();
	FVector PlayerLocation = PlayerPawn->GetActorLocation();

	float ProjectileSpeed = 1100.f;
	FVector PredictedLocation = PredictPlayerLocation(ProjectileSpeed);
	
	/*if (Distance < 300.f)
	{
		PredictedLocation = PlayerLocation; // ingen prediction på nära håll
	}*/

	// Ändrar hur högt fienden vill kasta, tycker inte riktigt att detta fungerar för tillfället
	float ArcHeight = 0.f;
	bool bBlocked = IsThrowPathBlocked(SpawnLocation, PredictedLocation, ArcHeight);

	if (bBlocked)
	{
		ArcHeight = 200.f; 
		bBlocked = IsThrowPathBlocked(SpawnLocation, PredictedLocation, ArcHeight);
	}

	/*if (bBlocked)
	{
		for (int i = 0; i < 3; i++)
		{
			if (!IsThrowPathBlocked(SpawnLocation, PredictedLocation, ArcHeight))
				break;

			ArcHeight += 150.f; 
		}
	}*/

	if (bBlocked)
	{
		// Back off, fungerar inte just nu!
		UE_LOG(LogTemp, Warning, TEXT("EnemyThrow: bBlocked är true"));
		FVector BackLoc = GetActorLocation() - GetActorForwardVector() * 250.f;
		AMeleeAIController* AICon = Cast<AMeleeAIController>(GetController());
		if (AICon)
		{
			AICon->StartBackOff(BackLoc); 
		}
		return;
	}


	// Spawn Projectile
	AAIThrowableObject* ThrownObject = GetWorld()->SpawnActor<AAIThrowableObject>(
		ThrowableObject,
		SpawnLocation,
		(PredictedLocation - SpawnLocation).Rotation()
	);

	if (!ThrownObject) return;


	ThrownObject->ChangeToThrowCollision(true);
	ThrownObject->ThrowCollision->SetUseCCD(true);
	ThrownObject->Thrown = true;
	ThrownObject->SetShowVFX(false);
	ThrownObject->bBreaksOnImpact = true;
	ThrownObject->DealtDamage = AttackDamage;
	ThrownObject->GravityZ = GetWorld()->GetGravityZ();

	// Throw Velocity
	FVector LaunchVelocity;
	FVector StartPos = SpawnLocation;
	FVector EndPos = PredictedLocation;

	FVector AdjustedEndPos = EndPos;
	AdjustedEndPos.Z -= 40.f; 
	
	bool bHasSolution = UGameplayStatics::SuggestProjectileVelocity(
		this,
		LaunchVelocity,
		StartPos,
		AdjustedEndPos,
		1100.f,
		false,
		0.f,
		0.f,
		ESuggestProjVelocityTraceOption::DoNotTrace
	);

	if (!bHasSolution)
	{
		LaunchVelocity = (AdjustedEndPos - StartPos).GetSafeNormal() * ProjectileSpeed;
	}
	
	// LaunchVelocity.Z += 300.f; 

	// Cap Z speed så fienden inte skjuter för högt
	
	ThrownObject->ThrowVelocity = LaunchVelocity;

	ThrownObject->ChangeToThrowCollision(true);
	ThrownObject->ThrowCollision->SetUseCCD(true);
	

	if (AttackSound)
	{
		ActionAudioComponent->SetSound(AttackSound);
		ActionAudioComponent->Play();
	}
}



bool AMeleeEnemy::IsThrowPathBlocked(const FVector& SpawnLocation, const FVector& TargetLocation, float TestArcHeight)
{
	const int Samples = 10;

	for (int i = 1; i <= Samples; i++)
	{
		float T = (float)i / Samples;

		FVector SamplePoint = FMath::Lerp(SpawnLocation, TargetLocation, T);
		
		SamplePoint.Z += TestArcHeight * FMath::Sin(T * PI); 

		FHitResult Hit;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(this);

		if (GetWorld()->LineTraceSingleByChannel(Hit, SpawnLocation, SamplePoint, ECC_Visibility, Params))
		{
			UE_LOG(LogTemp, Warning, TEXT("IsThrowPathBlocked: Något blockerar kastet"));
			return true; 
		}
	}

	return false; 
}


FVector AMeleeEnemy::PredictPlayerLocation(float ProjectileSpeed) const
{
	if (!PlayerPawn) return FVector::ZeroVector;

	FVector PlayerLocation = PlayerPawn->GetActorLocation();
	FVector PlayerVelocity = PlayerPawn->GetVelocity();

	FVector Origin = ProjectileSpawnPoint->GetComponentLocation();

	float Distance = FVector::Dist(Origin, PlayerLocation);

	float TravelTime = Distance / ProjectileSpeed;

	return PlayerLocation + PlayerVelocity * TravelTime;
}
