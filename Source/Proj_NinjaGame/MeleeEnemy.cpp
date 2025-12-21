// Fill out your copyright notice in the Description page of Project Settings.


#include "MeleeEnemy.h"

#include "AIThrowableObject.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/GameplayStaticsTypes.h"
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

	if (!PlayerPawn)
		return;
	
	SmoothedPlayerVelocity = FMath::VInterpTo(
		SmoothedPlayerVelocity,
		PlayerPawn->GetVelocity(),
		DeltaTime,
		5.f
	);

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

/*bool AMeleeEnemy::HasLineOfSightToPlayer() 
{
	Super::HasLineOfSightToPlayer();
}*/


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


void AMeleeEnemy::EnemyThrow()
{
	if (!ProjectileSpawnPoint || !ThrowableObject || !PlayerPawn) return;

	UWorld* World = GetWorld();
	if (!World) return;

	const FVector SpawnLocation = ProjectileSpawnPoint->GetComponentLocation();
	
	float BaseProjectileSpeed = 1100.f;
	float ChosenProjectileSpeed = BaseProjectileSpeed;

	FVector PredictedLocation = PredictPlayerLocation(BaseProjectileSpeed);
	FVector BestTargetLocation = PredictedLocation;
	float ChosenArcHeight = 0.f;

	bool bFoundValidThrow = false;
	
	// Testkombinationer
	const TArray<float> SpeedMultipliers = { 1.0f, 1.15f, 1.3f };
	const TArray<float> ArcHeights = { 0.f, 150.f, 300.f, 450.f, 600.f };
	const TArray<float> TargetZOffsets = { 0.f, 50.f, 100.f };
	
	// ARC sökning
	for (float SpeedMul : SpeedMultipliers)
	{
		float TestSpeed = BaseProjectileSpeed * SpeedMul;

		for (float ZOffset : TargetZOffsets)
		{
			FVector TestTarget = PredictedLocation;
			TestTarget.Z += ZOffset;

			for (float Arc : ArcHeights)
			{
				if (!IsThrowPathBlocked(SpawnLocation, TestTarget, Arc))
				{
					// Giltig bana hittad
					bFoundValidThrow = true;
					ChosenProjectileSpeed = TestSpeed;
					BestTargetLocation = TestTarget;
					ChosenArcHeight = Arc;

					//UE_LOG(LogTemp, Warning, TEXT("EnemyThrow: Valid arc found | Speed %.0f | Arc %.0f | ZOffset %.0f"), TestSpeed, Arc, ZOffset);

					goto FOUND_THROW;
				}
			}
		}
	}

FOUND_THROW:
	
	// Ingen giltig bana, kör backoff
	if (!bFoundValidThrow)
	{
		UE_LOG(LogTemp, Warning, TEXT("EnemyThrow: No valid throw found, attempting backoff"));

		if (AMeleeAIController* AICon = Cast<AMeleeAIController>(GetController()))
		{
			FVector BackLoc = GetActorLocation() - GetActorForwardVector() * 250.f;
			AICon->StartBackOff(BackLoc);
		}

		// Fallback: kasta rakt mot spelaren
		BestTargetLocation = PlayerPawn->GetActorLocation();
		ChosenProjectileSpeed = BaseProjectileSpeed * 0.9f;
		ChosenArcHeight = 0.f;
		//return;
	}

	// Debug 
	if (bVisionDebug)
	{
		const int DebugSamples = 20;
		FVector PrevPoint = SpawnLocation;

		for (int i = 1; i <= DebugSamples; i++)
		{
			float T = (float)i / DebugSamples;
			FVector Point = FMath::Lerp(SpawnLocation, BestTargetLocation, T);
			Point.Z += ChosenArcHeight * FMath::Sin(T * PI);

			DrawDebugLine(
				World,
				PrevPoint,
				Point,
				FColor::Blue,
				false,
				2.0f,
				0,
				2.5f
			);

			PrevPoint = Point;
		}

		DrawDebugSphere(World, BestTargetLocation, 20.f, 12, FColor::Yellow, false, 2.f);
	}
	
	// Spawna Projektil
	AAIThrowableObject* ThrownObject = World->SpawnActor<AAIThrowableObject>(
		ThrowableObject,
		SpawnLocation,
		(BestTargetLocation - SpawnLocation).Rotation()
	);

	if (!ThrownObject) return;

	ThrownObject->SetRandomMesh();
	ThrownObject->Thrown = true;
	ThrownObject->SetShowVFX(false);
	ThrownObject->bBreaksOnImpact = true;
	ThrownObject->DealtDamage = AttackDamage;
	ThrownObject->GravityZ = World->GetGravityZ();
	ThrownObject->ChangeToThrowCollision(true);
	ThrownObject->ThrowCollision->SetUseCCD(true);
	
	// Beräkna kasthastighet
	FVector LaunchVelocity;
	FVector AdjustedTarget = BestTargetLocation;
	AdjustedTarget.Z -= 40.f;

	UGameplayStatics::FSuggestProjectileVelocityParameters Params(
		this,
		SpawnLocation,
		AdjustedTarget,
		ChosenProjectileSpeed
	);

	Params.TraceOption = ESuggestProjVelocityTraceOption::DoNotTrace;
	Params.OverrideGravityZ = 0.f;
	Params.CollisionRadius = 0.f;
	Params.bFavorHighArc = ChosenArcHeight > 200.f;
	Params.bDrawDebug = false;
	Params.bAcceptClosestOnNoSolutions = false;

	bool bHasVelocity = UGameplayStatics::SuggestProjectileVelocity(Params, LaunchVelocity);

	if (!bHasVelocity)
	{
		LaunchVelocity = (AdjustedTarget - SpawnLocation).GetSafeNormal() * ChosenProjectileSpeed;
	}

	ThrownObject->ThrowVelocity = LaunchVelocity;
	
	// Ljud
	if (AttackSound)
	{
		ActionAudioComponent->SetSound(AttackSound);
		ActionAudioComponent->Play();
	}
}


