// Fill out your copyright notice in the Description page of Project Settings.


#include "HideSpot.h"

#include "StealthCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"


AHideSpot::AHideSpot()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>("Root");
	SetRootComponent(Root);

	HideMesh = CreateDefaultSubobject<UStaticMeshComponent>("HideMesh");
	HideMesh->SetupAttachment(Root);

	// Camera location 
	CameraPosition = CreateDefaultSubobject<USceneComponent>("CameraPosition");
	CameraPosition->SetupAttachment(Root);

	// Vart spelaren hamnar efter att dem lämnar hide object
	ExitPoint = CreateDefaultSubobject<USceneComponent>("ExitPoint"); 
	ExitPoint->SetupAttachment(Root); 

	// Collison
	HideMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	HideMesh->SetCollisionObjectType(ECC_WorldDynamic);

	/*const ECollisionChannel InteractChannel = static_cast<ECollisionChannel>(14);
	// Sätt Interact kanalen till Block 
	HideMesh->SetCollisionResponseToChannel(InteractChannel, ECR_Block);
	HideMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);*/

	HideMesh->SetGenerateOverlapEvents(false);

	
	bOccupied = false;
}

void AHideSpot::BeginPlay()
{
	Super::BeginPlay();
}


void AHideSpot::ShowInteractable_Implementation(bool bShow)
{
	IPlayerUseInterface::ShowInteractable_Implementation(bShow);

	if (bShow)
	{
		HideMesh->SetRenderCustomDepth(true);
	}
	else
	{
		HideMesh->SetRenderCustomDepth(false);
	}
}

void AHideSpot::Use_Implementation(AStealthCharacter* Player)
{
	if (!Player) return;
	IPlayerUseInterface::Use_Implementation(Player);
	
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
	bOccupied = true;
	PlayerPawn = Player;   

	// Stänger av collisions
	Player->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Gömmer spelaren
	Player->FirstPersonMesh->SetHiddenInGame(true);
	Player->FirstPersonMesh->SetVisibility(false, true);


	Player->bIsHiddenFromEnemy = true;
	
	Player->bIsHiding = true;

	// Stänger av movement
	Player->GetCharacterMovement()->DisableMovement();

	// Placerar spelaren vis hide spot
	Player->SetActorLocation(CameraPosition->GetComponentLocation());
	Player->SetActorRotation(CameraPosition->GetComponentRotation());

	// Placerar kameran
	Player->SetCustomCameraLocation(CameraPosition);
}

void AHideSpot::ExitHideSpot()
{
	if (!bOccupied) return;
	if (!PlayerPawn) return;
	//UE_LOG(LogTemp, Warning, TEXT("AHideSpot: ExitHideSpot"));

	AStealthCharacter* Player = PlayerPawn;
	
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

	// Återställer kameran
	Player->ResetToNormalCamera();

	bOccupied = false;

	// Teleporterar spelaren till exitpoint
	if (ExitPoint)
	{
		Player->SetActorLocation(ExitPoint->GetComponentLocation());
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
