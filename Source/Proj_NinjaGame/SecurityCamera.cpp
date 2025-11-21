// Fill out your copyright notice in the Description page of Project Settings.


#include "SecurityCamera.h"

#include "EnemyHandler.h"
#include "NavigationSystem.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"


ASecurityCamera::ASecurityCamera()
{
	PrimaryActorTick.bCanEverTick = true;

	//Mesh
	CameraMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CameraMesh"));
	RootComponent = CameraMesh;

	//hitbox
	HitCollision = CreateDefaultSubobject<USphereComponent>(TEXT("HitCollision"));
	HitCollision->SetupAttachment(CameraMesh);
	HitCollision->InitSphereRadius(40.f);
	HitCollision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	HitCollision->SetCollisionObjectType(ECC_Pawn);
	HitCollision->SetCollisionResponseToAllChannels(ECR_Block);
	HitCollision->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	HitCollision->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	HitCollision->SetNotifyRigidBodyCollision(true);
	
}

void ASecurityCamera::BeginPlay()
{
	Super::BeginPlay();
	PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

	
	/*if (CameraMesh && IdlePanAnimation)
	{
		CameraMesh->PlayAnimation(IdlePanAnimation, true);
		CameraMesh->SetPlayRate(0.25f); 
		bIsAnimationPlaying = true;
	}*/
}


void ASecurityCamera::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CheckPlayerVisibility(DeltaTime);
}


void ASecurityCamera::CheckPlayerVisibility(float DeltaTime)
{
	if (!PlayerPawn || !CameraMesh)
		return;

	if (bIsCameraDisabled)
		return;
	
	// Hämta position och forward från socket "Vision" i SKM
	const FName VisionSocket = FName("Vision");

	FVector CameraLocation = CameraMesh->GetSocketLocation(VisionSocket);
	FVector Forward = CameraMesh->GetSocketRotation(VisionSocket).Vector();
	
	// Beräkna riktning och distans till spelaren
	FVector ToPlayer = PlayerPawn->GetActorLocation() - CameraLocation;
	float Distance = ToPlayer.Size();
	ToPlayer.Normalize();

	bool bNewPlayerInCone = true;  // Temporär status
	
	// Avståndskoll
	if (Distance > VisionRange)
	{
		bNewPlayerInCone = false;
	}
	
	// Vinkelkoll
	float Dot = FVector::DotProduct(Forward, ToPlayer);
	float Angle = FMath::Acos(Dot) * (180.f / PI);

	if (Angle > VisionAngle * 0.5f)
	{
		bNewPlayerInCone = false;
	}


	// Linetracekoll
	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		Hit,
		CameraLocation,
		PlayerPawn->GetActorLocation(),
		ECC_Visibility,
		Params
	);

	if (bHit && Hit.GetActor() != PlayerPawn)
	{
		bNewPlayerInCone = false;
	}

	// Debug
	if (bVisionDebug)
	{
		DrawDebugCone(
			GetWorld(),
			CameraLocation,
			Forward,
			VisionRange,
			FMath::DegreesToRadians(VisionAngle * 0.5f),
			FMath::DegreesToRadians(VisionAngle * 0.5f),
			12,
			bNewPlayerInCone ? FColor::Green : FColor::Red,
			false,
			0.05f
		);

		/*DrawDebugLine(GetWorld(), CameraLocation, PlayerPawn->GetActorLocation(),
		              FColor::Yellow, false, 0.05f);*/
	}


	// Uppdatera state
	if (bNewPlayerInCone)
	{
		bPlayerInCone = true;
		SpotTimer += DeltaTime;

		/*// Stoppa animation om den spelar
		if (bIsAnimationPlaying)
		{
			CameraMesh->Stop();
			bIsAnimationPlaying = false;
		}*/

		if (SpotTimer >= TimeToSpotPlayer && !bHasSpottedPlayer)
		{
			LastSpottedPlayerLocation = PlayerPawn->GetActorLocation();
			
			bHasSpottedPlayer = true;
			OnPlayerSpotted();
		}
	}
	else
	{
		bPlayerInCone = false;
		SpotTimer = 0.f;

		// reseta bool
		bHasSpottedPlayer = false;

		// Starta animation igen om den inte längre spelar
		/*if (!bIsAnimationPlaying && IdlePanAnimation)
		{
			CameraMesh->PlayAnimation(IdlePanAnimation, true);
			CameraMesh->SetPlayRate(0.25f); 
			bIsAnimationPlaying = true;
		}*/
	}
}


