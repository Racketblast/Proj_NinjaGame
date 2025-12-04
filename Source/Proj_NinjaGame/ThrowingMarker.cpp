// Fill out your copyright notice in the Description page of Project Settings.


#include "ThrowingMarker.h"

#include "NiagaraComponent.h"

// Sets default values
AThrowingMarker::AThrowingMarker()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	MarkerRootComponent = CreateDefaultSubobject<USceneComponent>("MarkerRoot");
	RootComponent = MarkerRootComponent;

	MarkerMesh = CreateDefaultSubobject<UStaticMeshComponent>("MarkerMesh");
	MarkerMesh->SetupAttachment(MarkerRootComponent);
	MarkerMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MarkerMesh->Nanite(true);
	
	MarkerVFX = CreateDefaultSubobject<UNiagaraComponent>("MarkerVFX");
	MarkerVFX->SetupAttachment(MarkerRootComponent);
	
}

// Called when the game starts or when spawned
void AThrowingMarker::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AThrowingMarker::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AThrowingMarker::SetMarkerScale(FVector Size)
{
	MarkerMesh->SetRelativeScale3D(Size);
}

void AThrowingMarker::SetMarkerMesh(UStaticMesh* Mesh)
{
	MarkerMesh->SetStaticMesh(Mesh);
}

void AThrowingMarker::SetMarkerRelativeLocation(FVector Location)
{
	MarkerMesh->SetRelativeLocation(Location);
}


void AThrowingMarker::SetGroundMaterial()
{
	if (GroundHitMaterial && MarkerMesh->GetMaterial(0) != GroundHitMaterial)
	{
		MarkerMesh->SetMaterial(0,GroundHitMaterial);
	}
	if (GroundHitVFX && MarkerVFX->GetAsset() != GroundHitVFX)
	{
		MarkerVFX->SetAsset(GroundHitVFX);
	}
}

void AThrowingMarker::SetEnemyMaterial()
{
	if (EnemyHitMaterial && MarkerMesh->GetMaterial(0) != EnemyHitMaterial)
	{
		MarkerMesh->SetMaterial(0,EnemyHitMaterial);
	}
	if (EnemyHitVFX && MarkerVFX->GetAsset() != EnemyHitVFX)
	{
		MarkerVFX->SetAsset(EnemyHitVFX);
	}
}

void AThrowingMarker::SetHeadMaterial()
{
	if (HeadHitMaterial && MarkerMesh->GetMaterial(0) != HeadHitMaterial)
	{
		MarkerMesh->SetMaterial(0,HeadHitMaterial);
	}
	if (HeadHitVFX && MarkerVFX->GetAsset() != HeadHitVFX)
	{
		MarkerVFX->SetAsset(HeadHitVFX);
	}
}
