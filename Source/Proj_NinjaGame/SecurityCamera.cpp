// Fill out your copyright notice in the Description page of Project Settings.


#include "SecurityCamera.h"

#include "Kismet/GameplayStatics.h"


ASecurityCamera::ASecurityCamera()
{
	PrimaryActorTick.bCanEverTick = true;

	CameraMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CameraMesh"));
	RootComponent = CameraMesh;
}

void ASecurityCamera::BeginPlay()
{
	Super::BeginPlay();
	PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

	
	if (CameraMesh && IdlePanAnimation)
	{
		CameraMesh->PlayAnimation(IdlePanAnimation, true);
		CameraMesh->SetPlayRate(0.25f); 
		bIsAnimationPlaying = true;
	}
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
	
	// Hämta position & forward från socket "Vision" i SKM
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


	// Linetrace-koll
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

		// Stoppa animation om den spelar
		if (bIsAnimationPlaying)
		{
			CameraMesh->Stop();
			bIsAnimationPlaying = false;
		}

		if (SpotTimer >= TimeToSpotPlayer && !bHasSpottedPlayer)
		{
			bHasSpottedPlayer = true;
			OnPlayerSpotted();
		}
	}
	else
	{
		bPlayerInCone = false;
		SpotTimer = 0.f;

		// Starta animation igen om den inte längre spelar
		if (!bIsAnimationPlaying && IdlePanAnimation)
		{
			CameraMesh->PlayAnimation(IdlePanAnimation, true);
			CameraMesh->SetPlayRate(0.25f); 
			bIsAnimationPlaying = true;
		}
	}
}


void ASecurityCamera::OnPlayerSpotted()
{
	UE_LOG(LogTemp, Warning, TEXT("Player spotted by camera!"));
    

	// Kalla på fiender
	// Sätt bHasSpottedPlayer to false igen

}
