// Fill out your copyright notice in the Description page of Project Settings.


#include "MeleeEnemy.h"

#include "AIThrowableObject.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"
#include "StealthCharacter.h"
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

	if (ThrowCooldown > 2.0f)
	{
		EnemyThrow();
		ThrowCooldown = 0.f;
	}
	else
	{
		ThrowCooldown += DeltaTime;
	}
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
	FVector Start = GetCapsuleComponent()->GetComponentLocation();
	//din punkt william
	FVector End = GetCapsuleComponent()->GetComponentLocation();

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, Params))
	{
		End = Start;
	}

	FVector SpawnLocation = End;
	FRotator SpawnRotation = GetCapsuleComponent()->GetComponentRotation();
	if (ThrowableObject)
	{
		AAIThrowableObject* ThrownObject = GetWorld()->SpawnActor<AAIThrowableObject>(ThrowableObject, SpawnLocation, SpawnRotation);
		ThrownObject->Thrown = true;
		ThrownObject->SetShowVFX(false);
		ThrownObject->bBreaksOnImpact = true;
		ThrownObject->DealtDamage = AttackDamage;
		//Lägg till forward från fienden till spelaren
		ThrownObject->ThrowVelocity = GetCapsuleComponent()->GetForwardVector() * ThrowVelocity;

		ThrownObject->StaticMeshComponent->SetCollisionResponseToChannel(TRACE_CHANNEL_INTERACT,ECR_Ignore);
		ThrownObject->StaticMeshComponent->SetSimulatePhysics(true);
		ThrownObject->StaticMeshComponent->SetNotifyRigidBodyCollision(true);
		ThrownObject->StaticMeshComponent->SetCanEverAffectNavigation(false);
		ThrownObject->StaticMeshComponent->SetUseCCD(true);
		
		ThrownObject->StaticMeshComponent->SetPhysicsLinearVelocity(ThrownObject->ThrowVelocity, false);
	}
}

