// Fill out your copyright notice in the Description page of Project Settings.


#include "HideSpot.h"

#include "StealthCharacter.h"
#include "StealthGameInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"


AHideSpot::AHideSpot()
{
	PrimaryActorTick.bCanEverTick = false;

	HideRoot = CreateDefaultSubobject<USceneComponent>("Root");
	SetRootComponent(HideRoot);
	
	StaticMeshComponent->SetupAttachment(HideRoot);

	// Camera location 
	CameraPosition = CreateDefaultSubobject<USceneComponent>("CameraPosition");
	CameraPosition->SetupAttachment(HideRoot);

	// Vart spelaren hamnar efter att dem lämnar hide object
	ExitPoint = CreateDefaultSubobject<USceneComponent>("ExitPoint"); 
	ExitPoint->SetupAttachment(HideRoot);

	ExitBox = CreateDefaultSubobject<UBoxComponent>("ExitBox");
	ExitBox->SetupAttachment(CameraPosition);
	ExitBox->SetCollisionResponseToChannel(TRACE_CHANNEL_INTERACT, ECR_Ignore);

	/*const ECollisionChannel InteractChannel = static_cast<ECollisionChannel>(14);
	// Sätt Interact kanalen till Block 
	HideMesh->SetCollisionResponseToChannel(InteractChannel, ECR_Block);
	HideMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);*/

	StaticMeshComponent->SetGenerateOverlapEvents(false);

	
	bOccupied = false;
}

void AHideSpot::BeginPlay()
{
	Super::BeginPlay();
	
	InteractText = EnterText;
}

void AHideSpot::Use_Implementation(AStealthCharacter* Player)
{
	Super::Use_Implementation(Player);
	if (bOccupied)
	{
		ExitHideSpot();
		return;
	}

	EnterHideSpot(Player);
}

void AHideSpot::EnterHideSpot(AStealthCharacter* Player)
{
	if (!Player) return;
	//UE_LOG(LogTemp, Warning, TEXT("AHideSpot: EnterHideSpot"));
	ExitBox->SetCollisionResponseToChannel(TRACE_CHANNEL_INTERACT, ECR_Block);
	
	bOccupied = true;
	PlayerPawn = Player;
	InteractText = ExitText;
	Execute_UpdateShowInteractable(this);

	Player->SetCanTakeDamage(false);

	// Stänger av collisions
	Player->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Gömmer spelaren
	Player->FirstPersonMesh->SetHiddenInGame(true);
	Player->FirstPersonMesh->SetVisibility(false, true);
	
	Player->bIsHiddenFromEnemy = true;
	
	Player->bIsHiding = true;
	
	// Stänger av movement
	Player->GetCharacterMovement()->DisableMovement();

	// Placerar spelaren vid hide spot
	Player->SetActorLocation(CameraPosition->GetComponentLocation());
	Player->SetActorRotation(CameraPosition->GetComponentRotation());

	// Placerar kameran
	Player->SetCustomCameraLocation(CameraPosition);
	Player->HideBaseRotation = CameraPosition->GetComponentRotation();

	// sätt rotationsgränser
	APlayerController* PC = Cast<APlayerController>(Player->GetController());
	if (PC)
	{
		APlayerCameraManager* CamManager = PC->PlayerCameraManager;

		CamManager->ViewPitchMin = MinPitch;
		CamManager->ViewPitchMax = MaxPitch;

		CamManager->ViewYawMin = Player->HideBaseRotation.Yaw + MinYaw;
		CamManager->ViewYawMax = Player->HideBaseRotation.Yaw + MaxYaw;
	}
}

void AHideSpot::ExitHideSpot()
{
	if (!bOccupied) return;
	if (!PlayerPawn) return;
	//UE_LOG(LogTemp, Warning, TEXT("AHideSpot: ExitHideSpot"));
	
	ExitBox->SetCollisionResponseToChannel(TRACE_CHANNEL_INTERACT, ECR_Ignore);

	AStealthCharacter* Player = PlayerPawn;

	Player->SetCanTakeDamage(true);
	
	// Återställer movement
	Player->GetCharacterMovement()->SetMovementMode(MOVE_Walking);

	// Återställer collisions
	Player->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	// Visar spelaren igen
	Player->FirstPersonMesh->SetHiddenInGame(false);
	Player->FirstPersonMesh->SetVisibility(true, true);
	
	// Gör att fienden kan se spelaren igen. 
	Player->bIsHiddenFromEnemy = false;
	
	Player->bIsHiding = false;
	InteractText = EnterText;
	Execute_UpdateShowInteractable(this);

	// Återställer kameran
	Player->ResetToNormalCamera();

	bOccupied = false;

	APlayerController* PC = Cast<APlayerController>(Player->GetController());
	if (PC)
	{
		APlayerCameraManager* CamManager = PC->PlayerCameraManager;

		// Återställ rotationsgränser 
		CamManager->ViewPitchMin = Player->HideMinPitch;
		CamManager->ViewPitchMax = Player->HideMaxPitch;

		CamManager->ViewYawMin = Player->HideMinYaw;
		CamManager->ViewYawMax = Player->HideMaxYaw;

		//Se till att kontrollerns rotation är valid  
		FRotator ResetRot = PC->GetControlRotation();
		ResetRot.Yaw = Player->GetActorRotation().Yaw;
		PC->SetControlRotation(ResetRot);
		
		Player->FirstPersonCameraComponent->SetRelativeRotation(FRotator::ZeroRotator);
	}

	// Teleporterar spelaren till exitpoint
	if (ExitPoint)
	{
		FVector ExitLoction = ExitPoint->GetComponentLocation();
		ExitLoction.Z +=  Player->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
		Player->SetActorLocation(ExitLoction);
		Player->SetActorRotation(ExitPoint->GetComponentRotation());
	}
	else
	{
		// fallback ifall ExitPoint inte finns
		UE_LOG(LogTemp, Warning, TEXT("AHideSpot: ExitPoint missing"));
		const FVector ExitOffset = Player->GetActorForwardVector() * 50.f;
		Player->SetActorLocation(GetActorLocation() + ExitOffset);
	}
}
