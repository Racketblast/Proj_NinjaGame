// Fill out your copyright notice in the Description page of Project Settings.


#include "SecurityCamera.h"

#include "EnemyHandler.h"
#include "NavigationSystem.h"
#include "Components/AudioComponent.h"
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

	//Audio
	StateAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("StateAudioComponent"));
	StateAudioComponent->SetupAttachment(RootComponent);
	
	StateAudioComponent->bAutoActivate = false; 	// styr ljuden i koden, så detta ska vara false

	// VFX
	StateVFXComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("StateVFX"));
	StateVFXComponent->SetupAttachment(CameraMesh);
	StateVFXComponent->SetRelativeLocation(FVector(0.f, 0.f, 120.f));


	// Spotlight
	VisionSpotlight = CreateDefaultSubobject<USpotLightComponent>(TEXT("VisionSpotlight"));
	VisionSpotlight->SetupAttachment(CameraMesh);

	// Spotlight: vinkel 
	VisionSpotlight->SetInnerConeAngle(20.f);
	VisionSpotlight->SetOuterConeAngle(35.f);

	// Spotlight: styrka
	VisionSpotlight->Intensity = 3000.f;   
	VisionSpotlight->bUseInverseSquaredFalloff = false;

	// Spotlight: Färge
	VisionSpotlight->SetLightColor(FLinearColor::White);  

	// Spotlight: Räckvid
	VisionSpotlight->AttenuationRadius = 2200.f;

	// Spotlight: Position
	VisionSpotlight->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
	VisionSpotlight->SetRelativeRotation(FRotator(0.f, 0.f, 0.f));

	// Spotlight: inställningar för skuggor
	VisionSpotlight->SetMobility(EComponentMobility::Movable);
	VisionSpotlight->CastShadows = true;
	VisionSpotlight->bCastVolumetricShadow = true;
	VisionSpotlight->ShadowResolutionScale = 2.0f; 
	VisionSpotlight->ShadowBias = 0.5f;          
	VisionSpotlight->ShadowSharpen = 1.0f;
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

	VisionSpotlight->AttachToComponent(CameraMesh,
	FAttachmentTransformRules::SnapToTargetIncludingScale,
	"Vision");
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

		// VFX
		if (!bHasSpottedPlayer)
		{
			SetVFXState(ECameraVFXState::Alert);
		}

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

		// VFX
		SetVFXState(ECameraVFXState::None);

		// Audio
		if (StateAudioComponent && AlertSound)
		{
			//UE_LOG(LogTemp, Warning, TEXT("Camera: AlertSound!"));
			StateAudioComponent->SetSound(AlertSound);
			StateAudioComponent->Play();
		}

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

	// Audio
	if (StateAudioComponent && DetectedSound)
	{
		StateAudioComponent->SetSound(DetectedSound);
		StateAudioComponent->Play();
	}

	// VFX
	SetVFXState(ECameraVFXState::Detected);

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

	if (VisionSpotlight)
	{
		VisionSpotlight->SetVisibility(true);
		VisionSpotlight->SetIntensity(3000.f); 
	}
	

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

	// VFX
	if (StateVFXComponent)
	{
		StateVFXComponent->SetAsset(nullptr);
		StateVFXComponent->Activate(false);
	}

	if (VisionSpotlight)
	{
		VisionSpotlight->SetVisibility(false);
		VisionSpotlight->SetIntensity(0.f);
	}

	if (StateAudioComponent)
	{
		StateAudioComponent->Stop();
	}

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
		//ClosestEnemy->OnSuspiciousLocation.Broadcast(GetActorLocation());
		AI->AssignMission(EEnemyMission::Camera, GetActorLocation());
	}
	

	if (EnemyHandler)
	{
		if (EnemyHandler->GetAllEnemies().Contains(this))
		{
			EnemyHandler->RemoveEnemy(this);
		}
	}

	SetActorEnableCollision(false);
}


void ASecurityCamera::SetVFXState(ECameraVFXState NewState)
{
	if (CurrentVFXState == NewState)
		return; 

	CurrentVFXState = NewState;

	if (!StateVFXComponent)
		return;

	switch (CurrentVFXState)
	{
	case ECameraVFXState::None:
		StateVFXComponent->SetAsset(nullptr);
		StateVFXComponent->Deactivate();
		break;

	case ECameraVFXState::Alert:
		if (AlertVFX)
		{
			StateVFXComponent->SetAsset(AlertVFX);
			StateVFXComponent->Activate(true);
		}
		break;

	case ECameraVFXState::Detected:
		if (DetectedVFX)
		{
			StateVFXComponent->SetAsset(DetectedVFX);
			StateVFXComponent->Activate(true);
		}
		break;
	}
}
