// Fill out your copyright notice in the Description page of Project Settings.


#include "SoundUtility.h"
#include "Kismet/GameplayStatics.h"
#include "MeleeEnemy.h"

void USoundUtility::ReportNoise(UWorld* World, FVector Location, float Loudness)
{
	TArray<AActor*> FoundEnemies;
	UGameplayStatics::GetAllActorsOfClass(World, AMeleeEnemy::StaticClass(), FoundEnemies);

	for (AActor* EnemyActor : FoundEnemies)
	{
		AMeleeEnemy* Enemy = Cast<AMeleeEnemy>(EnemyActor);
		if (Enemy)
		{
			float Distance = FVector::Dist(Enemy->GetActorLocation(), Location);
			if (Distance <= Enemy->HearingRange * Loudness)
			{
				Enemy->HearSoundAtLocation(Location);
			}
			//DrawDebugSphere(World, Location, 50.f, 12, FColor::Cyan, false, 1.0f);
		}
	}
}

void USoundUtility::PlaySoundAtLocation(UObject* WorldContextObject, USoundBase* Sound, FVector Location, float Volume, float Pitch)
{
	if (!Sound || !WorldContextObject) return;
	UGameplayStatics::PlaySoundAtLocation(WorldContextObject, Sound, Location, Volume, Pitch);
}
