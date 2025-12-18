// Fill out your copyright notice in the Description page of Project Settings.

#include "TrophyHubActor.h"
#include "StealthGameInstance.h"


ATrophyHubActor::ATrophyHubActor()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ATrophyHubActor::BeginPlay()
{
	Super::BeginPlay();

	UStealthGameInstance* GI = GetGameInstance<UStealthGameInstance>();
	if (!GI) return;

	for (const auto& Elem : MissionThresholds)
	{
		EMission Mission = Elem.Key;
		const FTrophyScoreThresholds& Thresholds = Elem.Value;

		int32 Score;
		if (!GI->GetMissionScore(Mission, Score))
		{
			// Ifall Mission ej är klarad så får man ingen trophy alls
			continue;
		}

		ETrophyTier Tier = GetTrophyTierForScore(Score, Thresholds);
		if (Tier == ETrophyTier::None) continue;

		if (!TrophyActors.Contains(Tier) || !MissionTrophyAnchors.Contains(Mission))
			continue;

		AActor* Anchor = MissionTrophyAnchors[Mission];
		if (!Anchor) continue;

		GetWorld()->SpawnActor<AActor>(
			TrophyActors[Tier],
			Anchor->GetActorTransform()
		);
	}
}

ETrophyTier ATrophyHubActor::GetTrophyTierForScore(int32 Score, const FTrophyScoreThresholds& Thresholds) const
{
	if (Score >= Thresholds.Platinum) return ETrophyTier::Platinum;
	if (Score >= Thresholds.Gold)     return ETrophyTier::Gold;
	if (Score >= Thresholds.Silver)   return ETrophyTier::Silver;
	if (Score >= Thresholds.Bronze)   return ETrophyTier::Bronze;

	return ETrophyTier::None;
}

