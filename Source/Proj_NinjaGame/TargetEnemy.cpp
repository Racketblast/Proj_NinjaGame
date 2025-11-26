// Fill out your copyright notice in the Description page of Project Settings.


#include "TargetEnemy.h"


ATargetEnemy::ATargetEnemy()
{
}

void ATargetEnemy::BeginPlay()
{
	Super::BeginPlay();
}

void ATargetEnemy::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void ATargetEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ATargetEnemy::FaceRotation(FRotator NewRotation, float DeltaTime)
{
	Super::FaceRotation(NewRotation, DeltaTime);
}

float ATargetEnemy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator,
	AActor* DamageCauser)
{
	return Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
}

void ATargetEnemy::Die()
{
}

void ATargetEnemy::CheckPlayerVisibility()
{
}

void ATargetEnemy::CheckImmediateProximityDetection()
{
}

void ATargetEnemy::CheckChaseProximityDetection()
{
}

void ATargetEnemy::CheckCloseDetection()
{
}

void ATargetEnemy::OnSuspiciousLocationDetected()
{
}

bool ATargetEnemy::HasClearLOS(const FVector& Start, const FVector& End)
{
	return true;
}

void ATargetEnemy::OnAssasinationOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
}

void ATargetEnemy::OnAssasinationOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
}

void ATargetEnemy::OnMeleeOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
}

void ATargetEnemy::ResetAttackCooldown()
{
}

void ATargetEnemy::ForgetHeardSound()
{
}

void ATargetEnemy::PlayStateSound(USoundBase* NewSound)
{
}

void ATargetEnemy::PlayHurtSound()
{
}

void ATargetEnemy::UpdateLastSeenPlayerLocation()
{
}

void ATargetEnemy::StartAttack()
{
}

void ATargetEnemy::EnableHitbox(float WindowSeconds)
{
}

void ATargetEnemy::DisableHitbox()
{
}

void ATargetEnemy::ApplyDamageTo(AActor* Target)
{
}

void ATargetEnemy::ReduceEnemyRange(bool bShouldReduce)
{
}

void ATargetEnemy::ReduceEnemyHearingRange(bool bShouldReduce)
{
}

void ATargetEnemy::SetLastSeenPlayerLocation(FVector NewLocation)
{
}
/*
ATargetEnemy::FOnSuspiciousLocationDelegate::FOnSuspiciousLocationDelegate()
{
}

ATargetEnemy::FOnSuspiciousLocationDelegate::FOnSuspiciousLocationDelegate(
	const TMulticastScriptDelegate<>& InMulticastScriptDelegate)
{
}

void ATargetEnemy::FOnSuspiciousLocationDelegate::FOnSuspiciousLocationDelegate_DelegateWrapper(
	const FMulticastScriptDelegate&, FVector Location)
{
}*/

void ATargetEnemy::HearSoundAtLocation(FVector SoundLocation)
{
}

void ATargetEnemy::UpdateStateVFX(EEnemyState NewState)
{
}
