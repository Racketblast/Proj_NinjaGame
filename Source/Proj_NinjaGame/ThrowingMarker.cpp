// Fill out your copyright notice in the Description page of Project Settings.


#include "ThrowingMarker.h"

#include "NiagaraComponent.h"
#include "ThrowableObject.h"
#include "Components/BoxComponent.h"

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
	MarkerMesh->SetForceDisableNanite(true);
	
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

void AThrowingMarker::AddVFXMarkerRelativeLocation(FVector Location)
{
	FVector NewLocation = {MarkerVFX->GetRelativeLocation().X, MarkerVFX->GetRelativeLocation().Y, Location.Z};
	MarkerVFX->SetRelativeLocation(NewLocation);
}

void AThrowingMarker::UpdateSpawnMarkerMesh(TSubclassOf<class AThrowableObject> ObjectClass)
{
	if (ObjectClass)
	{
		AThrowableObject* DefaultActor = ObjectClass->GetDefaultObject<AThrowableObject>();
		if (DefaultActor && DefaultActor->StaticMeshComponent->GetStaticMesh())
		{
			SetMarkerScale(DefaultActor->StaticMeshComponent->GetRelativeScale3D());
			SetMarkerMesh(DefaultActor->StaticMeshComponent->GetStaticMesh());
			SetMarkerRelativeLocation(DefaultActor->StaticMeshComponent->GetRelativeLocation());
			AddVFXMarkerRelativeLocation(DefaultActor->ThrowCollision->GetScaledBoxExtent());
		}
	}
}

void AThrowingMarker::SetGroundMaterial()
{
	if (GroundHitMaterial && MarkerMesh->GetMaterial(0) != GroundHitMaterial)
	{
		for (int i = 0; i < MarkerMesh->GetNumMaterials(); ++i)
		{
			MarkerMesh->SetMaterial(i,GroundHitMaterial);
		}
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
		for (int i = 0; i < MarkerMesh->GetNumMaterials(); ++i)
		{
			MarkerMesh->SetMaterial(i,EnemyHitMaterial);
		}
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
		for (int i = 0; i < MarkerMesh->GetNumMaterials(); ++i)
		{
			MarkerMesh->SetMaterial(i,HeadHitMaterial);
		}
	}
	if (HeadHitVFX && MarkerVFX->GetAsset() != HeadHitVFX)
	{
		MarkerVFX->SetAsset(HeadHitVFX);
	}
}

UMaterialInterface* AThrowingMarker::GetMeshMaterial() const
{
	if (MarkerMesh && MarkerMesh->GetMaterial(0))
	{
		return MarkerMesh->GetMaterial(0);
	}
	return nullptr;
}
