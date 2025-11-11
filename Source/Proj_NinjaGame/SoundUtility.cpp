// Fill out your copyright notice in the Description page of Project Settings.


#include "SoundUtility.h"
#include "Kismet/GameplayStatics.h"
#include "MeleeEnemy.h"

/*void USoundUtility::ReportNoise(UWorld* World, FVector Location, float Loudness)
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
}*/

void USoundUtility::ReportNoise(UWorld* World, FVector Location, float Loudness)
{
	if (!World) return;

	TArray<AActor*> FoundEnemies;
	UGameplayStatics::GetAllActorsOfClass(World, AMeleeEnemy::StaticClass(), FoundEnemies);

	for (AActor* EnemyActor : FoundEnemies)
	{
		AMeleeEnemy* Enemy = Cast<AMeleeEnemy>(EnemyActor);
		if (!Enemy) continue;

		const float Distance = FVector::Dist(Enemy->GetActorLocation(), Location);

		//Bas-radius, alltså hur långt ett ljud med loudness = 1 hörs
		const float BaseHearingDistance = 1000.f;

		//Skala med Loudness
		const float EffectiveHearingDistance = BaseHearingDistance * FMath::Pow(Loudness, 0.7f);

		// Gör så att fienden inte kan höra saker som är utanför dens maximala hearing range. 
		const float FinalHearingRadius = FMath::Min(EffectiveHearingDistance, Enemy->HearingRange);

		if (Distance <= FinalHearingRadius)
		{
			Enemy->HearSoundAtLocation(Location);
		}

		// Debug visualisering
		// DrawDebugSphere(World, Location, FinalHearingRadius, 16, FColor::Cyan, false, 0.2f);
	}
}

void USoundUtility::PlaySoundAtLocation(UObject* WorldContextObject, USoundBase* Sound, FVector Location, float Volume, float Pitch)
{
	if (!Sound || !WorldContextObject) return;
	UGameplayStatics::PlaySoundAtLocation(WorldContextObject, Sound, Location, Volume, Pitch);
}