/*bool AMeleeEnemy::IsThrowPathBlocked(const FVector& SpawnLocation, const FVector& TargetLocation, float TestArcHeight)
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
			//UE_LOG(LogTemp, Warning, TEXT("IsThrowPathBlocked: Något blockerar kastet"));
			return true; 
		}
	}

	return false; 
}*/

bool AMeleeEnemy::IsThrowPathBlocked(
	const FVector& SpawnLocation,
	const FVector& TargetLocation,
	float TestArcHeight
)
{
	UWorld* World = GetWorld();
	if (!World) return true;

	const int Samples = 14;                 

	FVector PreviousPoint = SpawnLocation;

	for (int i = 1; i <= Samples; i++)
	{
		float T = (float)i / Samples;

		FVector CurrentPoint = FMath::Lerp(SpawnLocation, TargetLocation, T);
		CurrentPoint.Z += TestArcHeight * FMath::Sin(T * PI);

		FHitResult Hit;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(this);
		Params.AddIgnoredActor(PlayerPawn); 

		FCollisionShape Sphere = FCollisionShape::MakeSphere(ProjectileRadius);

		bool bHit = World->SweepSingleByChannel(
			Hit,
			PreviousPoint,
			CurrentPoint,
			FQuat::Identity,
			ECC_Visibility,
			Sphere,
			Params
		);

		// Debug
		if (bHit)
		{
			/*DrawDebugSphere(
				World,
				Hit.ImpactPoint,
				ProjectileRadius,
				12,
				FColor::Red,
				false,
				1.5f
			);

			DrawDebugLine(
				World,
				PreviousPoint,
				CurrentPoint,
				FColor::Red,
				false,
				1.5f,
				0,
				2.f
			);*/

			return true;
		}
		/*DrawDebugLine(
			World,
			PreviousPoint,
			CurrentPoint,
			FColor::Green,
			false,
			0.5f,
			0,
			1.5f
		);*/
		PreviousPoint = CurrentPoint;
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

/*FVector AMeleeEnemy::PredictPlayerLocation(float ProjectileSpeed) const
{
	if (!PlayerPawn || !ProjectileSpawnPoint)
		return FVector::ZeroVector;

	const FVector Origin = ProjectileSpawnPoint->GetComponentLocation();
	const FVector PlayerPos = PlayerPawn->GetActorLocation();
	const FVector PlayerVel = SmoothedPlayerVelocity;

	// Om spelaren nästan står still
	if (PlayerVel.SizeSquared() < 25.f)
	{
		return PlayerPos;
	}

	const float GravityZ = GetWorld()->GetGravityZ(); 
	const float MaxLeadTime = 0.8f;

	FVector PredictedPos = PlayerPos;
	float Time = 0.f;

	for (int i = 0; i < 4; ++i)
	{
		const float Distance = FVector::Dist(Origin, PredictedPos);
		Time = Distance / ProjectileSpeed;
		Time = FMath::Min(Time, MaxLeadTime);

		// Horisontell prediktion
		PredictedPos = PlayerPos + PlayerVel * Time;
		
		/*const float GravityCompensation = 0.5f * FMath::Abs(GravityZ) * Time * Time;
		PredictedPos.Z += GravityCompensation;#1#
	}

	if (bVisionDebug)
	{
		DrawDebugSphere(
			GetWorld(),
			PredictedPos,
			20.f,
			12,
			FColor::Green,
			false,
			1.f
		);
	}


	return PredictedPos;
}*/


bool AMeleeEnemy::IsLocationStillSeeingPlayer(const FVector& TestLoc) const
{
	if (!PlayerPawn) return false;

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		Hit,
		TestLoc,
		PlayerPawn->GetActorLocation(),
		ECC_Visibility,
		Params
	);
	
	return !bHit;
}
