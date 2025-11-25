// Fill out your copyright notice in the Description page of Project Settings.


#include "BreakableObject.h"

#include "SoundUtility.h"
#include "Components/AudioComponent.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "Misc/MapErrors.h"

ABreakableObject::ABreakableObject()
{
	AudioComp = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComp"));
	AudioComp->SetupAttachment(RootComponent);
	AudioComp->bAutoActivate = false;
	
	GetGeometryCollectionComponent()->SetPerLevelCollisionProfileNames({"None","Debris","Debris"});
	GetGeometryCollectionComponent()->SetNotifyBreaks(true);
	
	GetGeometryCollectionComponent()->OnChaosBreakEvent.AddDynamic(this, &ABreakableObject::OnChaosBreak);
}

void ABreakableObject::BeginPlay()
{
	Super::BeginPlay();

	GetGeometryCollectionComponent()->PutRigidBodyToSleep();
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
