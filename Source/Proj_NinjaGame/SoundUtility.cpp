// Fill out your copyright notice in the Description page of Project Settings.


#include "SoundUtility.h"
#include "Kismet/GameplayStatics.h"
#include "MeleeEnemy.h"


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


		
		/*// Dämpningsfaktor per vägg
		const float WallDamping = 0.5f; 

		// Line trace för att checka väggar
		FCollisionQueryParams TraceParams;
		TraceParams.AddIgnoredActor(Enemy);

		FHitResult Hit;
		bool bHit = World->LineTraceSingleByChannel(
			Hit,
			Location,
			Enemy->GetActorLocation(),
			ECC_Visibility,
			TraceParams
		);

		// Räkna antal väggar
		int WallsBlocking = 0;

		if (bHit)
		{
			WallsBlocking = 1;

			// Fortsätt från hit-punkten vidare för att hitta flera väggar. Upp till 3 väggar 
			FVector TraceStart = Hit.ImpactPoint + (Enemy->GetActorLocation() - Hit.ImpactPoint).GetSafeNormal() * 5.f;
			FVector TraceEnd = Enemy->GetActorLocation();

			for (int i = 0; i < 2; i++)
			{
				if (World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, TraceParams))
				{
					WallsBlocking++;
					TraceStart = Hit.ImpactPoint + (TraceEnd - Hit.ImpactPoint).GetSafeNormal() * 5.f;
				}
				else break;
			}
		}

		// Hur mycket volym som finns kvar efter väggar
		float OccludedLoudness = Loudness * FMath::Pow(WallDamping, WallsBlocking);

		// Slutlig hörselradie baserat på dämpningen
		float EffectiveHearingDistance = BaseHearingDistance * FMath::Pow(OccludedLoudness, 0.7f);

		// Gör så att fienden inte kan höra saker som är utanför dens maximala hearing range. 
		const float FinalHearingRadius = FMath::Min(EffectiveHearingDistance, Enemy->HearingRange);*/
		


			
		if (Distance <= FinalHearingRadius)
		{
			Enemy->HearSoundAtLocation(Location);
		}

		//DrawDebugLine(World, Location, Enemy->GetActorLocation(), WallsBlocking == 0 ? FColor::Green : FColor::Red,false, 0.2f, 0, 2.f);

		// Debug visualisering
		// DrawDebugSphere(World, Location, FinalHearingRadius, 16, FColor::Cyan, false, 0.2f);
	}
}

void USoundUtility::PlaySoundAtLocation(UObject* WorldContextObject, USoundBase* Sound, FVector Location, float Volume, float Pitch)
{
	if (!Sound || !WorldContextObject) return;
	UGameplayStatics::PlaySoundAtLocation(WorldContextObject, Sound, Location, Volume, Pitch);
}
