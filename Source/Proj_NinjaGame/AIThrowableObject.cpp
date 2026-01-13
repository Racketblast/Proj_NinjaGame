// Fill out your copyright notice in the Description page of Project Settings.


#include "AIThrowableObject.h"

#include "BreakableObject.h"
#include "Enemy.h"
#include "SoundUtility.h"
#include "StealthCharacter.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"

void AAIThrowableObject::ThrowableOnComponentHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!Thrown)
		return;
	if (AStealthCharacter* Player = Cast<AStealthCharacter>(OtherActor))
	{
		UGameplayStatics::ApplyPointDamage(
				Player,
				DealtDamage,
				GetVelocity().GetSafeNormal(),
				Hit,
				UGameplayStatics::GetPlayerController(this,0),
				this,
				UDamageType::StaticClass()
			);
		
		if (ImpactEnemySound)
		{
			
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), ImpactEnemySound, GetActorLocation(), 1, 1,0, ThrowableAttenuation);
		}
		else if (ImpactGroundSound)
		{
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), ImpactGroundSound, GetActorLocation(), 1, 1,0, ThrowableAttenuation);
		}
		

		SetShowVFX(true);
	
		Thrown = false;
		StaticMeshComponent->SetCollisionResponseToChannel(TRACE_CHANNEL_INTERACT, ECR_Block);
		StaticMeshComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
		SpawnFieldActor();
		DestroyObject();
	}
	else if (Cast<AEnemy>(OtherActor))
	{
		
	}
	else if (ABreakableObject* Breakable = Cast<ABreakableObject>(OtherActor))
	{
		Thrown = false;
		Breakable->BreakObject();
		if (ImpactGroundSound)
		{
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), ImpactGroundSound, GetActorLocation(), 1, 1,0, ThrowableAttenuation);
		}
		if (bBreaksOnImpact)
		{
			DestroyObject();
		}
		else
		{
			ThrowCollision->SetSimulatePhysics(true);
		}
	}
	else
	{
		if (ImpactGroundSound)
		{
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), ImpactGroundSound, GetActorLocation(), 1, 1,0, ThrowableAttenuation);
		}
		if (bBreaksOnImpact)
		{
			DestroyObject();
		}
		

		SetShowVFX(true);
	
		Thrown = false;
		StaticMeshComponent->SetCollisionResponseToChannel(TRACE_CHANNEL_INTERACT, ECR_Block);
		StaticMeshComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
		SpawnFieldActor();
	}
	/*
		//Sound för fienden
		float NoiseLevel = 4.0f;
	
		USoundUtility::ReportNoise(GetWorld(), Hit.ImpactPoint, NoiseLevel, this);*/
}


void AAIThrowableObject::SetRandomMesh()
{
	if (PossibleMeshes.Num() == 0 || !StaticMeshComponent)
		return;

	const int32 Index = FMath::RandRange(0, PossibleMeshes.Num() - 1);
	UStaticMesh* SelectedMesh = PossibleMeshes[Index];

	if (SelectedMesh)
	{
		StaticMeshComponent->SetStaticMesh(SelectedMesh);
	}
}