void ASecurityCamera::OnPlayerSpotted()
{
	UE_LOG(LogTemp, Warning, TEXT("Player spotted by camera!"));

	// Hitta EnemyHandler
	AEnemyHandler* Handler = Cast<AEnemyHandler>(UGameplayStatics::GetActorOfClass(
		GetWorld(), AEnemyHandler::StaticClass()
	));

	if (!Handler)
	{
		UE_LOG(LogTemp, Error, TEXT("SecurityCamera: No EnemyHandler found!"));
		return;
	}

	// Hämta två närmaste fiender
	TArray<AMeleeEnemy*> Squad = Handler->GetTwoClosestEnemies(LastSpottedPlayerLocation);

	for (AMeleeEnemy* Enemy : Squad)
	{
		if (!Enemy) continue;

		AMeleeAIController* AI = Cast<AMeleeAIController>(Enemy->GetController());
		if (!AI) continue;

		AI->StartChasingFromExternalOrder(LastSpottedPlayerLocation);          
	}
}

void ASecurityCamera::ActivateCamera()
{
	if (bIsCameraDead)
	{
		UE_LOG(LogTemp, Warning, TEXT("ActivateCamera: Camera is dead and cannot be reactivated."));
		return;
	}

	if (!bIsCameraDisabled)
	{
		return;
	}

	bIsCameraDisabled = false;

	/*// Starta kamerarotationsanimation igen
	if (CameraMesh)
	{
		CameraMesh->Play(true);
	}*/

	// Återställ vision
	bPlayerInCone = false;
	bHasSpottedPlayer = false;
	SpotTimer = 0.f;
	

	UE_LOG(LogTemp, Warning, TEXT("SecurityCamera Activated"));
}


void ASecurityCamera::DisableCamera()
{
	if (bIsCameraDisabled)
		return;

	bIsCameraDisabled = true;

	/*// Stoppa animation
	if (CameraMesh)
	{
		CameraMesh->Stop();
	}*/

	// Slå av vision 
	bPlayerInCone = false;
	bHasSpottedPlayer = false;
	SpotTimer = 0.f;
	

	UE_LOG(LogTemp, Warning, TEXT("SecurityCamera Disabled"));
}


float ASecurityCamera::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
								  AController* EventInstigator, AActor* DamageCauser)
{
	if (bIsCameraDisabled)
		return 0.f;

	float AppliedDamage = FMath::Min(CurrentHealth, DamageAmount);
	CurrentHealth -= AppliedDamage;

	UE_LOG(LogTemp, Warning, TEXT("Camera took %f damage. Health: %f"), AppliedDamage, CurrentHealth);

	if (CurrentHealth <= 0.f)
	{
		Die();
	}

	return AppliedDamage;
}

void ASecurityCamera::Die()
{
	/*if (bIsCameraDisabled)
		return;*/
	
	//UE_LOG(LogTemp, Warning, TEXT("SecurityCamera destroyed!"));
	
	DisableCamera();
	bIsCameraDead = true;

	// Hämta EnemyHandler
	AEnemyHandler* Handler = Cast<AEnemyHandler>(
		UGameplayStatics::GetActorOfClass(GetWorld(), AEnemyHandler::StaticClass())
	);

	if (!Handler)
	{
		UE_LOG(LogTemp, Error, TEXT("CameraDie: No EnemyHandler found!"));
		return;
	}

	// Hämta närmaste fienden till kameran
	AMeleeEnemy* ClosestEnemy = Handler->GetClosestEnemyToLocation(GetActorLocation()); 

	if (!ClosestEnemy)
	{
		UE_LOG(LogTemp, Warning, TEXT("CameraDie: No enemy found to investigate."));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("CameraDie: Closest enemy to investigate: %s"), *ClosestEnemy->GetName());

	
	// Hämta AI Controller
	AMeleeAIController* AI = Cast<AMeleeAIController>(ClosestEnemy->GetController());

	if (AI)
	{
		ClosestEnemy->OnSuspiciousLocation.Broadcast(GetActorLocation());
	}
	

	//CameraMesh->SetVisibility(false);
	SetActorEnableCollision(false);
	
}
