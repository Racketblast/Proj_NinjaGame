// Fill out your copyright notice in the Description page of Project Settings.


#include "SoundUtility.h"
#include "Kismet/GameplayStatics.h"
#include "MeleeEnemy.h"


void USoundUtility::ReportNoise(UWorld* World, FVector Location, float Loudness, AActor* ActorThatMadeNoice)
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




		
		// Dämpningsfaktor per vägg
		const float WallDamping = 0.3f;

		// Line trace params
		FCollisionQueryParams TraceParams;
		TraceParams.AddIgnoredActor(Enemy);
		if (ActorThatMadeNoice)
		{
			TraceParams.AddIgnoredActor(ActorThatMadeNoice);
		}

		// Lista med objekt som redan räknats som något som blockerade ljudet
		TSet<TWeakObjectPtr<AActor>> CountedBlockers;

		int WallsBlocking = 0;

		FVector Start = Location;
		FVector End = Enemy->GetActorLocation();

		// Up till max 3 träffar
		for (int i = 0; i < 3; i++)
		{
			FHitResult Hit;

			bool bHit = World->LineTraceSingleByChannel(
				Hit,
				Start,
				End,
				ECC_Visibility,
				TraceParams
			);

			if (!bHit)
				break;

			AActor* HitActor = Hit.GetActor();
			if (!HitActor)
				break;

			// Om objektet inte redan räknats, så lägger vi till det också
			if (!CountedBlockers.Contains(HitActor))
			{
				CountedBlockers.Add(HitActor);
				WallsBlocking++;
			}

			// Se till att vi aldrig träffar samma objekt igen
			TraceParams.AddIgnoredActor(HitActor);

			// Flytta startpunkten lite för att fortsätta längre fram
			Start = Hit.ImpactPoint + (End - Hit.ImpactPoint).GetSafeNormal() * 5.f;
		}
		

		//UE_LOG(LogTemp, Warning, TEXT("WallsBlocking: %i"), WallsBlocking);
		
		// Hur mycket volym som finns kvar efter väggar
		float OccludedLoudness = Loudness * FMath::Pow(WallDamping, WallsBlocking);

		// Slutlig hörselradie baserat på dämpningen
		float EffectiveHearingDistance = BaseHearingDistance * FMath::Pow(OccludedLoudness, 0.7f);

		// Gör så att fienden inte kan höra saker som är utanför dens maximala hearing range. 
		const float FinalHearingRadius = FMath::Min(EffectiveHearingDistance, Enemy->HearingRange);
		
			
		if (Distance <= FinalHearingRadius)
		{
			Enemy->HearSoundAtLocation(Location);
		}

		// Debug visualisering
		/*DrawDebugLine(World, Location, Enemy->GetActorLocation(), WallsBlocking == 0 ? FColor::Green : FColor::Red,false, 0.2f, 0, 2.f);
		DrawDebugSphere(World, Location, FinalHearingRadius, 16, FColor::Cyan, false, 0.2f);*/
	}
}

void USoundUtility::PlaySoundAtLocation(UObject* WorldContextObject, USoundBase* Sound, FVector Location, float Volume, float Pitch)
{
	if (!Sound || !WorldContextObject) return;
	UGameplayStatics::PlaySoundAtLocation(WorldContextObject, Sound, Location, Volume, Pitch);
}
