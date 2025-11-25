// Fill out your copyright notice in the Description page of Project Settings.


#include "BreakableObject.h"

#include "SoundUtility.h"
#include "ThrowableObject.h"
#include "Components/AudioComponent.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "Misc/MapErrors.h"

ABreakableObject::ABreakableObject()
{
	RootSceneComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComp"));
	RootComponent = RootSceneComp;
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	StaticMeshComponent->SetupAttachment(RootSceneComp);
	StaticMeshComponent->SetNotifyRigidBodyCollision(true);
	AudioComp = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComp"));
	AudioComp->SetupAttachment(StaticMeshComponent);
	AudioComp->bAutoActivate = false;

	StaticMeshComponent->OnComponentHit.AddDynamic(this, &ABreakableObject::OnCompHit);
}

void ABreakableObject::BeginPlay()
{
	Super::BeginPlay();
}

void ABreakableObject::OnChaosBreak(const FChaosBreakEvent& BreakEvent)
{
	if (bBroken)
		return;
	bBroken = true;

	UE_LOG(LogTemp, Warning, TEXT("Broken"));
	if (AudioComp)
	{
		AudioComp->Play();
	}
	
	USoundUtility::ReportNoise(GetWorld(), GetActorLocation(), ShatterNoiceLevel);
}

void ABreakableObject::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}


void ABreakableObject::OnCompHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (AThrowableObject* ThrowableObject = Cast<AThrowableObject>(OtherActor))
	{
		BreakObject();
	}
}

void ABreakableObject::BreakObject()
{
	FVector StaticScale = StaticMeshComponent->GetComponentScale();
	StaticMeshComponent->DestroyComponent(true);

	if (ImpactDebris)
	{
		UGeometryCollectionComponent* GeoComp =
			NewObject<UGeometryCollectionComponent>(this, UGeometryCollectionComponent::StaticClass());
		if (GeoComp)
		{
			GeoComp->SetMaterial(0, BreakMaterial);
			GeoComp->SetCanEverAffectNavigation(false);
			GeoComp->SetWorldScale3D(StaticScale);
			GeoComp->SetupAttachment(GetRootComponent());

			GeoComp->RegisterComponent();

			GeoComp->SetRelativeTransform(FTransform::Identity);

			GeoComp->SetRestCollection(ImpactDebris);

			GeoComp->SetCollisionProfileName(TEXT("Player"));
			GeoComp->SetPerLevelCollisionProfileNames({"None","Debris","Debris"});
			GeoComp->SetNotifyBreaks(true);
	
			GeoComp->OnChaosBreakEvent.AddDynamic(this, &ABreakableObject::OnChaosBreak);
		}
	}
}
